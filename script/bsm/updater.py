#!/usr/bin/env python

import config

import glob
import os
import hmac
import urllib
import signal
import thread
import time

#This separated thread is needed because networks has looooong timeouts
#and some underlaying function also blocks all signals, even ctrl+c!
def send_thread(addr, data, reply):
	f = urllib.urlopen(addr, data)
	reply.append(f.read())

def send_server(msg, name):
	print "Sending:", msg

	m = hmac.new(config.password, msg)
	sign = m.hexdigest()
	d = {}
	d['m'] = msg
	d['s'] = sign
	d['n'] = name
	data = urllib.urlencode(d)

	reply = []
	signal.alarm(config.net_timeout)

	thread.start_new_thread(send_thread, (config.url, data, reply))
	while len(reply) == 0:
		time.sleep(0.1)

	signal.alarm(0)

	return reply[0]

def timeout_handler(signum, frame):
	print "Network timeout"
	exit(-1)

#Gracefully handle network timeouts
signal.signal(signal.SIGALRM, timeout_handler)

if __name__ == "__main__":
	for unsent in glob.glob(config.logdir + "/*.unsent"):
		u = open(unsent, "r+")
		msg = u.read().strip()
		new_name = unsent.rstrip('.unsent')
		reply = send_server(msg, os.path.basename(new_name))

		sent = True

		if reply.startswith("OK"):
			print "OK"
		elif reply.startswith("ERR"):
			u.write(reply)
			new_name = new_name + ".err"
			print "Message not accepted by server, see", new_name
		else:
			print "Unknow error, will retry next time"
			sent = False

		u.close()
		if (sent):
			os.system("mv %s %s" % (unsent, new_name))
