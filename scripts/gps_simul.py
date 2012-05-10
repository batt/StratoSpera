#!/usr/bin/env python

import sys
import serial
import time
from datetime import datetime
from datetime import timedelta
import argparse

def gps_gga(h, m, s, lat, lon, alt, fix=True):
	t = "%02d%02d%02d" % (h,m,s)
	lat = "%02d%07.4f,%s" % (abs(int(lat)), abs((lat - int(lat)) * 60.0), 'N' if lat > 0 else 'S')
	lon = "%03d%07.4f,%s" % (abs(int(lon)), abs((lon - int(lon)) * 60.0), 'E' if lon > 0 else 'W')
	nmea_str = "GPGGA,%s,%s,%s,%d,08,0.9,%.1f,M,46.9,M,," % (t, lat, lon, 1 if fix else 0, alt)
	chks = 0
	for c in nmea_str:
		chks ^= ord(c)
	nmea_str = "$%s*%02X\r\n" % (nmea_str, chks)
	return nmea_str

def gen_gga(t_start, slat, elat, slon, elon, salt, ealt, delta_t, out, delay=1, fix=True):
	delta_lat = elat-slat
	delta_lon = elon-slon
	delta_alt = ealt-salt
	for i in range(delta_t):
		now = float(i) / delta_t
		curr_lat = now * delta_lat + slat
		curr_lon = now * delta_lon + slon
		curr_alt = now * delta_alt + salt
		h = t_start.timetuple()[3]
		m = t_start.timetuple()[4]
		s = t_start.timetuple()[5]
		nmea_str = gps_gga(h, m, s, curr_lat, curr_lon, curr_alt, fix)
		out.write(nmea_str)
		sys.stderr.write(nmea_str)
		t_start = t_start + timedelta(seconds=1)
		time.sleep(delay)


def gga_simple_path(path, out, delay=1):
	start = datetime.today()
	prev_p = None
	for p in path:
		if prev_p == None:
			prev_p = p
			continue

		gen_gga(start, prev_p[1], p[1], prev_p[2], p[2], prev_p[3], p[3], p[0], out, delay)
		start = start + timedelta(seconds=p[0])
		prev_p = p

def gga_real_data(csv, out, delay=1):
	prev_t = None
	with open(csv) as data:
		for l in data:
			l = l.split(';')
			if len(l) < 5:
				continue
			t = l[0]
			t = t.split(':')
			if len(t) != 3:
				continue
			h, m, s = map(int, t)
			t = h*3600 + m*60 + s
			fix = l[1]
			lat, lon, alt = map(float, l[2:5])
			if prev_t == None:
				prev_t = (h,m,s)
				prev_lat = lat
				prev_lon = lon
				prev_alt = alt
				prev_fix = fix
				continue

			start = datetime.today()
			start = start.replace(hour=prev_t[0], minute=prev_t[1], second=prev_t[2])
			prev_t = prev_t[0]*3600 + prev_t[1]*60 + prev_t[2]
			delta_t = min(t-prev_t, 3)
			if (prev_fix == 'FIX' and fix != 'FIX'):
				lat = prev_lat
				lon = prev_lon
				alt = prev_alt
			gen_gga(start, prev_lat, lat, prev_lon, lon, prev_alt, alt, delta_t, out, delay, prev_fix=='FIX')
			prev_t = (h,m,s)
			prev_lat = lat
			prev_lon = lon
			prev_alt = alt
			prev_fix = fix

parser = argparse.ArgumentParser(description="""
Emulates a GPS, sending GGA strings to a serial port.
It can replay a real csv file or can generate GGA strings based on a default path.
""")

parser.add_argument("--port", type=str, help="Serial port, if not specified, strings are sent to stdout.")
parser.add_argument("--baudrate", type=int, help="Serial port baudrate.", default=4800)
parser.add_argument("--csv", type=str, help="CSV file used as data source. If not specified use an internal default path.")
parser.add_argument("--delay", type=float, help="Delay between messages, usually 1s but you can increase it to speed up replay.", default=1.0)

args = parser.parse_args()

if args.port:
	sys.stderr.write("Using serial port:%s.\n" % args.port)
	out = serial.Serial(args.port, baudrate=args.baudrate)
else:
	sys.stderr.write("Using stdout for output.\n")
	out = sys.stdout

default_path = [
	#delay(s), lat, lon, alt(m)
	[0, 43.606407, 11.311953, 236], #start, standby
	[3*60, 43.606467, 11.311912, 241], #after 3 minutes, takeoff
	[2*3600 + 11*60, 43.76578, 11.817202, 39614], #after 2h and 11m of flight, burst
	[38*60, 43.794142, 12.030417, 990], #38 minutes after burst, landing
	[2*3600, 43.794295, 12.030065, 993], #2 hours after landing, turned off
]

if args.csv:
	sys.stderr.write("Using file '%s' as datasource.\n" % args.csv)
	gga_real_data(args.csv, out, args.delay)
else:
	sys.stderr.write("Using internal default path as datasource.\n")
	gga_simple_path(default_path, out, args.delay)
