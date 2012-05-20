import urllib
import signal
import sys
import os

def ensure_dir(f):
	d = os.path.dirname(f)
	if not os.path.exists(d):
		os.makedirs(d)

url = sys.argv[1]
path = sys.argv[2]

signal.alarm(5)

try:
	u = urllib.urlopen(url)
	if u.getcode() != 404:
		_, ext = os.path.splitext(path)
		if not ext:
			content = u.info().getheader("Content-Type")
			ensure_dir(path)
			h = open(path + ".content-type", "wb")
			h.write(content)
			h.close()

		ensure_dir(path)
		f = open(path, "wb")
		f.write(u.read())
		f.close()
	u.close()
except:
	exit(-1)
finally:
	signal.alarm(0)


