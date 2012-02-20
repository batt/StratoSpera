#!/usr/bin/env bash

BAUDRATE=
if [ ${PROGRAMMER_TYPE} == "arduino57600" ] ; then
	BAUDRATE="-b 57600"
	PROGRAMMER_TYPE=arduino
elif [ ${PROGRAMMER_TYPE} == "arduino19200" ] ; then
	BAUDRATE="-b 19200"
	PROGRAMMER_TYPE=arduino
fi

avrdude -p ${PROGRAMMER_CPU} -c ${PROGRAMMER_TYPE} -P ${PROGRAMMER_PORT} $BAUDRATE -U flash:w:${IMAGE_FILE} >flash.log 2>&1
