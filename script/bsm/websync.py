#!/usr/bin/env python

import config

import os
import glob
import time
import signal
import thread
import urllib

def get_thread(url, reply):
	reply.append(urllib.urlopen(url).read())


def get(url):
	reply = []
	signal.alarm(config.net_timeout)

	thread.start_new_thread(get_thread, ("http://www.develer.com/~batt/stratospera/bsm-2/" + url, reply))
	while len(reply) == 0:
		time.sleep(0.1)

	signal.alarm(0)

	return reply[0]

def get_webidx():
	s = set()
	w = get("msg_index").split()
	for f in w:
		s.add(f.strip())

	return s

if __name__ == "__main__":
	local = set()

	try:
		msg_index = open(config.logdir + "/msg_index")
		for f in msg_index:
			local.add(f.strip())
	except IOError:
		pass

	web = get_webidx()
	diff = web - local
	for d in diff:
		print "Getting", d
		msg = get(d)
		f_msg = open(config.logdir + "/webmsg.tmp", "w")
		f_msg.write(msg)
		f_msg.close()
		os.system("mv %s %s" % (config.logdir + "/webmsg.tmp", config.logdir + "/" + d))
