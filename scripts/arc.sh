#!/bin/bash

tar jcvf repeater.tar.bz2 --exclude 'webserver/maps/*' --exclude 'webserver/msg/*' *.py viewer webserver/
