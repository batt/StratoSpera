#Settings

#Sender callsign, leave empty in order to log everything
sender = ""
#Directory where messages will be logged, do not add / at the end!
logdir = "msg"
#CGI url
url = "http://www.develer.com/~batt/stratospera/bsm-2/add.cgi"
#password used to sign messages sent to server
password = "stsp2"
#Timeout while sending message to server, if set to 0 will use system timeouts (looongs!)
net_timeout = 10
#Set to true in order to log even messages without timestamp; dangerous!
log_all_messages = True
