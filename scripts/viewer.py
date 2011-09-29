#!/usr/bin/env python

import config
import utils
import math

import sys

def web_index():
	return utils.http_get(config.msg_index_url).split()

def parse_index(index):
	l = []
	for f in index:
		l.append(f.strip())

	return l

def msg_telemetry(msg):
	return msg[0] == '/' and msg[1:7].isdigit() and msg[7] == 'h'

def format_time(msg):
	return "%02d:%02d:%02d" % (int(msg[1:3]), int(msg[3:5]), int(msg[5:7]))

def msg_time(msg):
	return int(msg[1:3]) * 3600 +  int(msg[3:5]) * 60 + int(msg[5:7])

def msg_alt(msg):
	alt, _, _, _, _, _, _, _ = map(float, msg[27:].split(';'))
	return alt

def msg_lat(msg):
	lat = float(msg[8:10]) + float(msg[10:15])/60
	if msg[15] != 'N':
		lat *= -1
	return lat

def msg_lon(msg):
	lon = float(msg[17:20]) + float(msg[20:25])/60
	if msg[25] != 'E':
		lon *= -1
	return lon

def deg2rad(deg):
	return deg * math.pi / 180

def distance(lat1, lon1, lat2, lon2):
	PLANET_RADIUS = 6371000
	d_lat = deg2rad(lat2 - lat1)
	d_lon = deg2rad(lon2 - lon1)
	a = math.sin(d_lat / 2) * math.sin(d_lat / 2) + math.cos(deg2rad(lat1)) * math.cos(deg2rad(lat2)) * math.sin(d_lon / 2) * math.sin(d_lon / 2)
	c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
	return PLANET_RADIUS * c

def format_msg(msg):
	out = ''
	if msg_telemetry(msg):
		lat = msg_lat(msg)
		lon = msg_lon(msg)

		alt, t_ext, press, hum, t_int, vbatt, acc, hadarp = map(float, msg[27:].split(';'))
		url = 'http://maps.google.it/maps?q=lat,lon'
		url = url.replace('lat', "%.6f" % lat)
		url = url.replace('lon', "%.6f" % lon)

		out = "%s %-10.6f %-11.6f %-8.0f %-5.1f %-8.0f %-8.0f %-5.1f %-5.2f %-8.2f %-6.0f %s" % (format_time(msg),
		lat, lon,
		alt, t_ext, press, hum, t_int, vbatt, acc, hadarp, url)

	elif msg[0] == '>' and msg[1:7].isdigit() and msg[7] == 'h':
		out = "%s %s" % (format_time(msg), msg[8:])
	else:
		out = "%s" % msg

	return out


if __name__ == "__main__":
	try:
		l = open(config.logdir + "/" + config.msg_index, "r")
		local = parse_index(l)
		l.close()
	except IOError:
		local = []

	try:
		lastmsg = int(sys.argv[1])
	except:
		lastmsg = 10

	local.sort()
	local = local[-lastmsg:]

	try:
		start_msg = open(config.logdir + "/" + local[-2]).read().strip()
		end_msg = open(config.logdir + "/" + local[-1]).read().strip()
		lat = msg_lat(end_msg)
		lon = msg_lon(end_msg)
		dist = distance(lat, lon, config.start_lat, config.start_lon)

		start_alt = msg_alt(start_msg)
		start_time = msg_time(start_msg)
		end_alt = msg_alt(end_msg)
		end_time = msg_time(end_msg)
		ascent_rate = (end_alt - start_alt) / (end_time - start_time)
		#print start_time, end_time
		print "Vertical speed: %.2f m/s | %.0f km/h" % (ascent_rate, ascent_rate * 3.6)
		print "Distance from base: %.1f km" % (dist/1000)
	except:
		pass
	print "Time     Latitude   Longitude   Altitude T.Ext Pressure Humidity T.Int VBatt Acceler. HADARP Link"
	for m in reversed(local):
		msg = open(config.logdir + "/" + m).read().strip()
		print format_msg(msg)



