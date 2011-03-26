#!/usr/bin/env python

import config

import os
import glob
import time
import urllib
import thread

def write_index(idx):
	msg_index = open(config.logdir + "/msg_index.tmp", "w")
	for i in sorted(idx):
		msg_index.write(i + "\n")
	msg_index.close()
	os.system("mv %s %s" % (config.logdir + "/msg_index.tmp", config.logdir + "/msg_index"))

def websync_thread():
	try:
		while True:
			os.system("python websync.py")
			time.sleep(10)
	except:
		thread.interrupt_main()

if __name__ == "__main__":
	idx = set()

	thread.start_new_thread(websync_thread, ())
	while True:
		new_idx = set()
		for f in glob.glob(config.logdir + "/" + "[0-9]" * 6):
			f = os.path.basename(f)
			new_idx.add(f)

		for f in glob.glob(config.logdir + "/" + "[0-9]" * 6 + ".unsent"):
			f = os.path.basename(f)
			new_idx.add(f)

		if new_idx != idx:
			if new_idx - idx:
				print "Adding:", new_idx - idx
			if idx - new_idx:
				print "Removing:", idx - new_idx
			idx = new_idx
			write_index(idx)
		time.sleep(0.1)
