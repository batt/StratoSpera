#!/usr/bin/env python

import serial
import sys
from time import strftime


if __name__ == "__main__":
    import argparse
    p = argparse.ArgumentParser(description='usage: %prog [options]')

    p.add_argument('-p', '--port', nargs=1, default='/dev/ttyUSB0',type=str, help="Serial port.")
    p.add_argument("-b", "--baudrate", nargs=1, type=int, default="115200", help="Serial baudrate.")
    p.add_argument("-t", "--timeout", nargs=1, type=float, default=None, help="Read timeout in sec.")

    args = p.parse_args()
    print "Open serial port %s at %d baudrate..\n" % (args.port, args.baudrate)
    print args

    if type(args.port) == list:
        args.port = args.port[0]

    try:
        s = serial.Serial(args.port, bytesize=8, parity='N', stopbits=1, baudrate=args.baudrate, timeout=args.timeout)
    except ValueError, e:
        print e
        exit (1)

    s.flushInput()

    for line in s:
        sys.stdout.write("%s,%s" % (strftime("%d/%m/%Y,%H:%M:%S"), line))
        sys.stdout.flush()
