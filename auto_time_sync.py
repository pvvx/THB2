"""
Auto Time Sync for BTHome Devices
--------------------------------

This program automatically synchronizes the time on BTHome-compatible Bluetooth Low Energy (BLE) devices.
It continuously scans for devices that advertise the BTHome service UUID (0xFCD2) and ensures their 
internal clocks are synchronized with the host computer's time.

Key Features:
- Discovers BTHome devices using service UUID 0xFCD2
- Automatically connects and syncs time when drift exceeds 3 seconds
- Supports multiple devices with concurrent processing
- Implements retry mechanism for failed connections
- Prevents excessive reconnections with 60-second cooldown
- Includes timeout protection for unresponsive devices

Technical Implementation:
1. Device Discovery:
   - Uses BleakScanner with a detection callback
   - Filters devices by BTHome service UUID (0xFCD2)
   - Checks both service UUIDs and service data

2. Connection Process:
   - Establishes BLE connection with 30-second timeout
   - Discovers services and sets up notifications
   - Uses characteristic 0xFFF4 for time sync commands

3. Time Synchronization:
   - Reads current device time via command 0x33
   - Compares with system time accounting for timezone
   - Updates device time if difference exceeds 3 seconds
   - Uses command 0x23 to set new time value

4. Error Handling:
   - Implements connection retries (max 2 attempts)
   - Includes timeout protection for all BLE operations
   - Gracefully handles disconnections and exceptions

Usage:
Run the script directly to start the auto time sync service:
    python auto_time_sync.py

The program will continuously scan for and process BTHome devices, logging all operations
and any errors that occur. Use Ctrl+C to gracefully stop the service.
"""

import asyncio
import time
from datetime import datetime
from bleak import BleakScanner, BleakClient
from bleak.backends.device import BLEDevice

