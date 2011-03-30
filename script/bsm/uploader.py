#!/usr/bin/env python

import config

import os
import glob
import utils
import shutil

if __name__ == "__main__":
	files = glob.glob(config.logdir + "/" + "[0-9]" * 6 + ".unsent")
	for msg_file in reversed(sorted(files)):
		name = os.path.basename(msg_file).rstrip(".unsent")
		unsent = open(msg_file, "r+")
		msg = unsent.read().strip()
		reply = utils.send_server(msg, name)
		if reply.startswith("OK"):
			unsent.close()
			os.remove(msg_file)
			print "OK"
		elif reply.startswith("ERR"):
			unsent.write(reply)
			unsent.close()

			shutil.move(msg_file, config.logdir + "/" + name + ".err")
			print "Message not accepted by server, see", name + ".err"
		else:
			unsent.close()
			print "Unknown error, will retry next time"
