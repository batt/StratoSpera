#!/usr/bin/env python

import config
import os
import tempfile
import shutil
import urllib
import signal
import thread
import time
import glob


def write_file(name, data):
	fd, tmp_name = tempfile.mkstemp()
	os.write(fd, data)
	os.close(fd)
	shutil.move(tmp_name, name)

#This separated thread is needed because networks has looooong timeouts
#and some underlaying function also blocks all signals, even ctrl+c!
def send_thread(addr, data, reply):
	try:
		f = urllib.urlopen(addr, data)
		reply.append(f.read())
	except:
		thread.interrupt_main()

def http_get(url, data=None):
	if data:
		data = urllib.urlencode(data)

	def timeout_handler(signum, frame):
		print "Network timeout"
		exit(-1)

	#Gracefully handle network timeouts
	signal.signal(signal.SIGALRM, timeout_handler)

	reply = []
	signal.alarm(config.net_timeout)

	thread.start_new_thread(send_thread, (url, data, reply))
	while len(reply) == 0:
		time.sleep(0.1)

	signal.alarm(0)

	return reply[0]

def update_index(directory):
	msg_index = []
	for msg in glob.glob(directory + "/" + "[0-9]" * 6):
		msg_index.append(os.path.basename(msg))
	msg_index.sort()
	msg_index = '\n'.join(msg_index)
	write_file(directory + "/" + config.msg_index, msg_index)
