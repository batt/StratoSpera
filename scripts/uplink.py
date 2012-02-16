#!/usr/bin/env python

import config_uplink
import afsk
import ax25

import hmac
import sys
import pyaudio
import threading
import cmd
import Queue

data = Queue.Queue()

def sendMsg(msg):
	msg = "%x>" % config_uplink.seq + msg.strip()
	h = hmac.new(config_uplink.password)
	h.update(msg)
	d = h.digest()
	msg = "{{>%x>%s" %  (ord(d[7]) | (ord(d[13]) << 8), msg)
	sys.stdout.write(msg + '\n')
	data.put(msg)


class AudioThread(threading.Thread):
	def __init__(self, queue, terminate):
		threading.Thread.__init__(self)
		self.p = pyaudio.PyAudio()
		self.stream = self.p.open(format = pyaudio.paUInt8, channels = 1, rate = 9600, output = True, frames_per_buffer = 0)
		self.afsk = afsk.Afsk(self.stream)
		self.modem = ax25.Ax25(self.afsk)
		self.queue = queue
		self.terminate = terminate

	def run(self):
		while not self.terminate.is_set():
			if not self.queue.empty():
				self.modem.send(["apzbrt", config_uplink.sender], self.queue.get())
			else:
				self.stream.write(chr(128) * 128)

class CmdInterpreter(cmd.Cmd):
	def __init__(self):
		cmd.Cmd.__init__(self)
		self.prompt = ">> "

	def do_send(self, s):
		sendMsg(s)
		config_uplink.seq += 1

	def do_seq(self, s):
		if not s:
			print "%d - 0x%x" % (config_uplink.seq, config_uplink.seq)
		else:
			try:
				config_uplink.seq = int(s, 0)
			except ValueError:
				print "*** argument should be a number"

	def do_password(self, s):
		if not s:
			print config_uplink.password
		else:
			config_uplink.password = s

	def do_sender(self, s):
		if not s:
			print config_uplink.sender
		elif not s.isalnum() or len(s) > 6:
			print "*** argument should be a valid callsign"
		else:
			config_uplink.sender = s

	def do_exit(self, s):
		return True

	do_EOF = do_exit

	do_q = do_exit

terminate = threading.Event()
a = AudioThread(data, terminate)
a.start()
c = CmdInterpreter()
c.cmdloop()
terminate.set()
print "Exit"
