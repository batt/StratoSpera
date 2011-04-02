#!/usr/bin/env python

import config

import os
import time
import sys
import thread
import glob
from datetime import datetime
import utils

def uploader_thread():
	try:
		while True:
			time.sleep(0.1)
			os.system("python uploader.py")
	except:
		thread.interrupt_main()

def downloader_thread():
	try:
		while True:
			time.sleep(30)
			os.system("python downloader.py")
	except:
		thread.interrupt_main()


def parse_loop():
	logfile = config.logdir + "/multimon.log"
	os.system("mkdir -p " + config.logdir)
	os.system("touch " + logfile)
	#start multimon
	os.system("./aprs_decoder >%s&" % logfile)

	try:
		#start web updaters
		thread.start_new_thread(uploader_thread, ())
		thread.start_new_thread(downloader_thread, ())

		#open multimon logging file
		file = open(logfile,'r')

		wait_start = True
		while 1:
			where = file.tell()
			d = file.readline()
			if not d:
					time.sleep(0.1)
					file.seek(where)
			else:
				if wait_start:
					#Check for correct config.sender address
					if d.startswith("AFSK1200: fm %s" % config.sender.upper()):
						wait_start = False
				else:
					wait_start = True
					if d[1:7].isdigit() and d[7] == 'h':
						name = d[1:7]
					elif config.log_all_messages:
						now = datetime.utcnow()
						name = "%02d%02d%02d" % (now.hour, now.minute, now.second)
					else:
						print "Unhandled message:", d.strip()
						continue

					msg_name = config.logdir + "/" + name

					found = False
					for msg in glob.glob(config.logdir + "/" + "[0-9]" * 6 + "*"):
						if msg.startswith(msg_name):
							print "Message", msg, "already present"
							found = True
							break

					if not found:
						utils.write_file(config.logdir + "/" + name, d)
						utils.write_file(config.logdir + "/" + name + ".unsent", d)
						utils.update_index(config.logdir)

	except KeyboardInterrupt:
		print "\nCTRL-C pressed, exit"
	finally:
		os.system("killall aprs_decoder")

def auth_test(start):
	min = start
	for i in range(60):
		updater.send_server(">%04d00zTest %d" % (min, i))
		min += 1

if __name__ == "__main__":
	if len(sys.argv) > 1 and sys.argv[1].isdigit():
		auth_test(int(sys.argv[1]))
	else:
		parse_loop()

