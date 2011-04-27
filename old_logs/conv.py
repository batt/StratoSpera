#!/usr/bin/python

import sys

f = open(sys.argv[1])
#skip header
f.readline()
f.readline()
for l in f:
	d = l.split(';')
	t, fix, lat, lon, alt, _, _, press, _ = d
	t = t.split(':')
	t = int(t[0]) * 3600 + int(t[1]) * 60 + int(t[2])
	fix = 1 if fix == 'FIX' else 0
	lat = float(lat)
	lon = float(lon)
	alt = int(alt)
	press = int(press)
	print "%d;%d;%f;%f;%d;%d" % (t, fix, lat, lon, alt, press)