class AutoTimeSync:
    def __init__(self):
        self.device = None
        self.client = None
        self.cmd_characteristic = "0000fff4-0000-1000-8000-00805f9b34fb"
        self.service_uuid = "0000fcd2-0000-1000-8000-00805f9b34fb"
        self.is_handling_notification = False
        self.processed_devices = {}  # Track when each device was last processed
        self.current_task = None
        self.start_time = time.time()  # Track when the program started

    def add_log(self, text: str, is_error: bool = False):
        """Log messages with timestamp"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        prefix = "ERROR: " if is_error else ""
        print(f"{timestamp}: {prefix}{text}")

    def set_status(self, text: str, is_error: bool = False):
        """Update status with optional error indication"""
        prefix = "ERROR: " if is_error else "Status: "
        print(f"{prefix}{text}")

    async def set_device_time(self):
        """Set the device time"""
        if not self.client or not self.client.is_connected:
            return False

        # Calculate current time in seconds since epoch, adjusted for timezone
        current_time = int(time.time())
        timezone_offset = time.timezone if (time.localtime().tm_isdst == 0) else time.altzone
        adjusted_time = current_time - timezone_offset

        # Create time sync packet
        time_bytes = bytearray([0x23])  # Command byte
        time_bytes.extend(adjusted_time.to_bytes(4, 'little'))  # Time in little endian

        try:
            await self.client.write_gatt_char(self.cmd_characteristic, time_bytes, response=True)
            self.add_log('Time set successfully')
            return True
        except Exception as e:
            self.add_log(f'Error setting time: {str(e)}', True)
            return False

    async def handle_notification(self, sender: int, data: bytearray):
        """Handle incoming notifications from the device"""
        if not data or self.is_handling_notification:
            return

        self.is_handling_notification = True
        try:
            blk_id = data[0]
            
            # Handle measurement data packet (0x33)
            if blk_id == 0x33 and len(data) >= 15:
                device_time = int.from_bytes(data[11:15], 'little')
                local_time = int(time.time()) - time.timezone
                time_diff = abs(device_time - local_time)
                
                self.add_log(f'Device time: {datetime.fromtimestamp(device_time)}')
                self.add_log(f'Time difference: {time_diff} seconds')
                
                time_updated = False
                if time_diff > 3:
                    self.add_log('Time difference > 3s, updating device time...')
                    time_updated = await self.set_device_time()

                # Disconnect after processing
                self.add_log(f'Time {"updated" if time_updated else "checked"}, disconnecting...')
                if self.client and self.client.is_connected:
                    try:
                        await self.client.disconnect()
                    except:
                        pass  # Ignore disconnect errors

                # Update last processed time for this device
                if self.device:
                    self.processed_devices[self.device.address] = time.time()

        finally:
            self.is_handling_notification = False

    async def process_device(self, device: BLEDevice, max_retries=2):
        """Process a single device with retries"""
        for attempt in range(max_retries):
            try:
                self.device = device
                device_name = self.device.name if self.device.name else self.device.address
                self.set_status(f'Connecting to device {device_name} (attempt {attempt + 1}/{max_retries})...')
                
                # Create a timeout for the entire connection process
                async def connect_with_timeout():
                    self.client = BleakClient(device, timeout=20.0)
                    await self.client.connect()

                    self.set_status('Connected, discovering services...')
                    await asyncio.sleep(1)  # Give device time to settle
                    if not self.client.services:
                        raise Exception("No services found")

                    self.set_status('Setting up notifications...')
                    await self.client.start_notify(
                        self.cmd_characteristic,
                        self.handle_notification
                    )

                    self.set_status(f'Connected to {device_name}')
                    
                    # Request initial measurement and wait for response
                    await self.client.write_gatt_char(
                        self.cmd_characteristic,
                        bytearray([0x33]),
                        response=True
                    )
                    # Give some time for notification to arrive
                    await asyncio.sleep(2)

                try:
                    # Set a 30-second timeout for the entire connection process
                    await asyncio.wait_for(connect_with_timeout(), timeout=30.0)
                    # If we get here, we succeeded so break the retry loop
                    break
                except asyncio.TimeoutError:
                    raise Exception("Connection process timed out")

            except Exception as error:
                device_name = device.name if device.name else device.address
                self.set_status(f'Error with {device_name} (attempt {attempt + 1}/{max_retries}): {str(error)}', True)
                self.add_log(str(error), True)
                if self.client and self.client.is_connected:
                    await self.client.disconnect()
                if attempt < max_retries - 1:
                    await asyncio.sleep(2)  # Wait before retry

    async def connect(self):
        """Scan and process all THB devices"""
        try:
            self.set_status('Scanning for devices...')
            
            # Scan for devices with longer timeout
            discovered_devices = []
            def detection_callback(device, advertisement_data):
                service_uuid_lower = self.service_uuid.lower()
                # Check both service UUIDs and service data
                has_service_uuid = (advertisement_data.service_uuids and 
                                  service_uuid_lower in [str(uuid).lower() for uuid in advertisement_data.service_uuids])
                has_service_data = service_uuid_lower in [str(uuid).lower() for uuid in advertisement_data.service_data.keys()]
                
                if has_service_uuid or has_service_data:
                    discovered_devices.append(device)

            scanner = BleakScanner(detection_callback=detection_callback)
            await scanner.start()
            await asyncio.sleep(15.0)  # Scan for 15 seconds
            await scanner.stop()
            
            matching_devices = discovered_devices
            
            if not matching_devices:
                self.add_log('No devices with BTHome service found', True)
                return

            current_time = time.time()
            # Process each discovered device that hasn't been processed recently
            for device in matching_devices:
                # Skip devices processed in the last 60 seconds
                if device.address in self.processed_devices:
                    last_processed = self.processed_devices[device.address]
                    if current_time - last_processed < 60:
                        device_name = device.name if device.name else device.address
                        self.add_log(f'Skipping {device_name} - processed recently')
                        continue
                
                await self.process_device(device)

        except Exception as error:
            self.set_status(f'Error: {str(error)}', True)
            self.add_log(str(error), True)

    async def run(self):
        """Main run loop"""
        while time.time() - self.start_time < 600:  # Run for 10 minutes (600 seconds)
            try:
                await self.connect()
                # Wait before next scan
                await asyncio.sleep(1)
            except asyncio.CancelledError:
                self.add_log("Shutting down...")
                if self.client and self.client.is_connected:
                    await self.client.disconnect()
                break
            except Exception as e:
                self.add_log(f"Error in main loop: {e}", True)
                await asyncio.sleep(1)
        
        self.add_log("10 minute runtime completed")

if __name__ == "__main__":
    print("Auto Time Sync")
    print("-------------")
    print("Program will run for 10 minutes")
    
    sync = AutoTimeSync()
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    
    try:
        loop.run_until_complete(sync.run())
    except KeyboardInterrupt:
        print("\nShutting down due to user interrupt")
    finally:
        pending = asyncio.all_tasks(loop)
        for task in pending:
            task.cancel()
        
        # Give tasks a chance to complete
        loop.run_until_complete(asyncio.gather(*pending, return_exceptions=True))
        loop.close()
