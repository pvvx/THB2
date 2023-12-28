#!/usr/bin/env python3

# rdreg_phy6202.py 30.11.2019 pvvx #

import serial;
import time;
import argparse
import os
import struct
import sys

__version__ = "23.11.22"

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
	parser = argparse.ArgumentParser(description='RdRegs-PHY6222 Utility version %s' % __version__, prog='rdreg_phy6222')
	parser.add_argument(
		'--port', '-p',
		help='Serial port device',
		default='COM1');
	parser.add_argument(
		'--baud', '-b',
		help='Set Port Baud (115200, 250000, 500000, 1000000), default: 1000000',
		type=arg_auto_int,
		default=1000000);
	parser.add_argument('address', help='Start address', type=arg_auto_int)
	parser.add_argument('size', help='Size of region to dump', type=arg_auto_int)
	
	args = parser.parse_args()

	baud = 9600;
	print('RdRegs-PHY62x2 Utility version %s' % __version__)
	try:
		serialPort = serial.Serial(args.port, baud, \
								   serial.EIGHTBITS,\
								   serial.PARITY_NONE, \
								   serial.STOPBITS_ONE);
	except:
		print('Error: Open %s, %d baud!' % (args.port, baud))
		sys.exit(2)

	serialPort.setDTR(False) #TM   (lo)
	serialPort.setRTS(True) #RSTN (lo)
	serialPort.flushOutput()
	serialPort.flushInput()
	time.sleep(0.1)
	serialPort.setDTR(True) #TM  (hi)
	serialPort.flushOutput()
	serialPort.flushInput()
	time.sleep(0.1)
	serialPort.setRTS(False) #RSTN (hi)
	serialPort.timeout = 0.04

#--------------------------------

	byteSent = 0;
	byteRead = 0;
	byteSaved = 0;

	addr = args.address;
	length = args.size;

	ttcl = 250;
	pkt = 'UXTDWU' # UXTL16 UDLL48 UXTDWU
	while ttcl > 0:
		sent = serialPort.write(pkt.encode());
		byteSent += sent
		read = serialPort.read(6);
		byteRead += len(read);
		if read == b'cmd>>:' :
			break
		ttcl = ttcl - 1
		if ttcl < 1:
			print('PHY62x2 - Error Reset!')
			print('Check connection TX->RX, RX<-TX and Chip Power!')
			serialPort.close()
			exit(4)

	print('PHY62x2 - Reset Ok')
	serialPort.close()
	serialPort.baudrate = 115200
	serialPort.open();
	serialPort.timeout = 0.2

	pkt = 'wrreg4000f054 0 ';
	sent = serialPort.write(pkt.encode());
	byteSent += sent;
	read = serialPort.read(6);
	byteRead += len(read);
	if read != b'#OK>>:' :
		print('PHY62x2 - Error init1!')
		serialPort.close()
		exit(4)

	pkt = 'wrreg4000f140 0 ';
	sent = serialPort.write(pkt.encode());
	byteSent += sent;
	read = serialPort.read(6);
	byteRead += len(read);
	if read != b'#OK>>:' :
		print('PHY62x2 - Error init2!')
		serialPort.close()
		exit(4)

	pkt = 'wrreg4000f144 0 ';
	sent = serialPort.write(pkt.encode());
	byteSent += sent;
	read = serialPort.read(6);
	byteRead += len(read);
	if read != b'#OK>>:' :
		print('PHY62x2 - Error init3!')
		serialPort.close()
		exit(4)

	if baud != args.baud:
		baud = args.baud;
		print('Reopen %s port %i baud' % (args.port, baud))
		pkt = "uarts%i" % baud
		sent = serialPort.write(pkt.encode());
		byteSent += sent;
		serialPort.timeout = 1
# 012
# #OK
		read = serialPort.read(3);
		#print(read);
		serialPort.close()
		serialPort.baudrate = baud
		serialPort.open();
	
	serialPort.write(str.encode("rdrev"));
	serialPort.timeout = 0.1
	read = serialPort.read(16);
	if read[0:2] == b'0x' and read[10:16] == b'#OK>>:':
		print('Revision:', read[0:10])
	else:
		print('Error read Revision!')
		exit(2)
	print('Start address: 0x%08x, length: 0x%08x' % (addr, length))

	filename = "r%08x-%08x.bin" % (addr, length)
	try:
		ff = open(filename, "wb")
	except:
		serialPort.close()
		print('Error file open ' + filename)
		exit(2)
		
	t1 = time.time()
	while length > 0:
		if args.size > 128 and addr&127 == 0:
			print('\rRead 0x%08x...' % addr, end='') #, flush=True
		txt = "rdreg%08x" % addr;
		sent = serialPort.write(txt.encode());
		byteSent += sent;
# 01234567890123456
# =0x1fff3710#OK>>:		
		read = serialPort.read(17);
		byteRead += len(read);
		if read[0:3] == b'=0x' and read[11:17] == b'#OK>>:':
			dw = struct.pack('<I', int(read[1:11], 16))
			ff.write(dw);
			byteSaved +=len(dw);
		else:
			t2 = time.time()
			print('\r  Time: %.3f sec' % (t2-t1))
			print('Writes: %d Bytes' % byteSent)
			print(' Reads: %d Bytes' % byteRead)
			print
			print('\rError read address 0x%08x!' % addr)
			serialPort.close()
			ff.close()
			exit(1);
		addr += 4
		length -= 4
	t2 = time.time()
	serialPort.close()
	print('\r  Time: %.3f sec' % (t2-t1))
	print('Writes: %d Bytes' % byteSent)
	print(' Reads: %d Bytes' % byteRead)
	print
	if byteSaved > 1024:
		print("%.3f KBytes saved to file '%s'" % (byteSaved/1024, filename))
	else:
		print("%i Bytes saved to file '%s'" % (byteSaved, filename))
	ff.close()
	exit(0);

if __name__ == '__main__':
	main()
