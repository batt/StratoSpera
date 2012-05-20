from CGIHTTPServer import *
from BaseHTTPServer import *
import urllib
import os
import random
from SocketServer import ThreadingMixIn

class ThreadingCGIServer(ThreadingMixIn, HTTPServer):
	pass

class WebServer(CGIHTTPRequestHandler):

	def do_GET(self):
		path = self.translate_path(self.path)
		if self.path.startswith("/maps") and not os.path.exists(path):
			url = "http://" + random.choice("abc") + ".tile.opencyclemap.org/cycle" + self.path.replace("/maps", "")
			os.system("python download.py %s %s" % (url, path))
		CGIHTTPRequestHandler.do_GET(self)

	def guess_type(self, path):
		if os.path.exists(path + ".content-type"):
			return file(path + ".content-type").read()
		else:
			return CGIHTTPRequestHandler.guess_type(self, path)



if __name__ == "__main__":
	server = ThreadingCGIServer(("", 1234), WebServer)
	server.allow_reuse_address = True
	try:
		server.serve_forever()
	except KeyboardInterrupt:
		print "\nClosing."
