#!/usr/bin/env python

import sys
import serial
import time
from datetime import datetime
from datetime import timedelta

path = [
	#Time(s), lat, lon, alt(m)
	[0,    43.606414, 11.311832, 250],
	[3600, 43.500752, 11.932526, 40000],
	[3600 + 40*60, 43.777911, 12.057495, 1000],
]

if len(sys.argv) < 2:
	out = sys.stdout
else:
	out = serial.Serial(sys.argv[1], baudrate=4800)

end_time = path[-1][0]
path_idx = 0
start = datetime.today()
clock = start

for i in range(end_time):
	if i >= path[path_idx+1][0]:
		path_idx +=1
	prev_t = path[path_idx][0]
	next_t = path[path_idx+1][0]
	delta_t = next_t - prev_t

	prev_lat = path[path_idx][1]
	next_lat = path[path_idx+1][1]
	delta_lat = next_lat - prev_lat

	prev_lon = path[path_idx][2]
	next_lon = path[path_idx+1][2]
	delta_lon = next_lon - prev_lon

	prev_alt = path[path_idx][3]
	next_alt = path[path_idx+1][3]
	delta_alt = next_alt - prev_alt

	now = float(i - prev_t) / delta_t

	curr_lat = now * delta_lat + prev_lat
	curr_lon = now * delta_lon + prev_lon
	curr_alt = now * delta_alt + prev_alt
	h = clock.timetuple()[3]
	m = clock.timetuple()[4]
	s = clock.timetuple()[5]
	'''
	h=12
	m=35
	s=19
	curr_lat = 48.1173
	curr_lon = 11.516667
	curr_alt=545.4
	'''
	t = "%02d%02d%02d" % (h,m,s)
	lat = "%02d%06.3f,%s" % (int(curr_lat), (curr_lat - int(curr_lat)) * 60.0, 'N' if curr_lat > 0 else 'S')
	lon = "%03d%06.3f,%s" % (int(curr_lon), (curr_lon - int(curr_lon)) * 60.0, 'E' if curr_lat > 0 else 'W')
	nmea_str = "GPGGA,%s,%s,%s,1,08,0.9,%.1f,M,46.9,M,," % (t, lat, lon, curr_alt)
	chks = 0
	for c in nmea_str:
		chks ^= ord(c)
	nmea_str = "$%s*%02X\r\n" % (nmea_str, chks)
	out.write(nmea_str)
	sys.stderr.write(nmea_str)
	clock = start + timedelta(seconds=i+1)
	time.sleep(1)

