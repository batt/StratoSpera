#!/bin/bash
#
# Copyright 2005 Develer S.r.l. (http://www.develer.com/)
# Copyright 2008 Bernie Innocenti <bernie@codewiz.org>
#
# Version: $Id: run_tests.sh 4404 2010-10-05 09:29:31Z asterix $
# Author:  Bernie Innocenti <bernie@codewiz.org>
#

# Testsuite output level:
#  0 - quiet
#  1 - progress output
#  2 - build warnings
#  3 - execution output
#  4 - build commands
VERBOSE=${VERBOSE:-1}

CC=gcc
#FIXME: -Ibertos/emul should not be needed
CFLAGS="-W -Wall -Wextra -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare -Wmissing-noreturn \
-O0 -g3 -ggdb -Ibertos -Ibertos/emul -I. -Ibsm-2/ -std=gnu99 -fno-builtin -D_DEBUG -DARCH=(ARCH_EMUL|ARCH_UNITTEST) \
-DCPU_FREQ=(12288000UL) -ffunction-sections -fdata-sections -Wl,--gc-sections -lpulse-simple"

CXX=g++
CXXFLAGS="$CFLAGS"

TESTS=${TESTS:-`find . \
	\( -name .svn -prune -o -name .git -prune -o -name .hg  -prune \) \
	-o -name "*_test.c" -print` }

TESTOUT="testout"
SRC_LIST="
	bertos/drv/kdebug.c
	bertos/drv/timer.c
	bertos/mware/event.c
	bertos/mware/formatwr.c
	bertos/mware/hex.c
	bertos/mware/sprintf.c
	bertos/mware/config.c
	bertos/mware/ini_reader.c
	bertos/mware/strtod.c
	bertos/algo/crc_ccitt.c
	bertos/net/ax25.c
	bertos/net/afsk.c
	bertos/cfg/kfile_debug.c
	bertos/io/kfile.c
	bertos/emul/kfile_posix.c
	bsm-2/cutoff.c
	bsm-2/status_mgr.c
	bsm-2/landing_buz.c
"

buildout='/dev/null'
runout='/dev/null'
[ "$VERBOSE" -ge 2 ] && buildout='/dev/stdout'
[ "$VERBOSE" -ge 3 ] && runout='/dev/stdout'

# Needed to get build/exec result code rather than tee's
set -o pipefail

rm -rf "${TESTOUT}.old"
if [ -d "${TESTOUT}" ] ; then
	mv -f "${TESTOUT}" "$TESTOUT.old"
fi
mkdir -p "$TESTOUT"

for src in $TESTS; do
	name="`basename $src | sed -e 's/\.cpp$//' -e 's/\.c$//'`"
	testdir="./$TESTOUT/$name"
	cfgdir="$testdir/cfg"
	mkdir -p "$cfgdir"
	exe="$testdir/$name"

	PREPARECMD="test/parsetest.py $src"
	BUILDCMD="$CC -I$testdir $CFLAGS $src $SRC_LIST -o $exe"
	export testdir name cfgdir

	[ $VERBOSE -gt 0 ] && echo "Preparing $name..."
	[ $VERBOSE -gt 4 ] && echo " $PREPARECMD"
	if $PREPARECMD 2>&1 | tee -a >>$buildout $testdir/$name.prep; then
		[ $VERBOSE -gt 0 ] && echo "Building $name..."
		[ $VERBOSE -gt 4 ] && echo " $BUILDCMD"
		if $BUILDCMD 2>&1 | tee -a >>$buildout $testdir/$name.build; then
			[ $VERBOSE -gt 0 ] && echo "Running $name..."
			if ! $exe 2>&1 | tee -a >>$runout $testdir/$name.out; then
				echo "FAILED [RUN]: $name"
				cat $testdir/$name.out
				exit 2
			fi
		else
			echo "FAILED [BUILD]: $name"
			cat $testdir/$name.build
			exit 3
		fi
	else
		echo "FAILED [PREPARING]: $name"
		cat $testdir/$name.prep
		exit 4
	fi
done

