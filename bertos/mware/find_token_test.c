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
 * \brief Pimped implementation of strtok().
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "find_token.h"

#include <cfg/debug.h>
#include <cfg/test.h>

#include <string.h>

int find_token_testSetup(void)
{
	kdbg_init();
	return 0;
}

int find_token_testRun(void)
{
	#define LEN 8
	char val[LEN];

	const char msg0[] = "test";
	find_token(msg0, strlen(msg0), val, LEN, " ");
	ASSERT(strcmp("test", val) == 0);

	const char msg1[] = ";  test1  ,,  test2 ; test3  ";
	const char *buf = msg1;
	const char *end = msg1 + strlen(msg1);
	buf = find_token(buf, end - buf, val, LEN, ";, ");
	ASSERT(strcmp("test1", val) == 0);
	buf = find_token(buf, end - buf, val, 4+1, ";, ");
	ASSERT(strcmp("test", val) == 0);
	buf = find_token(buf, end - buf, val, LEN, ";, ");
	ASSERT(strcmp("test3", val) == 0);
	ASSERT(buf == end);
	buf = find_token(buf, end - buf, val, LEN, ";, ");
	ASSERT(*val == 0);
	ASSERT(buf == end);

	const char msg2[] = "test1234";
	find_token(msg2, strlen(msg2) - 4, val, LEN, " ");
	ASSERT(strcmp("test", val) == 0);

	const char msg3[] = "test1234";
	find_token(msg3, strlen(msg3), val, 4+1, " ");
	ASSERT(strcmp("test", val) == 0);

	const char msg4[] = "";
	find_token(msg4, 0, val, LEN, " ");
	ASSERT(*val == 0);

	return 0;
}


int find_token_testTearDown(void)
{
	return 0;
}

TEST_MAIN(find_token);
