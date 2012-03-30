#!/bin/bash
#set -x
set -e

if [ ! $# = 1 ] ; then
	echo "Usage:" $0 "<version_name>"
	exit 1
fi

git diff --exit-code > /dev/null
if [ $? -ne 0 ]; then
	echo "ERROR: uncommitted changes!"
	exit 2
fi
version=$1
echo "Current version will be \""$version"\", do you want to proceed?"
read resp
if [ $resp != y ]; then
	exit 0
fi
git tag "$version"
make
zip -j "$version" conf.ini images/bsm-2.bin
host="batt@shell.develer.com"
scp "$version".zip $host:public_html/bsm-2/fw/
ssh $host rm -rf public_html/bsm-2/fw/0current.zip
ssh $host ln -s "$version".zip public_html/bsm-2/fw/0current.zip
