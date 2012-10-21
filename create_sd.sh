#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Usage: $0 <skel dir> <sd root dir>"
	exit 0
fi
SD_DIR=$2
SKEL=$1
make && cp images/bsm-2.bin "$SKEL" && cp -r "$SKEL"/* "$SD_DIR" && sync
