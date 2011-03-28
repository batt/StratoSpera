#!/usr/bin/env python

import config

import os
import glob
import time
import utils
import hmac

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

	if diff_web:
		utils.update_index(config.logdir)
