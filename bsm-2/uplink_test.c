/**
 * \file
 * <!--
 * This file is part of BeRTOS.
 *
 * Bertos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 *
 * Copyright 2012 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \brief Uplink command parser
 *
 * \author Francesco Sacchi <batt@develer.com>
 *
 *
*
 * $test$: cp bertos/cfg/cfg_kfile.h $cfgdir/
 * $test$: echo "#undef CONFIG_KFILE_GETS" >> $cfgdir/cfg_kfile.h
 * $test$: echo "#define CONFIG_KFILE_GETS 1" >> $cfgdir/cfg_kfile.h
 */

#include "uplink.h"

#include <mware/config.h>

#include <cfg/test.h>
#include <cfg/debug.h>

#include <emul/kfile_posix.h>

#include <string.h>

static int cnt;
static int test_param;

static void test_cmd(long n)
{
	cnt++;
	kprintf("Test command, cnt = %d, val = %ld\n", cnt, n);
	test_param = n;
}

static void reset_cmd(long n)
{
	(void)n;
	cnt = 0;
	test_param = 0;
}


static const char ini_file[] = "./bsm-2/config_uplink_test.ini";
static KFilePosix kf;

static void uplink_reload(void);

DECLARE_CONF(uplink, uplink_reload,
	CONF_INT(int1, 0, 10, 5),
	CONF_FLOAT(float1, 0, 3.14, 2.7)
);

static bool reloaded = false;
static void uplink_reload(void)
{
	kprintf("Reloading:\n");
	kprintf("int1 %d\n", int1);
	kprintf("float1 %f\n", float1);
	reloaded = true;
}

int uplink_testSetup(void)
{
	kdbg_init();

	if (!kfile_posix_init(&kf, ini_file, "r"))
	{
		kprintf("No test file found\n");
		return -1;
	}
	config_init(&kf.fd);
	config_register(&uplink);
	config_load(&uplink);
	ASSERT(int1 == 3);
	ASSERT(ABS(float1 - 0.2) < 1e-6);

	uplink_init();
	uplink_registerCmd("test", test_cmd);
	uplink_registerCmd("reset", reset_cmd);
	return 0;
}

static void test_configs(void)
{
	reloaded = false;
	char msg1[] = "cfg uplink int1=2";
	ASSERT(uplink_parse(msg1, strlen(msg1)));
	ASSERT(reloaded == true);
	ASSERT(int1 == 2);

	reloaded = false;
	char msg2[] = "cfg uplink int1=9 float1:3";
	ASSERT(uplink_parse(msg2, strlen(msg2)));
	ASSERT(reloaded == true);
	ASSERT(int1 == 9);
	ASSERT(ABS(float1 - 3) < 1e-6);

	reloaded = false;
	char msg3[] = "cfg uplink int1=1232";
	ASSERT(uplink_parse(msg3, strlen(msg3)));
	ASSERT(reloaded == true);
	ASSERT(int1 == 10);

	reloaded = false;
	char msg4[] = "cfg uplink int1=hello";
	ASSERT(!uplink_parse(msg4, strlen(msg4)));
	ASSERT(reloaded == true);
	ASSERT(int1 == 10);

	reloaded = false;
	char msg5[] = "cfg unknown_module int1=0";
	ASSERT(!uplink_parse(msg5, strlen(msg5)));
	ASSERT(reloaded == false);

	reloaded = false;
	char msg6[] = "cfg uplink hello1";
	ASSERT(!uplink_parse(msg6, strlen(msg6)));
	ASSERT(reloaded == false);

	reloaded = false;
	char msg7[] = "cfg uplink hello=1 int1=4";
	ASSERT(!uplink_parse(msg7, strlen(msg7)));
	ASSERT(reloaded == true);
}

static void test_commands(void)
{
	char msg1[] = "test 123456";
	ASSERT(uplink_parse(msg1, strlen(msg1)));
	ASSERT(test_param == 123456);
	ASSERT(cnt == 1);
	ASSERT(uplink_parse(msg1, strlen(msg1)));
	ASSERT(test_param == 123456);
	ASSERT(cnt == 2);

	char msg2[] = "error";
	ASSERT(!uplink_parse(msg2, strlen(msg2)));
	ASSERT(test_param == 123456);
	ASSERT(cnt == 2);

	char msg3[] = "";
	ASSERT(!uplink_parse(msg3, strlen(msg3)));

	char msg4[] = "test=1";
	ASSERT(uplink_parse(msg4, strlen(msg4)));
	ASSERT(test_param == 1);
	ASSERT(cnt == 3);

	char msg5[] = "test(31)";
	ASSERT(uplink_parse(msg5, strlen(msg5)));
	ASSERT(test_param == 31);
	ASSERT(cnt == 4);

	char msg6[] = "test:17";
	ASSERT(uplink_parse(msg6, strlen(msg6)));
	ASSERT(test_param == 17);
	ASSERT(cnt == 5);

	char msg7[] = "test : 19";
	ASSERT(uplink_parse(msg7, strlen(msg7)));
	ASSERT(test_param == 19);
	ASSERT(cnt == 6);

	char msg8[] = "test : test";
	ASSERT(uplink_parse(msg8, strlen(msg8)));
	ASSERT(test_param == 0);
	ASSERT(cnt == 7);

	char msg9[] = "test :=( 0xdead)";
	ASSERT(uplink_parse(msg9, strlen(msg9)));
	ASSERT(test_param == 0xdead);
	ASSERT(cnt == 8);

	char msg10[] = "reset all";
	ASSERT(uplink_parse(msg10, strlen(msg10)));
	ASSERT(test_param == 0);
	ASSERT(cnt == 0);
}

int uplink_testRun(void)
{
	test_commands();
	test_configs();

	return 0;
}

int uplink_testTearDown(void)
{
	return 0;
}

TEST_MAIN(uplink);
