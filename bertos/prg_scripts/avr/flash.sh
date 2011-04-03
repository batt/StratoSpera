#!/usr/bin/env bash

avrdude -p ${PROGRAMMER_CPU} -c ${PROGRAMMER_TYPE} -P ${PROGRAMMER_PORT} -U flash:w:${IMAGE_FILE} >flash.log 2>&1
