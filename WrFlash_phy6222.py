#!/usr/bin/env python3

# wrflash_phy6202.py 07.12.2019 pvvx #

import serial;
import time;
import argparse
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

__version__ = "28.12.23"

class phyflasher:
	def __init__(self, port='COM1'):
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
	def write_cmd(self, pkt):
		print(pkt);
		self._port.write(pkt.encode());
		read = self._port.read(6);
		return read == b'#OK>>:'
	def read_regb(self, addr):
		pkt = 'rdreg%08x' % addr;
		sent = self._port.write(pkt.encode());
		read = self._port.read(17);
		if len(read) == 17 and read[0:3] == b'=0x' and read[11:17] == b'#OK>>:':
			dw = struct.pack('<I', int(read[1:11], 16))
			return dw
		return None
	def read_reg(self, addr):
		pkt = 'rdreg%08x' % addr;
		sent = self._port.write(pkt.encode());
		read = self._port.read(17);
		if len(read) == 17 and read[0:3] == b'=0x' and read[11:17] == b'#OK>>:':
			return int(read[1:11], 16)
		return None
	def write_reg(self, addr, data):
		pkt = 'wrreg%08x %08x ' % (addr, data);
		self._port.write(pkt.encode());
		read = self._port.read(6);
		return read == b'#OK>>:'
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
		if not self.ExpFlashSize():
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
		self._port.write(str.encode('era4k %X' % (offset | MAX_FLASH_SIZE))),
		tmp = self._port.timeout
		self._port.timeout = 0.5
		read = self._port.read(6)
		if read != b'#OK>>:':
			print ('error!')
			return False
		print ('ok')
		self._port.timeout = tmp
		return True
	def cmd_era64k(self, offset):
		print ('Erase block 64k Flash at 0x%08x...' % offset, end = ' '),
		self._port.write(str.encode('er64k %X' % (offset | MAX_FLASH_SIZE)))
		tmp = self._port.timeout
		self._port.timeout = 2
		read = self._port.read(6)
		if read != b'#OK>>:':
			print ('error!')
			return False
		print ('ok')
		self._port.timeout = tmp
		return True
	def cmd_er512(self, offset = 0):
		print ('Erase block 512k Flash at 0x%08x...' % offset, end = ' '),
		self._port.write(str.encode('er512 %X' % (offset | MAX_FLASH_SIZE)))
		tmp = self._port.timeout
		self._port.timeout = 2
		read = self._port.read(6)
		if read != b'#OK>>:':
			print ('error!')
			return False
		print ('ok')
		self._port.timeout = tmp
		return True
	def cmd_erase_all_chipf(self):
		print ('Erase Flash work area...', end = ' '),
		self._port.write(str.encode('erall '))
		tmp = self._port.timeout
		self._port.timeout = 7
		read = self._port.read(6)
		if read != b'#OK>>:':
			print ('error!')
			return False
		print ('ok')
		self._port.timeout = tmp
		return True
	def cmd_erase_all_flash(self):
		print ('Erase All Chip Flash...', end = ' '),
		if self.wr_flash_cmd(6) and self.wr_flash_cmd(0x60): #Write Enable, Chip Erase
			print ('ok')
			return True
		return False
	def EraseSectorsFlash(self, offset = 0, size = MAX_FLASH_SIZE):
		count = int((size + PHY_FLASH_SECTOR_SIZE - 1) / PHY_FLASH_SECTOR_SIZE)
		offset &= PHY_FLASH_SECTOR_MASK
		if count > 0 and count < 0x10000 and offset >= 0: # 1 byte .. 16 Mbytes
			while count > 0:
				if (offset & 0x7FFFF) == 0 and count > 127:
					if not self.cmd_er512(offset):
						return False
					offset += 0x80000
					count-=128
				elif (offset & 0x0FFFF) == 0 and count > 15:
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
			self._port.write(str.encode('cpnum %d ' % blkcnt))
			read = self._port.read(6)
			if read != b'#OK>>:':
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
	parser = argparse.ArgumentParser(description='WrFlash-PHY6222 Utility version %s' % __version__, prog='WrFlash_phy6222')
	parser.add_argument('--port', '-p', help='Serial port device',	default='COM1');
	parser.add_argument('--baud', '-b',	help='Set Port Baud (115200, 250000, 500000, 1000000)',	type=arg_auto_int, default=DEF_RUN_BAUD);
	parser.add_argument('--erase', '-e',  action='store_true', help='Erase Flash work area');
	parser.add_argument('--allerase', '-a',  action='store_true', help='All Chip Erase');
	parser.add_argument('address', help='Start Flash address', type=arg_auto_int)
	parser.add_argument('filename', help='File name')
	
	args = parser.parse_args()

	print('WrFlash-PHY6222 Utility version %s' % __version__)

	phy = phyflasher(args.port)
	print ('Connecting...')
	offset = args.address
	if offset >= MAX_FLASH_SIZE:
		print ('Error Start Flash address!')
		sys.exit(1)
	if phy.Connect(args.baud):
		stream = open(args.filename, 'rb')
		size = os.path.getsize(args.filename)
		if size < 1:
			stream.close
			print ('Error: File size = 0!')
			sys.exit(1)
		offset = args.address & (MAX_FLASH_SIZE-1)
		if size + offset > MAX_FLASH_SIZE:
			size = MAX_FLASH_SIZE - offset
		if size < 1:
			stream.close
			print ('Error: Write File size = 0!')
			sys.exit(1)
		aerase = True;
		if args.erase == True or args.allerase == True:
			aerase = False;
			if args.allerase == True:
				if not phy.cmd_erase_all_flash(): # cmd_erase_all_chipf():
					stream.close
					print ('Error: Erase Flash!')
					sys.exit(3)
			else:
				if args.erase == True:
					if not cmd_erase_all_chipf():
						stream.close
						print ('Error: Clear Flash!')
						sys.exit(3)
		phy.SetAutoErase(aerase)
		print ('Write Flash data 0x%08x to 0x%08x from file: %s ...' % (offset, offset + size, args.filename))
		if size > 0:
			if not phy.WriteBlockFlash(stream, offset, size):
				stream.close
				print ('Error: Write Flash!')
				sys.exit(2)
		stream.close
		print ('--------------------------------------------------------')
		print ('Write Flash data 0x%08x to 0x%08x from file: %s - ok.' % (offset, offset + size, args.filename))
	sys.exit(0)	

if __name__ == '__main__':
	main()
