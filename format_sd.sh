#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Usage: $0 <sd dir>"
	exit 0
fi
SD_DIR=$1
sudo umount $SD_DIR
sudo mkdosfs -F 16 -v -n stsp $SD_DIR
sync
