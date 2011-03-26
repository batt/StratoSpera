#!/usr/bin/env python

import updater
import time

delay = 10
real_delay = 0


log = open("log00006.csv")
#remove headers
log.readline()
log.readline()
start = 0
for l in log:
	tim, fix, lat, lon, alt, t_int, t_ext, press, vsupply = l.strip().split(';')
	h, m, s = tim[0:2], tim[3:5], tim[6:8]
	seconds = int(h) * 3600 + int(m) * 60 + int(s)
	if seconds >= start + delay:
		if start == 0:
			start = seconds
		else:
			start += delay
		lat = float(lat)
		dlat = int(lat)
		mlat = (lat - dlat) * 60
		ns = 'N' if lat > 0 else 'S'
		lon = float(lon)
		dlon = int(lon)
		mlon = (lon - dlon) * 60
		ew = 'E' if lon > 0 else 'W'
		msg =  "/%sh%02d%02.2f%c/%03d%02.2f%c>" % (h+m+s, dlat, mlat, ns, dlon, mlon, ew)
		msg = msg + alt + ';' + t_ext + ';' + press + ';0;' + t_int + ';' + vsupply + ';0;0'
		updater.send_server(msg)
		time.sleep(real_delay)
