[![english](https://img.shields.io/badge/language-english-C1C1C1?style=for-the-badge)](README.md)
[![russian](https://img.shields.io/badge/also%20available%20in-russian-blue?style=for-the-badge)](README-ru.md)

# BTHome THB1, THB2, THB3, BTH01, TH05 (HW: v1.3..1.6), TH05F

Custom firmware for Tuya devices based on the PHY622x2 chipset.

| [THB1](https://pvvx.github.io/THB1) | [THB2](https://pvvx.github.io/THB2) | [THB3](https://pvvx.github.io/THB3) | [BTH01](https://pvvx.github.io/BTH01/) | [TH05_V1.3](https://pvvx.github.io/TH05-v1.3) | [TH05_V1.4](https://pvvx.github.io/TH-05) | [TH05F](https://pvvx.github.io/TH05F) |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| ![THB1](https://pvvx.github.io/THB1/img/THB1.jpg) | ![THB2](https://pvvx.github.io/THB2/img/THB2.jpg) | ![THB3](https://pvvx.github.io/THB3/img/THB3.jpg) | ![BTH01](https://pvvx.github.io/BTH01/img/BTH01.jpg) | ![TH05V1.3](https://pvvx.github.io/TH05-v1.3/img/TH05-V1.3.jpg) | ![TH05V1.4](https://pvvx.github.io/TH-05/img/TH05V14.jpg) | ![TH05F](https://pvvx.github.io/TH05F/img/TH05F.jpg)

This firmware works with [Home Assistant](https://www.home-assistant.io/) and other software running in the [BTHome](https://bthome.io/) format.

All firmware supports any of these sensors: CHT8215 (CHT8310), CHT8305, AHT20..30.

**Software for setting up and making BLE OTA: [PHY62x2BTHome.html](https://pvvx.github.io/THB2/web/PHY62x2BTHome.html).**
> Uploading OTA files to [PHY62x2BTHome.html](https://pvvx.github.io/THB2/web/PHY62x2BTHome.html) is automatic. You don't need to download files from this repository for OTA.
  
> To run [PHY62x2BTHome.html](https://pvvx.github.io/THB2/web/PHY62x2BTHome.html) offline, just copy the [html](bthome_phy6222/web/PHY62x2BTHome.html) file to a local folder.

## Getting started with the device

To work with the device, you need to write the `FW Boot` firmware to the device via a USB-COM adapter. Boot firmware is a program with reduced functionality and provides secure OTA updates.

Next, using the BLE connection in PHY62x2BTHome.html through the OTA tab, the main operating program of the `FW APP` is recorded.

* `FW APP` can also be recorded using a USB-COM adapter.

## Boot and App firmware

The [Boot](#fw-boot-and-ota) firmware has minimal functions. `FW Boot` is only used for downloading via OTA the full-featured version of `FW APP` (`.bin` files).
* The device type can be distinguished externally by the smiley face symbol on the screen.

| Device | Boot file | OTA file | Printed circuit board labelling |
|:---:|:---:|:---:|:---:|
| [THB1](https://pvvx.github.io/THB1) | BOOT_THB1_v18.hex | THB1_v18.bin | no |
| [THB2](https://pvvx.github.io/THB2) | BOOT_THB2_v18.hex | THB2_v18.bin | no |
| [THB3](https://pvvx.github.io/THB3) | BOOT_THB2_v18.hex | THB2_v18.bin | no |
| [BTH01](https://pvvx.github.io/BTH01) | BOOT_BTH01_v18.hex | BTH01_v18.bin | no |
| [TH05_V1.4](https://pvvx.github.io/TH-05) | BOOT_TH05_v18.hex | TH05_v18.bin | TH05_V1.4, TH05_V1.5, TH05_V1.6 (chip: BL55028) |
| [TH05_V1.3](https://pvvx.github.io/TH05-v1.3) | BOOT_TH05D_v18.hex | TH05D_v18.bin | RSH-TH05-V1.3 (chip: BL55072) |
| [TH05F](https://pvvx.github.io/TH05F) | BOOT_TH05F_v18.hex | TH05F_v18.bin | TH05Y_V1.1, TH05Y_V1.2 (chip: QD01 2332 NT) |

The main firmware files, BOOT_XXX_vXX.hex (for programming via USB-COM adapter) and XXX_vXX.bin (for OTA), are located in the [bin](bin) directory.

Files for updating boot via OTA are located in the [update_boot](update_boot) directory. **The process of updating boot via OTA is not safe. Please check the battery level before doing this. If boot is working fine, there is no need to update to the new version. The need to replace boot with a new version will be announced later.**

> The current `FW Boot' version is **v1.7** for devices with CHT8305 sensor. For other variants, `FW boot' is not required from version **v1.4**.

## Main features

**With default settings**:

* BLE advertisement interval in [BTHome v2](https://bthome.io) format is 5 seconds.
* Humidity and temperature sensor is polled every second BLE advertisement interval - period of 10 seconds.
* Battery voltage measurement is done every minute.
* Recording of history every 30 minutes.
* The button is used for quick connection to old BT adapters. Pressing the button switches the BLE advertising interval to a shorter period (1562.5ms). The action will continue for 60 seconds, then the interval will be restored to the one set in the settings.
* The measured average consumption from a 3.3V source when scanning THB2 and BTH01 thermometers in passive mode is up to 8 µA.  
  For TH05_V1.4 the average consumption is about 23 µA ([this is the current of the installed components](https://github.com/pvvx/THB2/issues/8#issuecomment-1908982171)).
  Other versions with screen: 12 to 14 µA with LCD on and 7 to 12 µA with LCD off.
* Connection interval with Connect Latency (900ms).
* Handling of input counter contact for transmitted [Open/Close events](https://github.com/pvvx/THB2/issues/10#issuecomment-1935169274)
* Processing of output contact switchable by set temperature and/or humidity with hysteresis.
* Support low-cost [BLE to Zigbee advertising repeater](https://github.com/pvvx/TLB2Z)

## Version History

| Version | Description |
|---|--- |
| 1.0 | <ul><li>First release version.</li></ul> |
| 1.1 | <ul><li>Added trigger - TX2 output triggered by set temperature and/or humidity values with hysteresis. Transmission of the state of the RX2 output when connected.</li><li>Added display of smiley face with "comfort" for thermometers with screen.</li><li>Changed device name and MAC.</li></ul> |
| 1.2 | <ul><li>Processing and transmission of open/close events with counter from the output labelled "RX2" (for THB2 - "RX1").</li></ul> |
| 1.3 | <ul><li>Added THB1 and TH05_V1.3.</li><li>Next step is to reduce consumption for versions with LCD display by adding an option to switch off the display.</li></ul>
| 1.4 | <ul><li>Stabilized connection for all device variants.</li><li>Added [TH05F](https://pvvx.github.io/TH05F).</li><li>Fixed RTC progress.</li><li>Changed BLE name for TH05_V1.3 to "TH05D".</li><li>Added files for OTA Boot update.</li></ul> |
| 1.5 | <ul><li>Added option to encrypt BLE ads with BindKey.</li></ul> |
| 1.6 | <ul><li>Added averaging of battery voltage calculation</li><li>Added duplication of open/close contact</li><li>Added counter status 12..20 sec (multiple of ad interval) after triggering and then every 30 min. |
| 1.7 | <ul><li>Fixed en error (> 42 C) for sensor CHT8305</li></ul> |
| 1.8beta | <ul><li>Added display of temperature in degrees Fahrenheit</li></ul> |

## Firmware

It is possible to flash the device with the Boot program via USB-COM adapter with 3.3V outputs.

1. Connect GND, TX, RX, RTS-RESET, VCC (+3.3B):

| Adapter | Device |
|---|---|
| GND | -Vbat |
| +3.3V | +Vbat |
| TX | RX1 |
| RX | TX1 |
| RTS | RESET |

If there is no RST pin on the adapter, then short the RESET pin to GND (-Vbat) and quickly open it when the script starts (it may take a few tries).

The name of the pins on the device can be found in the description at the links: 
* [THB1](https://pvvx.github.io/THB1)
* [THB2](https://pvvx.github.io/THB2)
* [THB3](https://pvvx.github.io/THB3)
* [BTH01](https://pvvx.github.io/BTH01/)
* [TH05_V1.3](https://pvvx.github.io/TH05-v1.3)
* [TH05_V1.4](https://pvvx.github.io/TH-05)

2. Install python3 and the necessary libraries:

```
pip3 install -r requirements.txt
```

3. Download the BOOT_XXX_vXX.hex file required for the specific device from the [bin](bin) directory.

4. Run:

```
python3 rdwr_phy62x2.py -p COM11 -e -r wh BOOT_XXX_vXX.hex
```

5. Boot flashing is complete. The device is operational and the adapter can be disconnected.

6. Download the full version of the firmware via OTA. To do this:
   1. Apply power to the sensor.
   2. Go to [PHY62x2BTHome.html](https://pvvx.github.io/THB2/web/PHY62x2BTHome.html).
   3. Click the `Connect` button, look for the device, and connect.
   4. Once connected, go to the `OTA` tab, select the required firmware (`.bin`) and click `Start`.

7. The device should reboot and be ready for use.

> Optional:
> * To pre-wipe the entire flash, use the `-a` option.
> * To pre-wipe the flash workspace, use the `-e` option.
> * With the USB-COM adapter it is possible to write the main firmware (APP) immediately:
> ```
> python3 rdwr_phy62x2.py -p COM11 -r we 0x10000 XXX_vXX.bin
> ```

## Saving the original firmware

1. Connect GND, TX, RX, RTS-RESET, VCC (+3.3B).
2. Run:

```
python3 rdwr_phy62x2.py -p COM11 -r rc 0x11000000 0x80000 ff_thb2.bin
```

3. Save the resulting ff_thb2.bin file.

## Restoring the original firmware

> Original Tuya firmware for a specific device type can be obtained from [the links in the table at the beginning of this README](#boot-and-app-firmware).

1. Take the saved ff_thb2.bin file of the original firmware.
2. Connect GND, TX, RX, RTS-RESET, VCC (+3.3B).
3. Run:

```
python3 rdwr_phy62x2.py -p COM11 -b 1000000 -r we 0 ff_thb2.bin
```

> Not all USB-COM adapters support 1Mbit. Then remove the `-b 1000000` option or select a different baud rate.

4. The firmware has been flashed and the device should work.

## Flash allocation (512 kilobytes)

| Address | Description | Size |
|---|---|---|
| 0x00000 | ROM used | 8 kilobytes |
| 0x02000 | Boot Info for ROM | 4 kilobytes |
| 0x03000 | FW Boot with OTA function | 52 kilobytes |
| 0x10000 | FW APP | 128 kilobytes |
| 0x30000 | History Recording | 304 kilobytes |
| 0x7C000 | Save Settings (EEP) | 16 kilobytes | 16 kilobytes |

## FW Boot and OTA

* `FW Boot` has an OTA function, but does not have a history function or any other add-ons. It is used to handle OTA for any failed or incorrect updates.

* `FW APP` has no OTA function, for OTA it reboots into `FW Boot`. It has additional features and extensions.

Action of the button when the device is turned on:

* If the button is pressed at startup, `FW Boot` always starts.

* If the button is not pressed, the `FW APP` entry is checked or not. If there is an `FW APP`, it launches the `FW APP`. If there is no `FW APP`, `FW Boot` is launched.

On thermometers with a screen, if the time display is not turned on, during start or restart the following is displayed for a short time:

"Bot 14" - `FW Boot` version 1.4

"APP 15" - `FW APP` version 1.5 

There are two ways to force a reboot to `FW Boot` from `FW APP`:

1. Switch off the power and hold down the button to switch on the power.
2. Enter the `7233` command in the `Service` menu of the [PHY62x2BTHome.html program](https://pvvx.github.io/THB2/web/PHY62x2BTHome.html).

> Complete reboot: Enter the `7201` command in the `Service` menu of the [PHY62x2BTHome.html program](https://pvvx.github.io/THB2/web/PHY62x2BTHome.html).

Through USB-UART adapter, APP can be written immediately after boot. For example:
```
python3 rdwr_phy62x2.py -p COM11 -e wh ./bin/BOOT_TH05V13_v13.hex
python3 rdwr_phy62x2.py -p COM11 -r we 0x10000 ./bin/TH05V13_v13.bin
```

## Open/Close event and pulse counting

Since version 1.2 it is supported to poll a pin connected to a reed switch or a contact shorted to GND.

The maximum switching frequency is 100 times per second.

If the contact is rattling, then it is advisable to shunt the contact with a capacitor.

When closing or opening, a block of 5 BLE adverts is transmitted following each other after a period of 50 ms.

At each "Open" event a counter is added.

The counter value is transmitted together with each "Open/Close" event.

Input contact on the thermometer board:

* On [THB1](https://pvvx.github.io/THB1) - labelled as `RX`.
* On [THB2](https://pvvx.github.io/THB2) - labelled as `RX`.
* On [THB3](https://pvvx.github.io/THB3) - labelled as `RX`.
* On [BTH01](https://pvvx.github.io/BTH01/) - labelled as `RX2`.
* On [TH05_V1.3](https://pvvx.github.io/TH05-v1.3) - labelled as `RX0`.
* On [TH05_V1.4](https://pvvx.github.io/TH-05) - labelled as `RX2`.

![image](https://github.com/pvvx/THB2/assets/12629515/09f6f810-f2e2-4b61-9c84-f7c3770bb76a)

![image](https://github.com/pvvx/THB2/assets/12629515/40de4978-2d97-4f79-af9d-565236d0ba2a)

## Temperature and/or humidity control output of an external device

The contact on the printed circuit board labelled "TX" or "TX2" is controlled with hysteresis setpoints for temperature and humidity.
It is possible to switch to inverse output control.

The setting is made in [PHY62x2BTHome.html program](https://pvvx.github.io/THB2/web/PHY62x2BTHome.html).

Output operation is assigned by setting the hysteresis value:

* If the hysteresis value is zero, there will be no switching.
* If the hysteresis value is greater than zero, switching (switching on) will occur at a value lower than the setpoint + hysteresis.
* If the hysteresis value is less than zero, switching (switching on) will take place at a value higher than setpoint + hysteresis.

## Reset basic settings

To reset basic parameters to initial values, take an Android smartphone and the "nRFConnect" app.
Connect to the device and in the service `0xFCD2` with characteristic `0xFFF4` write `56`.

![image](https://github.com/pvvx/THB2/assets/12629515/85cfaf06-e430-492e-930a-536afb163b5b)

* Pressing/releasing the button temporarily changes the connection interval for connection. Press the button briefly and connect in 60 sec. If you don't have time, press the button more often...
* The firmware checks all settings for compliance with Bluetooth SIG standards.
> If the interval is more than 10 sec, it will set 10 sec as it is the maximum in the standard. So do other settings. However, not all BT adapters can work with 10 sec interval for connection.
* **When working with BLE in Linux it is mandatory to change Bluez options and/or kernel patches!**

## Build Firmware

The GNU Arm Embedded Toolchain is used to build the firmware.

To work in Eclipse, use project import and install toolchain.path.

See [this](https://github.com/pvvx/PHY62x2) for more information on PHY62xx chips.
