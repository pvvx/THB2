#!/usr/bin/env python3

# wrflash_phy6202.py 07.12.2019 pvvx #

import serial;
import time;
import argparse
import io
import os
import struct
import sys

START_BAUD = 9600
DEF_RUN_BAUD = 115200
MAX_FLASH_SIZE = 0x200000
EXT_FLASH_ADD = 0x400000

PHY_FLASH_SECTOR_SIZE = 4096
PHY_FLASH_SECTOR_MASK = 0xfffff000
PHY_WR_BLK_SIZE = 0x2000

__progname__ = 'PHY6222 Utility'
__filename__ = 'rdwr_phy6222.py'
__version__ = "03.01.24"

def ParseHexFile(hexfile):
	try:
		fin = open(hexfile)
	except:
		print('No file opened', hexfile)
		return None
	
	table = []
	result = bytearray()
	addr = 0
	addr_flg = 0
	table.append([0, result, 0x2000])
	for hexstr in fin.readlines():
		hexstr = hexstr.strip()
		#size = int(hexstr[1:3],16)
		if hexstr[7:9] == '04':
			if(len(result)):
				#print(hex(addr))
				table.append([addr, result, 0])
			addr = 	int(hexstr[9:13],16) << 16
			addr_flg = 0
			result = bytearray()
			continue
		if hexstr[7:9] == '05' or hexstr[7:9] == '01':
			table.append([addr, result, 0])
			break
		if addr_flg == 0:
			addr_flg = 1
			addr = addr | (int(hexstr[3:7],16))
		#print(hexstr[9:-3])
		result.extend(bytearray.fromhex(hexstr[9:-2]))
	fin.close()
	return table

