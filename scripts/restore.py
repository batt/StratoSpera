#!/usr/bin/env python

import config

import glob
import os

if __name__ == "__main__":
	for f in glob.glob(config.logdir + "/" + "[0-9]" * 6):
		os.system("mv %s %s" % (f, f + ".unsent"))
