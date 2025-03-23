#!/usr/bin/env python3

# rdwr_phy62x2.py 27.01.2025 pvvx #

import serial
import time
import argparse
import io
import os
import struct
import sys

START_BAUD = 9600
DEF_RUN_BAUD = 115200
MAX_FLASH_SIZE = 0x200000
EXT_FLASH_ADD = 0x400000

DEF_START_RUN_APP_ADDR = 0x1FFF1838
DEF_START_WR_FLASH_ADDR = 0x05000

PHY_FLASH_SECTOR_SIZE = 4096
PHY_FLASH_SECTOR_MASK = 0xfffff000
PHY_WR_BLK_SIZE = 0x2000

PHY_FLASH_ADDR = 0x11000000
PHY_SRAM_ADDR = 0x1fff0000

__progname__ = 'PHY62x2/ST17H66B/TG7100B Utility'
__filename__ = 'rdwr_phy62x2.py'
__version__ = "01.03.25"

def ParseHexFile(hexfile):
	try:
		fin = open(hexfile)
	except:
		print('No file opened', hexfile)
		return None
	
	table = []
	result = bytearray()
	addr = 0
	naddr = 0
	taddr = 0
	addr_flg = 0
	table.append([0, result, 0x2000])
	for hexstr in fin.readlines():
		hexstr = hexstr.strip()
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
		taddr = (int(hexstr[3:7],16))
		if addr_flg == 0:
			addr_flg = 1
			addr = addr | taddr
			naddr = taddr
		if taddr != naddr:
			addr_flg = 1
			table.append([addr, result, 0])
			addr = (addr & 0xFFFF0000) | taddr
			result = bytearray()
		#print(hexstr[9:-3])
		result.extend(bytearray.fromhex(hexstr[9:-2]))
		naddr = taddr + int(hexstr[1:3],16)
	fin.close()
	return table

