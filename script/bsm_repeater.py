#!/usr/bin/env python
import os
import time
import pickle
import subprocess


#Settings
logfile = 'bsm.log'
sender = 'IZ1DYB-9'
server =  'batt@develer.com'
unsentfile = 'unsent.pic'
keyfile = '/tmp/bsm-key'
key = """-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEAuiKUeg75DdGBe2CmNj+IQ1lTgdiFtdY+xOnqKd82kmumhzRn
vssHwAv/ZlFWVxg774FDFrTtWlr2u3hr/dS7bI0gtNpuaUNPf2so2ssJfkDXedMC
NEZMRIE6xKjU23jL+vT8PB1zcFTwXluddN4j6fUw1O3lMgYFNzRTEcCcuEa/b8BW
CTbJmvMcTSYwsv04sfUt4HTAwr+sBbj4cGPR3Rax8jCQhxsk3LNroFCWRCOUtZbR
i/XIxNtPjlJUApXyGIwyYczt7wY/Jwj+Qxf0EmLT8R7BjPei49neFYtr0+yYSAky
Y2xUbQSCtc/gL8xn21evFFTXDCLbBebaNS4fmwIDAQABAoIBAFuoS5UPanTzhd9L
aI2uG1uO0SBPjwhLx/0Pcs/LqwPPHuHVXIEHWXmobsvjobChrmHyNScpUPf9cyI9
2m/UIbcCh+iG53fOPYdxLV0QPkx9gZ3r2loDg+r3+Ah/WtH4WfnqSYsQtZfLO9I7
Kv4xpV2/75vqycIoVMkqav2C66mR1hQumCRYqgqEtUwdug1sqKPmXNNd9xZAUYWS
d5WzITn8TLmsTtamg3biCpr+CobgGqQcZLlk6CRF4gFHZhOFh/qZY6DpEl1punhC
wXjmc1wO5i8xpLp7vTdJy+mZrJlFAbY6RUytlv3bSpbP+hBozkqUP+D0pC7SthYw
arOA24ECgYEA3Gbfbfc2+gdZCrCSMNb086FA3HH2oPOPtKk5E9XbdfmDr5b3YIrl
yMSWRamAzIFO63nR/4i99oUaXidnomfBGLS6dv5Q5MZu4DKkMZmzGAHJ6AkFInJ5
nRfDIsr42U2JsSgxedHkiRWrt5BXeRpMDVygKgGrmTI03rckMZlJzb8CgYEA2DLa
NXgJmLisOFDou95Dfm3fVq9kBcHNSW6+nVYJgn7yBRnqDEv/2H9iTvuxaQIUfYoF
/N2zLS2/HcSCj2BbCmFlm/UW2obsufD9J5JbwD/LSpresu8U3u9xtW0NAIrQ2ztK
HMf06tqQsuNCtUOtjDii4Ck3mzzfmsL39V17XSUCgYEAxbwxpinOFAF1nEaP0et4
df9s/pnsB7icbBwKTkZmj3Bc+bK6m83hm/7rRvJs0I0ObeqsQdK/gyUlY+V8b9Mg
BXdXdxxxHN7+aHxmnO1lJRuttlQpXB9SBmNkOZnaKrMK3nrN8Jojq+1aUuTX5Zl3
M+Gu/CDlgBrwHgQ3H5yD/2cCgYEAxbz1EXRhqpo1giiWRMYMC0WowNsX66APB7vP
gY3gksdSylGiXG7iaPxRSRYxdG6fmRa1VUrch1TQ+QPzufkSK5NbYOIwbdx8BbEh
iaD2ZAa1A95UE2pSN3jfEmXUP4u3bNx0c5B7NJaFQ+hR6gIlaBug9M8d+dEaXq02
JufCXjUCgYEAxnSz0aZvOPxkaJRUTqD2wudW8CCzm3uQHD7ycB2GgcbYLMEOHVUC
F6BhGPEAaEGfWjWJ9nM8+6yIXCfQNo+Cz6PYJA+Oes8gKRXIqbCuqfwE6k00ynr1
wD9s+BD31bWv1RclkNtmpqPXTL2DsaI2cgMC/mWZpFd1H+xbKkEucZE=
-----END RSA PRIVATE KEY-----
"""
#end of settings

def send_server(msg):
	return (subprocess.call(["ssh",
			"-o ConnectTimeout=3",
			"-o CheckHostIP=no",
			"-o Compression=yes",
			"-i", keyfile, server, msg]) == 0)

def update_unsent(msg, file):
	file.seek(0)
	pickle.dump(msg, file)
	file.flush()

def parse_loop():
	#create key file
	k = open(keyfile, 'w')
	k.write(key)
	k.close()
	os.system("chmod 600 " + keyfile)

	print "Testing server connection..."
	if send_server("test"):
		print "OK"
	else:
		print "Error"

	#start modem
	os.system("script -afc \"padsp multimon -a afsk1200\" %s&" % logfile)
	#open modem logging file
	file = open(logfile,'r')

	#Open unsent messages file
	try:
		picfile = open(unsentfile, 'r+')
		messages = pickle.load(picfile)
	except IOError:
		picfile = open(unsentfile, 'w+')
		messages = []

	#Move to the logfile end
	file.seek(0, os.SEEK_END);

	wait_start = True
	try:
		while 1:
			d = file.readline()
			if not d:
				time.sleep(0.1)
				#if there are messages in queue send them starting from
				#the newers.
				if len(messages) > 0:
					print "Sending msg", messages[-1]
					if (send_server(messages[-1])):
						print "Message sent"
						messages.pop()
						update_unsent(messages, picfile)
					else:
						print "Error"
			else:
				if wait_start:
					#Check for correct sender address
					if d.startswith("AFSK1200: fm %s" % sender):
						wait_start = False
				else:
					wait_start = True
					d = d.strip()
					if d not in messages:
						messages.append(d)
						update_unsent(messages, picfile)

	except KeyboardInterrupt:
		print "\nCTRL-C pressed, exit"
	finally:
		os.system("killall multimon")
		picfile.close()

if __name__ == "__main__":
	parse_loop()

