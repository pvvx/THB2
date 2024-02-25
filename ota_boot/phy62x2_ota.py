#!/usr/bin/env python3

# phy62x2_ota.py 22.01.2024 pvvx #

import argparse
import io
import os
import struct
import sys
import zlib

START_UP_FLAG = 0x36594850 #"PHY6"

MAX_FLASH_SIZE = 0x200000
EXT_FLASH_ADD = 0x400000

DEF_START_RUN_APP_ADDR = 0x1FFF1838
DEF_START_WR_FLASH_ADDR = 0x010000

PHY_FLASH_SECTOR_SIZE = 4096
PHY_FLASH_SECTOR_MASK = 0xfffff000
PHY_WR_BLK_SIZE = 0x2000

__progname__ = 'PHY62x2 OTA Utility'
__filename__ = 'phy62x2_ota.py'
__version__ = "25.02.24"

def do_crc(s, c):
	return zlib.crc32(s, c) & 0xffffffff

class phy_ota:
	def ParseHexFile(self, hexfile, faddr = DEF_START_WR_FLASH_ADDR):
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
		table.append([0, result, faddr])
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
	def WriteHexf(self, sn, ph):
		offset = ph[2]
		offset &= 0x00ffffff
		idx = 0
		size = len(ph[1])
		return True
	def HexfHeader(self, hp, start = DEF_START_RUN_APP_ADDR, wrfaddr = DEF_START_WR_FLASH_ADDR, otaid = START_UP_FLAG):
		if len(hp) > 1:
			if len(hp) > 15:
				print('Maximum number of segments = 15!')
				return None
			size = 0x100
			sections = 15
			wfaddr = (wrfaddr & (MAX_FLASH_SIZE-1)) + size
			wrfaddr = wfaddr; 
			faddr_min = MAX_FLASH_SIZE-1 # xip addr min
			faddr_max = 0 # xip addr max
			rsize = 0 # size ram data
			hexf = bytearray(struct.pack('<IIII', otaid, len(hp)-1, start, 0xffffffff))
			for ihp in hp:
				if (ihp[0] & 0x1fff0000) == 0x1fff0000:	# SRAM
					rsize += len(ihp[1])
				elif (ihp[0] & (~(MAX_FLASH_SIZE-1))) == 0x11000000: # Flash (XIP)
					offset = ihp[0] & (MAX_FLASH_SIZE-1)
					if faddr_min > offset:
						faddr_min = offset
					send = offset + len(ihp[1])
					if faddr_max <= send:
						faddr_max = send
			if (wrfaddr + rsize) >= faddr_min:  
				wrfaddr = (faddr_max + 3) & 0xfffffffc # start wr faddr ram data
			#print('Test: Flash addr min: %08x, max: %08x, RAM addr: %08x' % (faddr_min, faddr_max, wrfaddr))
			print ('---- Segments Table -------------------------------------')
			
			for ihp in hp:
				if (ihp[0] & 0x1fff0000) == 0x1fff0000:	# SRAM
					faddr = wrfaddr # wr faddr ram data
					wrfaddr += (len(ihp[1])+3) & 0xfffffffc # next wr faddr ram data
				elif (ihp[0] & (~(MAX_FLASH_SIZE-1))) == 0x11000000: # Flash
					faddr = ihp[0] & (MAX_FLASH_SIZE-1)
					if wfaddr != faddr:
						print('Error: The segment Flash addr: 0x%08x, Size: 0x%08x does not match the markup (0x%08x)!' % (ihp[0], len(ihp[1]), wfaddr))
						return None
				elif ihp[0] == 0:
					continue
				else:
					print('Error: Invalid Segment Address 0x%08x!' % ihp[0])
					return None
				ihp[2] = faddr
				sections -= 1
				print('Segment: %08x <- Flash addr: %08x, Size: %08x' % (ihp[0], faddr, len(ihp[1])))
				crc = 0xffffffff - do_crc(ihp[1], 0)
				hexf.extend(bytearray(struct.pack('<IIII', faddr, len(ihp[1]), ihp[0], crc)))
				fill = len(ihp[1]) % 4
				if fill != 0:
					ihp[1].extend(bytearray(b'\xff')*(4 - fill))
				size += len(ihp[1])
				wfaddr += len(ihp[1])
			if sections > 0:
				hexf.extend(bytearray(b'\xff')*(0x10*sections))
			fill = size % 16
			if fill != 0:
				hp[len(hp)-1][1].extend(bytearray(b'\xff')*(16 - fill))
				size += 16 - fill
			hexf[12:16] = int.to_bytes(size, 4, byteorder='little')
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
	parser.add_argument('--idota', '-i',  help = 'Flag ID OTA (default: 0x%08x)' % START_UP_FLAG, type = arg_auto_int, default = START_UP_FLAG);
	parser.add_argument('--runaddr', '-r',  help = 'Application run-start address (default: 0x%08x)' % DEF_START_RUN_APP_ADDR, type = arg_auto_int, default = DEF_START_RUN_APP_ADDR);
	parser.add_argument('--wraddr', '-w',  help = 'Application write address (default: 0x%08x)' % DEF_START_WR_FLASH_ADDR, type = arg_auto_int, default = DEF_START_WR_FLASH_ADDR);
	parser.add_argument('--fbup', '-f',  help = 'Boot update file (default: none)', default = None);
	parser.add_argument('--outfile', '-o',  help = 'Output bin file')
	parser.add_argument('filename', help = 'Name of hex file')

	args = parser.parse_args()

	print('=========================================================')
	print('%s version %s' % (__progname__, __version__))
	print('---------------------------------------------------------')
	
	if args.fbup != None:
		try:
			fbup = open(args.fbup, 'rb')
		except Exception as e:
			print('Error: Open file %s, %s' % (args.fbup, e))
			exit(1)
		try:
			datafbup = fbup.read();	
		except Exception as e:
			print('Error: Read file %s,: %s' % (args.fbup, e))
			exit(1)
		fbup.close()
		fbupid = int.from_bytes(datafbup[0:4], byteorder='little')
		fbuplen = int.from_bytes(datafbup[12:16], byteorder='little')
		print("Boot update file id: %08x:%08x" %(fbupid, fbuplen))
		if (fbupid != START_UP_FLAG) or ((fbuplen + 4) != len(datafbup)) or (fbuplen > 4096) or (fbuplen <= 256):
			print('Error: Boot update file %s format!' % args.fbup)
			exit(2)
		datafbup = bytearray(datafbup[0:-4])

	phy = phy_ota()

	hp = phy.ParseHexFile(args.filename, args.wraddr)

	if hp == None:
		sys.exit(3)
	hexf = phy.HexfHeader(hp, args.runaddr, args.wraddr, args.idota)
	if hexf == None:
		sys.exit(4)
	hp[0][1] = hexf
				 
	print ('---- File Structure -------------------------------------')
	(outfile, ext) = os.path.splitext(args.filename)
	outfile += '.bin'
	if args.outfile != None:
		outfile = args.outfile
	try:
		fout = open(outfile, 'wb')
	except:
		print('No file opened', outfile)
		sys.exit(5)
	fsize = 0
	for ihp in hp:
		 fsize += len(ihp[1])
	segment = 0
	crc = 0
	try:
		if args.fbup != None:
			#print('size: %08x , %08x' % (fsize, fbuplen))
			datafbup[12:16] = int.to_bytes(fsize + fbuplen, 4, byteorder='little')
			datafbup[0xfc:0x100] = int.to_bytes((fbuplen + DEF_START_WR_FLASH_ADDR) | 0x11000000, 4, byteorder='little')
			crc = do_crc(datafbup, crc)
			fout.write(datafbup)
		for ihp in hp:
			if ihp[0] == 0:
				print('Segment Table[%02d] <- Flash addr: %08x, Size: %08x' % (len(hp) - 1, ihp[2], len(ihp[1])))
			else:
				print('Segment: %08x <- Flash addr: %08x, Size: %08x' % (ihp[0], ihp[2], len(ihp[1])))
			fout.write(ihp[1])
			crc = do_crc(ihp[1], crc)
			segment += 1
		crc = 0xffffffff - crc
		#print('CRC32: %04x' % crc)
		fout.write(bytearray(struct.pack('<I', crc)))
		size = fout.tell()
		fout.close()
	except Exception as e:
		print('No write file %s, %s' % (outfile, e))
		sys.exit(6)
	print ('---------------------------------------------------------')
	print ('Write to file: %s %u bytes - ok.' % (outfile, size))
	sys.exit(0)

if __name__ == '__main__':
	main()
