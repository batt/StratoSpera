#! /usr/bin/env bash
#set -x
BASE_DIR=`dirname $0`
. ${BASE_DIR}/common.sh

OUT_FILE="openocd.tmp"
rm -rf ${OUT_FILE}
sed -e "s#PROGRAMMER_TYPE#${INT_FILE}#" ${BASE_DIR}/openocd/debug.cfg | sed -e "s#PROGRAMMER_CPU#${CPU_FILE}#" > ${OUT_FILE}

openocd -d 1 -f ${OUT_FILE} -l debug.log
exit $?
