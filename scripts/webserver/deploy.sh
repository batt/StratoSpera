#!/bin/bash
set -e
set -u

app_url="batt@shell.develer.com:~/public_html/stratospera/bsm-2"

#Create the web config file.
echo "var local_app=false;" | cat - config.js > config_web.js
rsync -vu config_web.js $app_url/config.js
rm config_web.js

#Sync other files/directories
files="add.cgi balloon.js clear.cgi index.html lib upd_idx.sh"
rsync -vru $files $app_url

