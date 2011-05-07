#!/usr/bin/env python

import config
import utils

import sys

def web_index():
	return utils.http_get(config.msg_index_url).split()

def parse_index(index):
	l = []
	for f in index:
		l.append(f.strip())

	return l

def format_time(msg):
	return "%02d:%02d:%02d" % (int(msg[1:3]), int(msg[3:5]), int(msg[5:7]))

def format_msg(msg):
	out = ''
	if msg[0] == '/' and msg[1:7].isdigit() and msg[7] == 'h':
		lat = float(msg[8:10]) + float(msg[10:15])/60
		if msg[15] != 'N':
			lat *= -1
		lon = float(msg[17:20]) + float(msg[20:25])/60
		if msg[25] != 'E':
			lon *= -1

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
	print "Time     Latitude   Longitude   Altitude T.Ext Pressure Humidity T.Int VBatt Acceler. HADARP Link"
	for m in reversed(local[-lastmsg:]):
		msg = open(config.logdir + "/" + m).read().strip()
		print format_msg(msg)
