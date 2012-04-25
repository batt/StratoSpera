#!/bin/bash

set -e
set -u

MSG_DIR="msg"

cd $MSG_DIR
rm -f last_message
rm -f *.tmp
ls | grep -E "^[0-9]{6}$" | xargs rm -f
cd ..
./upd_idx.sh
rm -f $MSG_DIR/last_message
