#!/usr/bin/env bash
#set -x
ps -ea | grep $1 | gawk '$1 ~ /^[^SI]/ { system("kill -9 " $1); }'
