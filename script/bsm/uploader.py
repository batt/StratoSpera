#!/usr/bin/env python

import config

import os
import glob
import utils

if __name__ == "__main__":
	for msg_file in glob.glob(config.logdir + "/" + "[0-9]" * 6 + ".unsent"):
		name = os.path.basename(msg_file).rstrip(".unsent")
		unsent = open(msg_file, "r+")
		msg = unsent.read().strip()
		reply = utils.send_server(msg, name)
		if reply.startswith("OK"):
			unsent.close()
			os.remove(msg_file)
			print "OK"
		elif reply.startswith("ERR"):
			#todo test this branch
			unsent.write(reply)
			unsent.close()
			shutil.move(msg_file, config.logdir + "/" + name + ".err")
			print "Message not accepted by server, see", name + ".err"
		else:
			unsent.close()
			print "Unknown error, will retry next time"