class phyflasher:
	def __init__(self, port='COM1', tm = False, next = False, debug = False):
		self.debug = debug
		self.tm = tm
		self.next = next
		self.chip = 'Unknown'
		self.cpbin = 0
		self.autoerase = True
		self.old_erase_start = EXT_FLASH_ADD
		self.old_erase_end = EXT_FLASH_ADD
		self.port = port
		self.tg7100 = False
		if tm:
			self.baud = DEF_RUN_BAUD
		else:
			self.baud = START_BAUD
		try:
			self._port = serial.Serial(self.port, self.baud)
			self._port.timeout = 1
		except Exception as e:
			print ('Error: Open %s, %d baud! Error: %s' % (self.port, self.baud, e))
			sys.exit(1)
	def SetAutoErase(self, enable = True):
		self.autoerase = enable
	def AddSectionToHead(self, addr, size):
		#self.hexf.sec[0:4] = int.to_bytes(self.hexidx, 4, byteorder='little')
		self.hexf.sec.extend(bytearray(struct.pack('<IIII', phy_head[secn][0], size, addr, 0xffffffff)))
		return self.hexf
	def write_cmd(self, pkt):
		self._port.write(pkt.encode())
		read = self._port.read(6)
		return read == b'#OK>>:'
	def SendResetCmd(self):
		return self._port.write(str.encode('reset '))
	def read_reg(self, addr):
		pkt = 'rdreg%08x ' % addr
		sent = self._port.write(pkt.encode())
		read = self._port.read(17)
		if len(read) == 17 and read[0:3] == b'=0x' and read[11:17] == b'#OK>>:':
			return int(read[1:11], 16)
		return None
	def write_reg(self, addr, data):
		return self.write_cmd('wrreg%08x %08x ' % (addr, data))
	def subwrite(self, addr, data1, data2, data3):
		return self.write_cmd('subwr%08x %u %u %x ' % (addr, data1, data2, data3))
	def ExpFlashSize(self):
		if self.tg7100:
			addr = 0x1fff1080
			size = self.flash_size * 2
			#return True
		else:
			addr = 0x1fff0898
			size = self.patch_addr_flash << 2 # EXT_FLASH_ADD
		if self.debug:
			rw = self.read_reg(addr)
			if rw == None:
				print('error!')
				print('\rError read address 0x%08x!' % addr)
				return False
			print('# Read Flash size from 0x%08x: 0x%08x' % (addr, rw))
			print('# Flash Read-Write offset 0x%08x' % self.patch_addr_flash)
		if not self.write_reg(addr, size):
			print('Error set ext.Flash size %08x!' % size)
			return False
		print('Expand Flash window size to %i KB' % (size >> 10))
		return True
	def wr_flash_cmd(self, cmd, data = 0, wrlen = 0, addr = 0, addrlen = 0, rdlen = 0, mbit = 0, dummy = 0):
		regcmd = cmd << 24
		if wrlen > 0:
			regcmd = regcmd | 0x8000 | ((wrlen - 1) << 12)
			if not self.write_reg(0x4000c8a8, data): #Flash Command Write Data Register
				print('Error write Flash Data Register!')
				return False
		if addrlen > 0:
			regcmd = regcmd | 0x80000 | ((addrlen - 1) << 16)
			if not self.write_reg(0x4000c894, addr): #Flash Command Write Addr Register
				print('Error write Flash Address Register!')
				return False
		if rdlen > 0:
			regcmd = regcmd | 0x800000 | ((rdlen - 1) << 20)
		if mbit > 0:
			regcmd = regcmd | 0x40000
		if dummy > 0:
			regcmd = regcmd | (dummy << 7)
		if not self.write_reg(0x4000c890, regcmd | 1):
			print('Error write Flash Command Register!')
			return False
		return True
	def flash_wait_idle(self):
		i = 5
		while i > 0:
			r = self.read_reg(0x4000c890)
			if r == None:
				return False
			if (r & 2) == 0:
				i = 5
				while i > 0:
					r = self.read_reg(0x4000c800)
					if r == None:
						return False
					if (r & 0x80000000) != 0:
						return True
					i -= 1
				return False
			i -= 1		
		return False
	def flash_read_otp(self, addr = 0):
		if self.wr_flash_cmd(0x48,addr,3,0,1,4) and self.flash_wait_idle(): 
			r1 = self.read_reg(0x4000c8a0)
			if r1 == None:
				return None
			ret = bytearray(3)
			ret[0:3] = int.to_bytes(r1 & 0x0ffffff, 3, byteorder='little')
			return ret 
		return None
	def flash_read_jdec_id(self):
		if self.wr_flash_cmd(0x9f,0,0,0,0,3): # and self.flash_wait_idle(): 
			r1 = self.read_reg(0x4000c8a0)
			if r1 == None:
				return None
			ret = bytearray(3)
			ret[0:3] = int.to_bytes(r1 & 0x0ffffff, 3, byteorder='little')
			return ret
		return None
	def read_efuse(self, block = 0):
		block &= 3
		e = [0]*2
		#temp = self.read_reg(0x4000f024) # RTCCTL
		#if not self.write_reg(0x4000f024, temp | 1):
		#	return None
		if not self.write_reg(0x4000f180, 0x0f): #1 << block): #
			return None
		#if not self.subwrite(0x4000f054, 16+block, 16+block, 1):
		#	return None
		if not self.write_reg(0x4000f054, (1 << (16 + block))):
			return False
		if not self.write_reg(0x4000f054, 0):
			return False
		e[0] = self.read_reg(0x4000f160 + 8*block) # n.0
		if e[0] == None:
			return None
		e[1] = self.read_reg(0x4000f164 + 8*block) # n.1
		if e[1] == None:
			return None
		return e
	def _write_efuse(self, block, dw0, dw1):
		block &= 3
		temp = self.read_reg(0x4000f024) # RTCCTL
		if not self.write_reg(0x4000f024, temp | 1):
			return False
		if not self.write_reg(0x4000f180, 0xffffffff):
			return False
		if not self.write_reg(0x4000f140, dw0) or not self.write_reg(0x4000f144, dw1):
			return False
		i = 1
		while i < 32:
			if not self.write_reg(0x4000f054, (1 << i) | 0x8000):
				return False
			i += 1
		#if not self.write_reg(0x4000f180, 0x0f):
		#	return False
		#if not self.write_reg(0x4000f140, dw0) or not self.write_reg(0x4000f144, dw1):
		#	return False
		#if not self.write_reg(0x4000f054, (1 << (28 + block)) | 0x0ff0ffff):
		#	return False
		#if not self.write_reg(0x4000f054, 0):
		#	return False
		e = self.read_efuse(block)
		print(e)
		if e == None:
			return False
		return e[0] == dw0 and e[1] == dw1
	def read_all_efuse(self, block = 0):
		efuse = [0]*10
		#self._port.write(str.encode("rdrev+ "))
		#read = self._port.read(26)
		#print(read)
		if not self.write_reg(0x4000f054, 0):
			return False
		efuse[0] = self.read_reg(0x4000f180) # EFUSE LOCK Status
		if efuse[0] == None:
			return None
		e = self.read_efuse(0)
		if e == None:
			return None
		efuse[1:3] = e
		e = self.read_efuse(1)
		if e == None:
			return None
		efuse[3:5] = e
		e = self.read_efuse(2)
		if e == None:
			return None
		efuse[5:7] = e
		e = self.read_efuse(3)
		if e == None:
			return None
		efuse[7:9] = e
		self._port.write(str.encode("rdrev "))
		read = self._port.read(16)
		if len(read) == 16 and read[0:2] == b'0x' and read[10:16] == b'#OK>>:':
			efuse[9] = int(read[:10], 16)
			return efuse
		return None
	def spif_remap(self, addr = 0x80000):
		r1 = self.read_reg(0x4000C800)
		if r1 == None:
			return False
		if addr == 0:
			return self.write_reg(0x4000C824, addr) and self.write_reg(0x4000C800, r1 & 0xfffeffff)
		return self.write_reg(0x4000C824, addr) and self.write_reg(0x4000C800, r1 | 0x10000)
	def flash_read_unique_id(self):
		if self.wr_flash_cmd(0x4B,0,0,0,4,8): # and self.flash_wait_idle(): 
			r1 = self.read_reg(0x4000c8a0)
			if r1 == None:
				return None
			r2 = self.read_reg(0x4000c8a4)
			if r2 == None:
				return None
			ret = bytearray(8)
			ret[0:4] = int.to_bytes(r1, 4, byteorder='little')
			ret[4:8] = int.to_bytes(r2, 4, byteorder='little')
			return ret 
		return None
	def flash_read_status(self):
		#Flash cmd: Read status
		if self.wr_flash_cmd(5,0,0,0,0,2): # and self.flash_wait_idle(): 
			r = self.read_reg(0x4000c8a0)
			if r == None:
				return None
			return r & 0x0ffff
		return None
	def FlashUnlock(self):
		#Flash cmd: Write Enable, Write Status Register 0x00 
		if self.flash_man == 0x85: # Puya
			return self.wr_flash_cmd(6) and self.wr_flash_cmd(0x50) and self.wr_flash_cmd(1, 0, 2)
		return self.wr_flash_cmd(6) and self.wr_flash_cmd(1, 0, 1)
	def ReadRevision(self):
		#0x001364c8 6222M005 #OK>>:
		self._port.write(str.encode('rdrev+ '))
		self._port.timeout = 0.1
		read = self._port.read(26)
		#print(read)
		if len(read) == 16 and read[0:2] == b'0x' and read[10:16] == b'#OK>>:':
			print('Revision:', read[2:10])
			self.flash_id = int(read[2:10], 16)
			self.flash_man = int(read[8:10], 16)
			self.flash_size = 1 << (self.flash_id  & 0xff)
			self.patch_addr_flash = self.flash_size
			self.chip = 'TG7100B'
			self.tg7100 = True
		elif len(read) == 26 and read[0:2] == b'0x' and read[20:26] == b'#OK>>:':
			print('Revision:', read[2:19])
			if read[11:15] == b'6230':
				self.chip = 'PHY6230'
				print('Chip PHY6230: OTP Version!')
			else:
				if read[11:15] != b'6222':
					print('Wrong Version!')
				self.chip = 'PHY6222'
				self.flash_id = int(read[2:10], 16)
				self.flash_man = int(read[8:10], 16)
				self.flash_size = 1 << ((self.flash_id >> 16) & 0xff)
				self.patch_addr_flash = self.flash_size << 1 # MAX_FLASH_SIZE
		else:
			print('Error read Revision!')
			return False
		print('FlashID: %06x, size: %d kbytes' % (self.flash_id & 0xffffff, self.flash_size >> 10))
		sb = self.flash_id >> 24
		if sb != 0:
			print("Security Boot:", sb)
		if self.debug:
			read = self.flash_read_jdec_id()
			if read != None:
				print('# Flash JEDEC ID:', read.hex())
		return True
	def SetBaud(self, baud):
		if self._port.baudrate != baud:
			print ('Reopen %s port %i baud...' % (self.port, baud), end = ' '),
			self._port.timeout = 0.7
			self._port.write(str.encode("uarts%i" % baud))
			read = self._port.read(3)
			self.baud = baud
			try:
				self._port.baudrate = baud
			except Exception as e:
				print ('Error set %i baud on %s port!' % (baud, self.port))
				sys.exit(1)
			if read != b'#OK':
				if self.read_reg(PHY_SRAM_ADDR) == None:
					print ('error!')
					print ('Error set %i baud on %s port!' % (baud, self.port))
					self._port.close()
					sys.exit(3)
			print ('ok')
			self._port.timeout = 0.2
			time.sleep(0.05)
			self._port.flushOutput()
			self._port.flushInput()
		return True
	def Connect(self, baud=DEF_RUN_BAUD):
		if self.next:
			if self._port.baudrate != baud:
				try:
					self._port.baudrate = baud
				except Exception as e:
					print ('Error set %i baud on %s port!' % (baud, self.port))
					sys.exit(1)
			time.sleep(0.05)
			self._port.flushOutput()
			self._port.flushInput()
			time.sleep(0.05)
			if not self.ReadRevision():
				self._port.close()
				exit(4)
			return True
		self._port.setRTS(True) #RSTN (lo)
		self._port.setDTR(True) #TM   (lo)
		time.sleep(0.1)
		self._port.flushOutput()
		self._port.flushInput()
		time.sleep(0.1)
		print('PHY62x2/TG7100B: Release RST_N if RTS is not connected...')
		print('PHY6252/ST17H66B: Turn on the power...')
		self._port.setDTR(False) #TM  (hi)
		self._port.setRTS(False) #RSTN (hi)
		self._port.timeout = 0.04
		ttcl = 250
		fct_mode = False
		pkt = 'UXTDWU' # UXTL16 UDLL48 UXTDWU
		while ttcl > 0:
			sent = self._port.write(pkt.encode())
			read = self._port.read(6)
			if read == b'cmd>>:' :
				break
			if read == b'fct>>:' :
				fct_mode = True
				break
			ttcl = ttcl - 1
			if ttcl < 1:
				print('Chip Reset error! Response: %s' % read)
				print('Check connection TX->RX, RX<-TX, RTS->RESET, TM, and Chip Power!')
				self._port.close()
				exit(4)
		print('Chip Reset Ok. Response: %s' % read)
		self._port.baudrate = DEF_RUN_BAUD
		self._port.timeout = 0.2
		if fct_mode:
			print('Chip in FCT mode!')
			return False
		if not self.ReadRevision():
			self._port.close()
			exit(4)
		if not self.FlashUnlock():
			self._port.close()
			exit(4)
		#if not self.tg7100:
		#	if not self.write_reg(0x4000f054, 0):
		#		print('PHY62x2 - Error init1!')
		#		self._port.close()
		#		exit(4)
		#	if not self.write_reg(0x4000f140, 0):
		#		print('PHY62x2 - Error init2!')
		#		self._port.close()
		#		exit(4)
		#	if not self.write_reg(0x4000f144, 0):
		#		print('PHY62x2 - Error init3!')
		#		self._port.close()
		#		exit(4)
		print(self.chip, '- connected Ok')
		return self.SetBaud(baud)
	def cmd_era4k(self, offset):
		print ('Erase sector Flash at 0x%08x...' % offset, end = ' ')
		tmp = self._port.timeout
		self._port.timeout = 0.5
		ret = self.write_cmd('era4k %X' % (offset | self.patch_addr_flash))
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
		ret = self.write_cmd('er64k %X' % (offset | self.patch_addr_flash))
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
		if self.tg7100:
			ret = self.write_cmd('er512 ')
		else:
			ret = self.write_cmd('er512 %X' % (offset | self.patch_addr_flash))
		self._port.timeout = tmp
		if not ret:
			print ('error!')
		else:
			print ('ok')
		return ret
	def cmd_erase_work_flash(self):
		print ('Erase Flash work area...', end = ' '),
		tmp = self._port.timeout
		self._port.timeout = 7
		if self.tg7100:
			ret = self.write_cmd('etcpf ')
		else:
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
			i = 77
			while i > 0:
				r = self.flash_read_status()
				if r == None:
					print ('Error!')
					return False
				if (r & 1) == 0:
					print ('ok')
					return True
				i -= 1	
		print ('Timeout!')
		return False
	def EraseSectorsFlash(self, offset = 0, size = MAX_FLASH_SIZE):
		count = int((size + PHY_FLASH_SECTOR_SIZE - 1 + (offset & (PHY_FLASH_SECTOR_SIZE - 1))) / PHY_FLASH_SECTOR_SIZE)
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
	def send_blk(self, stream, offset, size, blkcnt, blknum, segment = 0):
		data = stream.read(size)
		for e in data:
			if e != 0xff:
				self._port.timeout = 1
				print ('Write 0x%08x bytes to Flash at 0x%08x...' % (size, offset), end = ' '),
				self._port.write(str.encode('cpbin c%d %X %X %X' % (blknum, offset | self.patch_addr_flash, size, segment + offset)))
				read = self._port.read(12)
				if read != b'by hex mode:':
					print ('error!')
					print ('Error cmd cpbin!', read)
					return False
				self._port.write(data)
				if self.tg7100:
					read = self._port.read(25)  #' checksum is: 0x00001d1e'
					if read[0:16] != b' checksum is: 0x': # TG7100
						print ('error!')
						print ('Error send bin data! ', read)
						return False
					self._port.write(read[16:24])
				else:
					read = self._port.read(23)  #'checksum is: 0x00001d1e'
					if read[0:15] != b'checksum is: 0x':
						print ('error!')
						print ('Error send bin data! ', read)
						return False
					self._port.write(read[15:])
				read = self._port.read(6)
				if read != b'#OK>>:':
					print ('error!')
					print ('Error CRC!', read)
					return False
				print ('ok')
				return True
		return True
	def WriteBlockFlash(self, stream, offset = 0, size = 0x8000, segment = PHY_SRAM_ADDR):
		offset &= 0x00ffffff
		if self.autoerase:
			if not self.EraseSectorsFlash(offset, size):	
				return False
		sblk = PHY_WR_BLK_SIZE
		blkcount = (size + sblk - 1) / sblk
		while(size > 0):
			if size < sblk:
				sblk = size
			if not self.send_blk(stream, offset, sblk, blkcount, self.cpbin, segment):
				return False
			self.cpbin+=1
			offset+=sblk
			size-=sblk
		return True
	def ReadBusToFile(self, ff, addr=PHY_FLASH_ADDR, size=0x80000):
		flg = size > 128
		if not flg:
			print('\rRead 0x%08x...' % addr, end=' ') #, flush=True
		while size > 0:
			if flg and addr & 127 == 0:
				print('\rRead 0x%08x...' % addr, end=' ') #, flush=True
			rw = self.read_reg(addr)
			if rw == None:
				print('error!')
				print('\rError read address 0x%08x!' % addr)
				return False
			dw = struct.pack('<I',rw)
			ff.write(dw)
			addr += 4
			size -= 4
		print('ok')
		return True
	def ReadBusTobArray(self, addr=PHY_FLASH_ADDR, size=0x100):
		db = bytearray()
		while size > 0:
			rw = self.read_reg(addr)
			if rw == None:
				return None
			db += int.to_bytes(rw, 4, byteorder='little') #struct.pack('<I',rw)
			addr += 4
			size -= 4
		return db
	def WriteFileToBus(self, stream, addr=PHY_FLASH_ADDR, size=0x80000):
		flg = size > 128
		if not flg:
			print('\rWrite 0x%08x...' % addr, end=' ') #, flush=True
		while size > 0:
			if flg and addr & 127 == 0:
				print('\rrWrite 0x%08x...' % addr, end=' ') #, flush=True
			dw = int.from_bytes(stream.read(4), byteorder='little', signed=False)
			if not self.write_cmd('wrreg%08x %08x ' % (addr, dw)):
				print('error!')
				print('\rError write address 0x%08x!' % addr)
				return False
			addr += 4
			size -= 4
		print('ok')
		return True
	def ReadAllFlash(self, ff):
		addr = PHY_FLASH_ADDR
		size = self.flash_size
		print('Read at 0x%08x, size: 0x%08x:' % (addr, size))
		while size > 0:
			if addr & 127 == 0:
				print('\rRead 0x%08x...' % addr, end=' ') #, flush=True
			rw = self.read_reg(addr)
			if rw == None:
				print('error!')
				print('\rError read address 0x%08x!' % addr)
				return None
			dw = struct.pack('<I',rw)
			ff.write(dw)
			addr += 4
			size -= 4
		print('ok')
		return self.flash_size
	def SpifsInit(self):
		if self.tm:
			return self.write_cmd('cpnum ffffffff ')
		else:
			return self.write_cmd('spifs 0 1 3 0 ') and self.write_cmd('sfmod 2 2 ') and self.write_cmd('cpnum ffffffff ')
	def HexfHeader(self, hp, start = DEF_START_RUN_APP_ADDR, raddr = DEF_START_WR_FLASH_ADDR):
		if len(hp) > 1:
			if self.tg7100:
				hp[0][2] = 0x2100
				hexf = bytearray(b'\xff')*(4)
				hexf[0:4] = int.to_bytes(len(hp)-1, 4, byteorder='little')
				#sections = 0
				faddr_min = MAX_FLASH_SIZE-1
				faddr_max = 0
				rsize = 0
				for ihp in hp:
					if (ihp[0] & PHY_SRAM_ADDR) == PHY_SRAM_ADDR:	# SRAM
						rsize += len(ihp[1])
					elif (ihp[0] & (~(MAX_FLASH_SIZE-1))) == PHY_FLASH_ADDR: # Flash
						offset = ihp[0] & (MAX_FLASH_SIZE-1)
						if faddr_min > offset:
							faddr_min = offset
						send = offset + len(ihp[1])
						if faddr_max <= send:
							faddr_max = send
				if (raddr + rsize) >= faddr_min:
					raddr = (faddr_max + 15) & 0xfffffff0
				print ('---- Segments Table -------------------------------------')
				for ihp in hp:
					if (ihp[0] & PHY_SRAM_ADDR) == PHY_SRAM_ADDR:	# SRAM
						faddr = raddr
						raddr += (len(ihp[1])+15) & 0xfffffff0
					elif (ihp[0] & (~(MAX_FLASH_SIZE-1))) == PHY_FLASH_ADDR: # Flash
						faddr = ihp[0] & (MAX_FLASH_SIZE-1)
					elif ihp[0] == 0:
						continue
					else:
						print('Invalid Segment Address 0x%08x!' % ihp[0])
						return None
					ihp[2] = faddr
					print('Segment: %08x <- Flash addr: %08x, Size: %08x' % (ihp[0], faddr, len(ihp[1])))
					hexf.extend(bytearray(struct.pack('<III', faddr, len(ihp[1]), ihp[0])))
				return hexf
			hexf = bytearray(b'\xff')*(0x100)
			hexf[0:4] = int.to_bytes(len(hp)-1, 4, byteorder='little')
			hexf[8:12] = int.to_bytes(start, 4, byteorder='little')
			#sections = 0
			faddr_min = MAX_FLASH_SIZE-1
			faddr_max = 0
			rsize = 0
			for ihp in hp:
				if (ihp[0] & PHY_SRAM_ADDR) == PHY_SRAM_ADDR:	# SRAM
					rsize += len(ihp[1])
				elif (ihp[0] & (~(MAX_FLASH_SIZE-1))) == PHY_FLASH_ADDR: # Flash
					offset = ihp[0] & (MAX_FLASH_SIZE-1)
					if faddr_min > offset:
						faddr_min = offset
					send = offset + len(ihp[1])
					if faddr_max <= send:
						faddr_max = send
			if (raddr + rsize) >= faddr_min:
				raddr = (faddr_max + 3) & 0xfffffffc
			#print('Test: Flash addr min: %08x, max: %08x, RAM addr: %08x' % (faddr_min, faddr_max, raddr))
			print ('---- Segments Table -------------------------------------')
			for ihp in hp:
				if (ihp[0] & PHY_SRAM_ADDR) == PHY_SRAM_ADDR:	# SRAM
					faddr = raddr
					raddr += (len(ihp[1])+3) & 0xfffffffc
				elif (ihp[0] & (~(MAX_FLASH_SIZE-1))) == PHY_FLASH_ADDR: # Flash
					faddr = ihp[0] & (MAX_FLASH_SIZE-1)
				elif ihp[0] == 0:
					continue
				else:
					print('Invalid Segment Address 0x%08x!' % ihp[0])
					return None
				ihp[2] = faddr
				print('Segment: %08x <- Flash addr: %08x, Size: %08x' % (ihp[0], faddr, len(ihp[1])))
				hexf.extend(bytearray(struct.pack('<IIII', faddr, len(ihp[1]), ihp[0], 0xffffffff)))
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
	parser = argparse.ArgumentParser(description='%s version %s' % (__progname__, __version__), prog = __filename__)
	parser.add_argument('--port', '-p', help = 'Serial port device',	default='COM1')
	parser.add_argument('--baud', '-b',	help = 'Set Port Baud rate (115200, 250000, 500000, 1000000)',	type = arg_auto_int, default = DEF_RUN_BAUD)

	parser.add_argument('--allerase', '-a',  action='store_true', help = 'Pre-processing: All Chip Erase')
	parser.add_argument('--erase', '-e',  action='store_true', help = 'Pre-processing: Erase Flash work area')
	parser.add_argument('--reset', '-r',  action='store_true', help = 'Post-processing: Reset')
	parser.add_argument('--start', '-s',  help = 'Application start address for hex writer (default: 0x%08x)' % DEF_START_RUN_APP_ADDR, type = arg_auto_int, default = DEF_START_RUN_APP_ADDR)
	parser.add_argument('--write', '-w',  help = 'Flash starting address for hex writer (default: 0x%08x)' % DEF_START_WR_FLASH_ADDR, type = arg_auto_int, default = DEF_START_WR_FLASH_ADDR)
	parser.add_argument('--tm', '-t',  action='store_true', help = 'If pin TM is set "1"')
	parser.add_argument('--next', '-n',  action='store_true', help = 'Next commands (skip startup)')
	parser.add_argument('--debug', '-d',  action='store_true', help = 'Debug')

	subparsers = parser.add_subparsers(
			dest='operation',
			help = 'Run '+__filename__+' {command} -h for additional help')

	parser_hex_flash = subparsers.add_parser(
			'wh',
			help = 'Write hex file to Flash')
	parser_hex_flash.add_argument('filename', help = 'Name of hex file')

	parser_burn_flash = subparsers.add_parser(
			'we',
			help = 'Write bin file to Flash with sectors erases')
	parser_burn_flash.add_argument('address', help = 'Start address', type = arg_auto_int)
	parser_burn_flash.add_argument('filename', help = 'Name of binary file')

	parser_write_flash = subparsers.add_parser(
			'wf',
			help = 'Write bin file to Flash without sectors erases')
	parser_write_flash.add_argument('address', help = 'Start address', type = arg_auto_int)
	parser_write_flash.add_argument('filename', help = 'Name of binary file')

	parser_write_regs = subparsers.add_parser(
			'wc',
			help = 'Write bin file to chip bus address')
	parser_write_regs.add_argument('address', help = 'Start address', type = arg_auto_int)
	parser_write_regs.add_argument('filename', help = 'Name of binary file')

	parser_erase_sec_flash = subparsers.add_parser(
			'er',
			help = 'Erase Region (sectors) of Flash')
	parser_erase_sec_flash.add_argument('address', help = 'Start address', type = arg_auto_int)
	parser_erase_sec_flash.add_argument('size', help = 'Size of region', type = arg_auto_int)

	parser_erase_work_flash = subparsers.add_parser(
			'ew',
			help = 'Erase Flash Work Area')

	parser_erase_all_flash = subparsers.add_parser(
			'ea',
			help = 'Erase All Flash (MAC, ChipID/IV)')

	parser_read_chip = subparsers.add_parser(
			'rc',
			help = 'Read chip bus address to binary file')
	parser_read_chip.add_argument('address', help = 'Start address', type = arg_auto_int)
	parser_read_chip.add_argument('size', help = 'Size of region', type = arg_auto_int)
	parser_read_chip.add_argument('filename', help = 'Name of binary file')

	parser_read_all_flash = subparsers.add_parser(
			'rf',
			help = 'Read all Flash',
			)
	parser_read_all_flash.add_argument('filename', help = 'Name of binary file')

	parser_read_info = subparsers.add_parser(
			'i', help = 'Chip Information')
	
	args = parser.parse_args()

	print('=========================================================')
	print('%s version %s' % (__progname__, __version__))
	print('---------------------------------------------------------')
	phy = phyflasher(args.port, args.tm, args.next, args.debug)
	print ('Connecting...')
	#--------------------------------
	if not phy.Connect(args.baud):
		if args.operation == 'ea':
			if not phy.cmd_er512():
				print ('Error: Erase All Flash!')
				sys.exit(3)
			exit(0)
		else:
			print ("Use the 'Erase All Flash' (ea) command to exit FCT mode!")
			exit(2)
	if args.operation == 'i':
		rs = phy.flash_read_status()
		if rs == None:
			print ('Error Flash read Status!')			
			sys.exit(3)
		print ('Flash Status: 0x%04x' % rs)
		rb = phy.flash_read_unique_id()
		if rb == None:
			print ('Error Flash read Unique ID!')			
			sys.exit(3)
		print ('Flash Serial Number:', rb.hex()) # Unique ID
		r1 = phy.flash_read_otp(0)
		if r1 == None:
			print ('Error Flash read OTP!')			
			sys.exit(3)
		r2 = phy.flash_read_otp(3)
		if r1 == None:
			print ('Error Flash read OTP!')			
			sys.exit(3)
		r3 = phy.flash_read_otp(3)
		if r1 == None:
			print ('Error Flash read OTP!')			
			sys.exit(3)
		print ('Flash OTP:', r1.hex()+r2.hex()+r3.hex()+"...")
		size = 16
		addr = 0x11002000
		db = phy.ReadBusTobArray(addr, size)
		if db == None:
			print('Error read address 0x%08x!' % addr)
			sys.exit(3)
		print('Flash 0x%08x: %s' % (addr,db.hex()))
		addr = 0x11002100
		db = phy.ReadBusTobArray(addr, size)
		if db == None:
			print('Error read address 0x%08x!' % addr)
			sys.exit(3)
		print('Flash 0x%08x: %s' % (addr,db.hex()))
		#print(phy._write_efuse(0, 0x00000000, 0x80000000)) # test!!!
		#print(phy._write_efuse(1, 0x9f6dcc0b, 0xfd530804)) # test!!!
		#print(phy._write_efuse(2, 0x00000000, 0x80000000)) # test!!!
		#print(phy._write_efuse(3, 0x00000000, 0x80000000)) # test!!!
		e = phy.read_all_efuse()
		if e == None:
			print('Error read efuse!')
			sys.exit(3)
		print("eFuse Lock Status: 0x%08x" % e[0])
		print("eFuse 0: %08x%08x" % (e[1], e[2]))
		print("eFuse 1: %08x%08x" % (e[3], e[4]))
		print("eFuse 2: %08x%08x" % (e[5], e[6]))
		print("eFuse 3: %08x%08x" % (e[7], e[8]))
		boot = e[9] >> 24
		print("Security Boot:", boot)
	if args.operation == 'rf':
		try:
			ff = open(args.filename, "wb")
		except:
			print("Error file open '%s'" % args.filename)
			exit(2)
		size = phy.ReadAllFlash(ff)
		if  size == None:
			ff.close()
			exit(4)
		#print
		print ('\r---------------------------------------------------------')
		byteSaved = (size + 3) & 0xfffffffc
		print("%.3f KBytes saved to file '%s'" % (byteSaved/1024, args.filename))
		ff.close()
	if args.operation == 'rc':
		if not phy.ExpFlashSize():
			exit(4)
		if args.filename == 'x':
			args.filename = "r%08x-%08x.bin" % (args.address, args.size)
		if args.size == 0:
			print("Read Size = 0!" )
			exit(1)
		try:
			ff = open(args.filename, "wb")
		except:
			print("Error file open '%s'" % args.filename)
			exit(2)
		if not phy.ReadBusToFile(ff, args.address, args.size):
			ff.close()
			exit(4)
		#print
		print ('\r---------------------------------------------------------')
		byteSaved = (args.size + 3) & 0xfffffffc
		if byteSaved > 1024:
			print("%.3f KBytes saved to file '%s'" % (byteSaved/1024, args.filename))
		else:
			print("%i Bytes saved to file '%s'" % (byteSaved, args.filename))
		ff.close()
	#--------------------------------Write bin file to Flash
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
		if not phy.SpifsInit():
			print ('Error: Spifs start init error!')
			sys.exit(2)
		aerase = args.operation == 'we'
		if args.erase == True or args.allerase == True:
			aerase = False
			if args.allerase == True:
				if not phy.cmd_erase_all_flash():
					stream.close()
					print ('Error: Erase All Flash!')
					sys.exit(3)
			else:
				if args.erase == True:
					if not phy.cmd_erase_work_flash():
						stream.close()
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
	#--------------------------------Write bin file to chip bus address
	if args.operation == 'wc':
		offset = args.address
		stream = open(args.filename, 'rb')
		size = os.path.getsize(args.filename)
		if size < 1:
			stream.close()
			print ('Error: File size = 0!')
			sys.exit(1)
		offset = args.address
		if size < 1:
			stream.close()
			print ('Error: Write File size = 0!')
			sys.exit(1)
		if not phy.SpifsInit():
			print ('Error: Spifs start init error!')
			sys.exit(2)
		if args.erase == True or args.allerase == True:
			if args.allerase == True:
				if not phy.cmd_erase_all_flash():
					stream.close()
					print ('Error: Erase All Flash!')
					sys.exit(3)
			else:
				if args.erase == True:
					if not phy.cmd_erase_work_flash():
						stream.close()
						print ('Error: Erase Flash!')
						sys.exit(3)
		print ('Write data 0x%08x to 0x%08x from file: %s ...' % (offset, offset + size, args.filename))
		if not phy.ExpFlashSize():
			exit(4)
		if size > 0:
			if not phy.WriteFileToBus(stream, offset, size):
				stream.close()
				print ('Error: Write data!')
				sys.exit(2)
		stream.close()
		print ('----------------------------------------------------------')
		print ('Write data 0x%08x to 0x%08x from file: %s - ok.' % (offset, offset + size, args.filename))
	#--------------------------------wr flash hex
	if args.operation == 'wh':
		hp = ParseHexFile(args.filename)
		if hp == None:
			sys.exit(2)
		hexf = phy.HexfHeader(hp, args.start, args.write)
		if hexf == None:
			sys.exit(2)
		hp[0][1] = hexf
		if not phy.SpifsInit():
			print ('Error: Spifs start init error!')
			sys.exit(2)
		print ('----------------------------------------------------------')
		aerase = True
		if args.erase == True or args.allerase == True:
			aerase = False
			if args.allerase == True:
				if not phy.cmd_erase_all_flash():
					print ('Error: Erase All Flash!')
					sys.exit(3)
			else:
				if args.erase == True:
					if not phy.cmd_erase_work_flash():
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
			if not phy.WriteBlockFlash(stream, ihp[2], len(ihp[1]), 0):
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
		if not phy.cmd_erase_all_flash():
			print ('Error: Erase All Flash!')
			sys.exit(3)
	if args.operation == 'ew':
		if not phy.cmd_erase_work_flash():
			print ('Error: Erase Flash Work Area!')
			sys.exit(3)
	if args.reset:
		phy.SendResetCmd()
		print ("Send command 'reset' - ok")
	sys.exit(0)

if __name__ == '__main__':
	main()
