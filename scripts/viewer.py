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

def msg_telemetry(msg):
	return msg[0] == '/' and msg[1:7].isdigit() and msg[7] == 'h'

def format_time(msg):
	return "%02d:%02d:%02d" % (int(msg[1:3]), int(msg[3:5]), int(msg[5:7]))

def msg_time(msg):
	return int(msg[1:3]) * 3600 +  int(msg[3:5]) * 60 + int(msg[5:7])

def msg_alt(msg):
	alt, _, _, _, _, _, _, _ = map(float, msg[27:].split(';'))
	return alt

def format_msg(msg):
	out = ''
	if msg_telemetry(msg):
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
	local = local[-lastmsg:]

	start_msg = open(config.logdir + "/" + local[0]).read().strip()
	end_msg = open(config.logdir + "/" + local[-1]).read().strip()
	try:
		start_alt = msg_alt(start_msg)
		start_time = msg_time(start_msg)
		end_alt = msg_alt(end_msg)
		end_time = msg_time(end_msg)
		ascent_rate = (end_alt - start_alt) / (end_time - start_time)
		#print start_time, end_time
		print "Ascent rate %.2f m/s" % ascent_rate
	except:
		pass
	print "Time     Latitude   Longitude   Altitude T.Ext Pressure Humidity T.Int VBatt Acceler. HADARP Link"
	for m in reversed(local):
		msg = open(config.logdir + "/" + m).read().strip()
		print format_msg(msg)