class phyflasher:
	def __init__(self, port='COM1'):
		self.old_erase_start = EXT_FLASH_ADD
		self.old_erase_end = EXT_FLASH_ADD
		self.port = port
		self.baud = START_BAUD
		try:
			self._port = serial.Serial(self.port, self.baud)
			self._port.timeout = 1
		except:
			print ('Error: Open %s, %d baud!' % (self.port, self.baud))
			sys.exit(1)
	def SetAutoErase(self, enable = True):
		self.autoerase = enable
	def CreateHead(self):
		self.hexf = [bytearray(b'\xff')*(0x100), bytearray(), bytearray(), bytearray()]
		self.hexf[8:12] = int.to_bytes(0x1fff1838, 4, byteorder='little')
		return self.hexf
	def AddSectionToHead(self, addr, size):
		#self.hexf.sec[0:4] = int.to_bytes(self.hexidx, 4, byteorder='little')
		self.hexf.sec.extend(bytearray(struct.pack('<IIII', phy_head[secn][0], size, addr,0xffffffff)))
		return self.hexf
	def write_cmd(self, pkt):
		self._port.write(pkt.encode());
		read = self._port.read(6);
		return read == b'#OK>>:'
	def SendResetCmd(self):
		return self._port.write(str.encode('reset '));
	def read_reg(self, addr):
		pkt = 'rdreg%08x' % addr;
		sent = self._port.write(pkt.encode());
		read = self._port.read(17);
		if len(read) == 17 and read[0:3] == b'=0x' and read[11:17] == b'#OK>>:':
			return int(read[1:11], 16)
		return None
	def write_reg(self, addr, data):
		return self.write_cmd('wrreg%08x %08x ' % (addr, data))
	def ExpFlashSize(self):
		if not self.write_reg(0x1fff0898, EXT_FLASH_ADD):
			print('Error set ext.Flash size %08x!' % EXT_FLASH_ADD)
			return False
		return True
	def wr_flash_cmd(self, cmd, data = 0, size = 0):
		if size > 0:
			if not self.write_reg(0x4000c8a8, data): #Flash Command Write Data Register
				print('Error write Flash Data Register!')
				return False
		if not self.write_reg(0x4000c890, (cmd << 24) | (size <<15) | 1):
			print('Error write Flash Command Register!')
			return False
		return True
	def FlashUnlock(self):
		#Flash cmd: Write Enable, Write Status Register 0x00 
		return self.wr_flash_cmd(6) and self.wr_flash_cmd(1, 0, 1)	
	def ReadRevision(self):
		#0x001364c8 6222M005 #OK>>:
		self._port.write(str.encode('rdrev+ '));
		self._port.timeout = 0.1
		read = self._port.read(26);
		if len(read) == 26 and read[0:2] == b'0x' and read[20:26] == b'#OK>>:':
			print('Revision:', read[2:19])
			if read[11:15] != b'6222':
				print('Wrong Version!')
			self.flash_id = int(read[2:11], 16)
			self.flash_size = 1 << ((self.flash_id >> 16) & 0xff)
			print('FlashID: %06x, size: %d kbytes' % (self.flash_id, self.flash_size >> 10))
			return True
		else:
			print('Error read Revision!')
		return False
	def SetBaud(self, baud):
		if self._port.baudrate != baud:
			print ('Reopen %s port %i baud...' % (self.port, baud), end = ' '),
			self._port.write(str.encode("uarts%i" % baud));
			self._port.timeout = 1
			read = self._port.read(3);
			if read == b'#OK':
				print ('ok')
				self._port.close()
				self.baud = baud
				self._port.baudrate = baud
				self._port.open();
			else:
				print ('error!')
				print ('Error set %i baud on %s port!' % (baud, self.port))
				self._port.close()
				sys.exit(3)
		return True
	def Connect(self, baud=DEF_RUN_BAUD):
		self._port.setDTR(True) #TM   (lo)
		self._port.setRTS(True) #RSTN (lo)
		self._port.flushOutput()
		self._port.flushInput()
		time.sleep(0.2)
		self._port.setDTR(False) #TM  (hi)
		self._port.flushOutput()
		self._port.flushInput()
		time.sleep(0.1)
		self._port.setRTS(False) #RSTN (hi)
		self._port.timeout = 0.04
		ttcl = 50;
		pkt = 'UXTDWU' # UXTL16 UDLL48 UXTDWU
		while ttcl > 0:
			sent = self._port.write(pkt.encode());
			read = self._port.read(6);
			if read == b'cmd>>:' :
				break
			ttcl = ttcl - 1
			if ttcl < 1:
				print('PHY62x2 - Error Reset!')
				print('Check connection TX->RX, RX<-TX and Chip Power!')
				self._port.close()
				exit(4)
		print('PHY62x2 - Reset Ok')
		self._port.close()
		self._port.baudrate = DEF_RUN_BAUD
		self._port.open();
		self._port.timeout = 0.2
		if not self.ReadRevision():
			self._port.close()
			exit(4)
		if not self.FlashUnlock():
			self._port.close()
			exit(4)
		if not self.write_reg(0x4000f054, 0):
			print('PHY62x2 - Error init1!')
			self._port.close()
			exit(4)
		if not self.write_reg(0x4000f140, 0):
			print('PHY62x2 - Error init2!')
			self._port.close()
			exit(4)
		if not self.write_reg(0x4000f144, 0):
			print('PHY62x2 - Error init3!')
			self._port.close()
			exit(4)
		print('PHY6222 - connected Ok')
		return self.SetBaud(baud)
	def cmd_era4k(self, offset):
		print ('Erase sector Flash at 0x%08x...' % offset, end = ' ')
		tmp = self._port.timeout
		self._port.timeout = 0.5
		ret = self.write_cmd('era4k %X' % (offset | MAX_FLASH_SIZE))
		self._port.timeout = tmp
		if not ret:
			print ('error!')
		else:
			print ('ok')
		return ret
	def cmd_era64k(self, offset):
		print ('Erase block 64k Flash at 0x%08x...' % offset, end = ' '),
		tmp = self._port.timeout
		self._port.timeout = 2
		ret = self.write_cmd('er64k %X' % (offset | MAX_FLASH_SIZE))
		self._port.timeout = tmp
		if not ret:
			print ('error!')
		else:
			print ('ok')
		return ret
	def cmd_er512(self, offset = 0):
		print ('Erase block 512k Flash at 0x%08x...' % offset, end = ' '),
		tmp = self._port.timeout
		self._port.timeout = 2
		ret = self.write_cmd('er512 %X' % (offset | MAX_FLASH_SIZE))
		self._port.timeout = tmp
		if not ret:
			print ('error!')
		else:
			print ('ok')
		return ret
	def cmd_erase_all_chipf(self):
		print ('Erase Flash work area...', end = ' '),
		tmp = self._port.timeout
		self._port.timeout = 7
		ret = self.write_cmd('erall ')
		self._port.timeout = tmp
		if not ret:
			print ('error!')
		else:
			print ('ok')
		return ret
	def cmd_erase_all_flash(self):
		print ('Erase All Chip Flash...', end = ' '),
		if self.wr_flash_cmd(6) and self.wr_flash_cmd(0x60): #Write Enable, Chip Erase
			time.sleep(7)
			print ('ok')
			return True
		print ('error!')
		return False
	def EraseSectorsFlash(self, offset = 0, size = MAX_FLASH_SIZE):
		count = int((size + PHY_FLASH_SECTOR_SIZE - 1) / PHY_FLASH_SECTOR_SIZE)
		offset &= PHY_FLASH_SECTOR_MASK
		if count > 0 and count < 0x10000 and offset >= 0: # 1 byte .. 16 Mbytes
			while count > 0:
				if offset >= self.old_erase_start and  offset < self.old_erase_end:
					offset += PHY_FLASH_SECTOR_SIZE
					count -= 1
					continue
				if (offset & 0x0FFFF) == 0 and count > 15:
					if not self.cmd_era64k(offset):
						return False
					self.old_erase_start = offset
					self.old_erase_end = offset + 0x10000
					offset += 0x10000
					count -= 16
				else:
					if not self.cmd_era4k(offset):
						return False
					self.old_erase_start = offset
					self.old_erase_end = offset + PHY_FLASH_SECTOR_SIZE
					offset += PHY_FLASH_SECTOR_SIZE
					count -= 1
		else:
			return False
		return True
	def EraseSectorsFlash2(self, offset = 0, size = MAX_FLASH_SIZE):
		count = int((size + PHY_FLASH_SECTOR_SIZE - 1) / PHY_FLASH_SECTOR_SIZE)
		offset &= PHY_FLASH_SECTOR_MASK
		if count > 0 and count < 0x10000 and offset >= 0: # 1 byte .. 16 Mbytes
			while count > 0:
				if (offset & 0x0FFFF) == 0 and count > 15:
					if not self.cmd_era64k(offset):
						return False
					offset += 0x10000
					count-=16
				else:
					if not self.cmd_era4k(offset):
						return False
					offset += PHY_FLASH_SECTOR_SIZE
					count-=1
		else:
			return False
		return True
	def send_blk(self, stream, offset, size, blkcnt, blknum):
		self._port.timeout = 1
		print ('Write 0x%08x bytes to Flash at 0x%08x...' % (size, offset), end = ' '),
		if blknum == 0:  
			if not self.write_cmd('cpnum %d ' % blkcnt):
				print ('error!')
				return False
		self._port.write(str.encode('cpbin c%d %X %X %X' % (blknum, offset | MAX_FLASH_SIZE, size, 0x1FFF0000 + offset)))
		read = self._port.read(12)
		if read != b'by hex mode:':
			print ('error!')
			return False
		data = stream.read(size)
		self._port.write(data)
		read = self._port.read(23); #'checksum is: 0x00001d1e'
		#print ('%s' % read),
		if read[0:15] != b'checksum is: 0x':
			print ('error!')
			return False
		self._port.write(read[15:])
		read = self._port.read(6)
		if read != b'#OK>>:':
			print ('error!')
			return False
		print ('ok')
		return True
	def WriteBlockFlash(self, stream, offset = 0, size = 0x8000):
		offset &= 0x00ffffff
		if self.autoerase:
			if not self.EraseSectorsFlash(offset, size):	
				return False
		sblk = PHY_WR_BLK_SIZE
		blkcount = (size + sblk - 1) / sblk
		blknum = 0
		while(size > 0):
			if size < sblk:
				sblk = size
			if not self.send_blk(stream, offset, sblk, blkcount, blknum):
				return False
			blknum+=1
			offset+=sblk
			size-=sblk
		return True
	def ReadBusToFile(self, ff, addr=0x11000000, size=0x80000):
		flg = size > 128
		while size > 0:
			if flg and addr & 127 == 0:
				print('\rRead 0x%08x...' % addr, end='') #, flush=True
			rw = self.read_reg(addr)
			if rw == None:
				print('\rError read address 0x%08x!' % addr)
				return False
			dw = struct.pack('<I',rw)
			ff.write(dw)
			addr += 4
			size -= 4
		print('ok')
		return True
	def WriteHexf(self, sn, ph):
		offset = ph[2]
		offset &= 0x00ffffff
		idx = 0
		size = len(ph[1])
		if self.autoerase:
			if not self.EraseSectorsFlash(offset, size):	
				return False
		sblk = PHY_WR_BLK_SIZE
		blkcount = (size + sblk - 1) / sblk
		blknum = 0
		while(size > 0):
			if size < sblk:
				sblk = size
			if not self.send_blk(stream, offset, sblk, blkcount, blknum):
				return False
			blknum+=1
			offset+=sblk
			size-=sblk
		return True
	def HexStartSend(self):
		return self.write_cmd('spifs 0 1 3 0 ') and self.write_cmd('sfmod 2 2 ') and self.write_cmd('cpnum ffffffff ')
	def HexfHeader(self, hp):
		if len(hp) > 1:
			hexf = bytearray(b'\xff')*(0x100)
			hexf[0:4] = int.to_bytes(len(hp), 4, byteorder='little')
			hexf[8:12] = int.to_bytes(0x1fff1838, 4, byteorder='little')
			sections = 0
			faddr = 0x00020000
			raddr = 0x00005414
			print ('---- Segments Table -------------------------------------')
			for ihp in hp:
				if (ihp[0] & 0x1fff0000) == 0x1fff0000:
					faddr = raddr
					raddr += (len(ihp[1])+3) & 0xfffffffc
				elif (ihp[0] & (~(MAX_FLASH_SIZE-1))) == 0x11000000:
					faddr = ihp[0] & (MAX_FLASH_SIZE-1)
				elif ihp[0] == 0:
					continue
				else:
					print('Invalid Segment Address 0x%08x!' % ihp[0])
					return None
				ihp[2] = faddr
				print('Segment: %08x <- Flash addr: %08x, Size: %08x' % (ihp[0], faddr, len(ihp[1])))
				hexf.extend(bytearray(struct.pack('<IIII', faddr, len(ihp[1]), ihp[0], 0xffffffff)))
				sections += 1
			return hexf
		return None

