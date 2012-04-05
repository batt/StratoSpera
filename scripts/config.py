#Settings

#Sender callsign, leave empty in order to log everything
sender = ""
#Directory where messages will be logged, do not add / at the end!
logdir = "webserver/msg"
#Base url for remote logging
#base_url = "http://www.develer.com/~batt/stratospera/bsm-2/"
base_url = "http://83.149.158.210/~batt/stratospera/bsm-2/"
add_cgi = "add.cgi"
msg_index = "msg_index"
#password used to sign messages sent to server
password = "stsp4"
#Timeout while sending message to server, if set to 0 will use system timeouts (looongs!)
net_timeout = 20
#Set to true in order to log even messages without timestamp; dangerous!
log_all_messages = True
#starting base coordinated used to compute distance in the viewer
start_lat = 43.606414
start_lon = 11.311832

add_url = base_url + add_cgi
msg_index_url = base_url + msg_index
