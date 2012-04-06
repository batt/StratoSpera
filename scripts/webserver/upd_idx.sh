#!/bin/bash
set -e
set -u

UPD_URL="http://tracker.stratospera.com/refresh.php"
MSG_DIR="."

TEMP=$$.tmp
ls $MSG_DIR | grep -E "^[0-9]{6}$" | sort > $TEMP
#mv are atomic
mv $TEMP $MSG_DIR/msg_index
LAST=`ls $MSG_DIR | grep -E "^[0-9]{6}$" | sort | tail -n1`
if [ ! z$LAST = z ] ; then
	ln -s $MSG_DIR/$LAST $TEMP
	#mv are atomic
	mv $TEMP $MSG_DIR/last_message
fi

wget $UPD_URL -O /dev/null
