from CGIHTTPServer import *
from BaseHTTPServer import *
import urllib
import os

def ensure_dir(f):
	d = os.path.dirname(f)
	if not os.path.exists(d):
		os.makedirs(d)

class WebServer(CGIHTTPRequestHandler):

	def do_GET(self):
		path = self.translate_path(self.path)
		if not os.path.exists(path):
			u = urllib.urlopen("http://www.develer.com" + self.path)
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
				self.copyfile(u, f)
				f.close()
			u.close()
		CGIHTTPRequestHandler.do_GET(self)

	def guess_type(self, path):
		if os.path.exists(path + ".content-type"):
			return file(path + ".content-type").read()
		else:
			return CGIHTTPRequestHandler.guess_type(self, path)

		

if __name__ == "__main__":
	server = HTTPServer(("", 1234), WebServer)
	server.allow_reuse_address = True
	try:
		server.serve_forever()
	except KeyboardInterrupt:
		print "\nClosing."
