#!/usr/bin/env python

import os
import time
import pickle
import urllib
import hmac
import sys

#Settings
logfile = "bsm.log"
sender = "IZ1DYB-9"
url = "http://www.develer.com/~batt/stratospera/bsm-2/add.cgi"
unsentfile = "unsent.pic"
password = "stsp2"
#end of settings

def send_server(msg):
	try:
		m = hmac.new(password, msg)
		sign = m.hexdigest()
		d = {}
		d['m'] = msg
		d['s'] = sign
		f = urllib.urlopen(url, urllib.urlencode(d))
		if f.read(2) != "OK":
			raise IOError
	except IOError:
		return False

	return True

def update_unsent(msg, file):
	file.seek(0)
	pickle.dump(msg, file)
	file.flush()

def parse_loop():
	#start modem
	os.system("script -afc \"padsp multimon -a afsk1200\" %s&" % logfile)

	try:
		#open modem logging file
		file = open(logfile,'r')

		#Open unsent messages file
		try:
			picfile = open(unsentfile, 'r+')
			messages = pickle.load(picfile)
		except (IOError, EOFError):
			picfile = open(unsentfile, 'w+')
			messages = []

		#Move to the logfile end
		file.seek(0, os.SEEK_END);

		wait_start = True
		while 1:
			d = file.readline()
			if not d:
				#if there are messages in queue send them starting from
				#the newers.
				if len(messages) > 0:
					if (send_server(messages[-1])):
						print "Message sent:", messages.pop()
						update_unsent(messages, picfile)
					else:
						print "Network error, retrying..."
				else:
					time.sleep(0.5)
			else:
				if wait_start:
					#Check for correct sender address
					if d.startswith("AFSK1200: fm %s" % sender):
						wait_start = False
				else:
					wait_start = True
					d = d.strip()
					if d not in messages:
						messages.append(d)
						update_unsent(messages, picfile)

	except KeyboardInterrupt:
		print "\nCTRL-C pressed, exit"
	finally:
		os.system("killall script")
		picfile.close()

def auth_test(start):
	min = start
	for i in range(60):
		send_server(">%04d00zTest %d" % (min, i))
		min += 1

if __name__ == "__main__":
	if len(sys.argv) > 1 and sys.argv[1].isdigit():
		auth_test(int(sys.argv[1]))
	else:
		parse_loop()

