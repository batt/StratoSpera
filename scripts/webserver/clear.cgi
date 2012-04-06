#!/bin/bash

set -e
set -u

echo "Content-type: text/html"
echo ""

rm -f last_message
rm -f *.tmp
ls | grep -E "^[0-9]{6}$" | xargs rm -f
./upd_idx.sh
rm -f last_message
echo "OK"
