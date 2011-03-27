#!/usr/bin/env python

import config

import os
import glob
import time
import utils
import hmac

def send_server(msg, name):
	print "Sending:", msg
	m = hmac.new(config.password, msg)
	sign = m.hexdigest()
	d = {}
	d['m'] = msg
	d['s'] = sign
	d['n'] = name
	return utils.http_get(config.add_url, d)

def web_index():
	return utils.http_get(config.msg_index_url).split()

def parse_index(index):
	s = set()
	for f in index:
		s.add(f.strip())

	return s

if __name__ == "__main__":
	try:
		l = open(config.logdir + "/" + config.msg_index)
		local = parse_index(l)
	except IOError:
		local = set()

	web = parse_index(web_index())
	diff_web = web - local
	for d in diff_web:
		print "Getting", d
		msg = utils.http_get(config.base_url + d)
		utils.write_file(config.logdir + "/" + d, msg)

	diff_local = local - web
	for d in diff_local:
		unsent = open(config.logdir + "/" + d, "r+")
		msg = unsent.read().strip()
		reply = send_server(msg, d)
		if reply.startswith("OK"):
			print "OK"
		elif reply.startswith("ERR"):
			#todo test this branch
			unsent.write(reply)
			unsent.close()
			shutil.move(config.logdir + "/" + d, config.logdir + "/" + d + ".err")
			print "Message not accepted by server, see", d + ".err"
		else:
			print "Unknown error, will retry next time"

	if diff_web:
		utils.update_index(config.logdir)
