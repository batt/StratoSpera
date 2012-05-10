#!/usr/bin/env python
import sys
import os
import urllib
import cgi
import cgitb
import hmac
cgitb.enable()

password = "stsp4"

print "Content-type: text/html\n"
form = cgi.FieldStorage()
if 'passwd' in form and form['passwd'].value == password:
        print "OK<br>"
        os.system("./clear.sh")
else:
    print "ERR<br>\nInvalid command format.<br>\nFormat is ?passwd=password"
