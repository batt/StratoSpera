#!/usr/bin/env python
import sys
import os
import urllib
import cgi
import cgitb
import hmac
cgitb.enable()

password = "stsp4"
msg_dir='msg'

def check_msg(msg):
    if msg[0] != '/' and msg[0] != '>':
        return False
    return msg[1:7].isdigit()

def check_hash(msg, sign):
    m = hmac.new(password, msg)
    return (m.hexdigest() == sign)

print "Content-type: text/html\n"
form = cgi.FieldStorage()
if 'm' in form and 's' in form and 'n' in form:
    msg = form['m'].value
    sign = form['s'].value
    msg_name = msg_dir + os.sep + form['n'].value
    if not check_hash(msg, sign):
        print "ERR<br>\nInvalid sign."
    else:
        print "OK<br>"
        print msg
        if not os.path.exists(msg_name):
            tmp_name = str(os.getpid()) + ".tmp"
            f = open(tmp_name, 'w')
            f.write(msg)
            f.close()
            os.system("mv %s %s" % (tmp_name, msg_name))
            #trigger msg_index update
            os.system("./upd_idx.sh")
else:
    print "ERR<br>\nInvalid message format.<br>\nFormat is ?m=msg&s=sign&n=filename"