class FatalError(RuntimeError):
	def __init__(self, message):
		RuntimeError.__init__(self, message)

	@staticmethod
	def WithResult(message, result):
		message += " (result was %s)" % hexify(result)
		return FatalError(message)

def arg_auto_int(x):
	return int(x, 0)

def main():
	parser = argparse.ArgumentParser(description='%s version %s' % (__progname__, __version__), prog=__filename__)
	parser.add_argument('--port', '-p', help='Serial port device',	default='COM1');
	parser.add_argument('--baud', '-b',	help='Set Port Baud (115200, 250000, 500000, 1000000)',	type=arg_auto_int, default=DEF_RUN_BAUD);

	parser.add_argument('--allerase', '-a',  action='store_true', help='Pre-processing: All Chip Erase');
	parser.add_argument('--erase', '-e',  action='store_true', help='Pre-processing: Erase Flash work area');
	parser.add_argument('--reset', '-r',  action='store_true', help='Post-processing: Reset');

	subparsers = parser.add_subparsers(
			dest='operation',
			help='Run '+__filename__+' {command} -h for additional help')

	parser_hex_flash = subparsers.add_parser(
			'wh',
			help='Write hex file to Flash')
	parser_hex_flash.add_argument('filename', help='Name of hex file')

	parser_burn_flash = subparsers.add_parser(
			'we',
			help='Write bin file to Flash with sectors erases')
	parser_burn_flash.add_argument('address', help='Start address', type=arg_auto_int)
	parser_burn_flash.add_argument('filename', help='Name of binary file')

	parser_write_flash = subparsers.add_parser(
			'wf',
			help='Write bin file to Flash without sectors erases')
	parser_write_flash.add_argument('address', help='Start address', type=arg_auto_int)
	parser_write_flash.add_argument('filename', help='Name of binary file')

	parser_erase_sec_flash = subparsers.add_parser(
			'er',
			help='Erase Region (sectors) of Flash')
	parser_erase_sec_flash.add_argument('address', help='Start address', type=arg_auto_int)
	parser_erase_sec_flash.add_argument('size', help='Size of region', type=arg_auto_int)

	parser_erase_all_flash = subparsers.add_parser(
			'ea',
			help='Erase All Flash')

	parser_read_chip = subparsers.add_parser(
			'rc',
			help='Read chip bus address to binary file')
	parser_read_chip.add_argument('address', help='Start address', type=arg_auto_int)
	parser_read_chip.add_argument('size', help='Size of region', type=arg_auto_int)
	parser_read_chip.add_argument('filename', help='Name of binary file')

	parser_read_flash = subparsers.add_parser(
			'i', help='Chip Information')
	
	args = parser.parse_args()

	print('=========================================================')
	print('%s version %s' % (__progname__, __version__))
	print('---------------------------------------------------------')
	phy = phyflasher(args.port)
	print ('Connecting...')
	#--------------------------------
	phy.Connect(args.baud)
	if args.operation == 'rc':
		#filename = "r%08x-%08x.bin" % (addr, length)
		if args.size == 0:
			print("Read Size = 0!" )
			exit(1);
		try:
			ff = open(args.filename, "wb")
		except:
			print("Error file open '%s'" % filename)
			exit(2)
		if not phy.ReadBusToFile(ff, args.address, args.size):
			ff.close()
			exit(4)
		print
		print ('---------------------------------------------------------')
		byteSaved = (args.size + 3) & 0xfffffffc
		if byteSaved > 1024:
			print("%.3f KBytes saved to file '%s'" % (byteSaved/1024, args.filename))
		else:
			print("%i Bytes saved to file '%s'" % (byteSaved, args.filename))
		ff.close()
	#--------------------------------wr flash bin
	if args.operation == 'we' or args.operation == 'wf':
		offset = args.address & (MAX_FLASH_SIZE-1)
		if offset >= MAX_FLASH_SIZE:
			print ('Error Start Flash address!')
			sys.exit(1)
		stream = open(args.filename, 'rb')
		size = os.path.getsize(args.filename)
		if size < 1:
			stream.close()
			print ('Error: File size = 0!')
			sys.exit(1)
		offset = args.address & (MAX_FLASH_SIZE-1)
		if size + offset > MAX_FLASH_SIZE:
			size = MAX_FLASH_SIZE - offset
		if size < 1:
			stream.close()
			print ('Error: Write File size = 0!')
			sys.exit(1)
		aerase = args.operation == 'we'
		if args.erase == True or args.allerase == True:
			aerase = False;
			if args.allerase == True:
				if not phy.cmd_erase_all_flash():
					stream.close()
					print ('Error: Erase All Flash!')
					sys.exit(3)
			else:
				if args.erase == True:
					if not phy.cmd_erase_all_chipf():
						stream.close
						print ('Error: Erase Flash!')
						sys.exit(3)
		phy.SetAutoErase(aerase)
		print ('Write Flash data 0x%08x to 0x%08x from file: %s ...' % (offset, offset + size, args.filename))
		if not phy.ExpFlashSize():
			exit(4)
		if size > 0:
			if not phy.WriteBlockFlash(stream, offset, size):
				stream.close()
				print ('Error: Write Flash!')
				sys.exit(2)
		stream.close()
		print ('----------------------------------------------------------')
		print ('Write Flash data 0x%08x to 0x%08x from file: %s - ok.' % (offset, offset + size, args.filename))
	#--------------------------------wr flash hex
	if args.operation == 'wh':
		hp = ParseHexFile(args.filename)
		if hp == None:
			sys.exit(2)
		hexf = phy.HexfHeader(hp)
		if hexf == None:
			sys.exit(2)
		hp[0][1] = hexf
		if not phy.HexStartSend():
			sys.exit(2)
		print ('----------------------------------------------------------')
		aerase = True
		if args.erase == True or args.allerase == True:
			aerase = False;
			if args.allerase == True:
				if not phy.cmd_erase_all_flash():
					stream.close()
					print ('Error: Erase All Flash!')
					sys.exit(3)
			else:
				if args.erase == True:
					if not phy.cmd_erase_all_chipf():
						stream.close
						print ('Error: Erase Flash!')
						sys.exit(3)
		phy.SetAutoErase(aerase)
		if not phy.ExpFlashSize():
			exit(4)
		segment = 0
		for ihp in hp:
			if ihp[0] == 0:
				print('Segment Table[%02d] <- Flash addr: %08x, Size: %08x' % (len(hp) - 1, ihp[2], len(ihp[1])))
			else:
				print('Segment: %08x <- Flash addr: %08x, Size: %08x' % (ihp[0], ihp[2], len(ihp[1])))
			stream = io.BytesIO(ihp[1])
			if not phy.WriteBlockFlash(stream, ihp[2], len(ihp[1])):
				stream.close()
				sys.exit(2)
			stream.close()
			segment += 1
		print ('----------------------------------------------------------')
		print ('Write Flash from file: %s - ok.' % args.filename)
	#--------------------------------erase flash region
	if args.operation == 'er':
		offset = args.address & (MAX_FLASH_SIZE-1)
		if offset >= MAX_FLASH_SIZE:
			print ('Error Flash Start address!')
			sys.exit(1)
		size = args.size & (MAX_FLASH_SIZE-1)
		if size >= MAX_FLASH_SIZE:
			print ('Error Flash Erase size!')
			sys.exit(1)
		if size + offset > MAX_FLASH_SIZE:
			size = MAX_FLASH_SIZE - offset
		if size < 1:
			print ('Error Flash Erase size!')
			sys.exit(1)
		if not phy.ExpFlashSize():
			exit(4)
		if not phy.EraseSectorsFlash(offset, size):
			sys.exit(2)
	#--------------------------------erase flash all
	if args.operation == 'ea':
		if not phy.cmd_erase_all_chipf():
			print ('Error: Erase Flash!')
			sys.exit(3)
	if args.reset:
		phy.SendResetCmd()
		print ("Send command 'reset' - ok")
	sys.exit(0)

if __name__ == '__main__':
	main()
