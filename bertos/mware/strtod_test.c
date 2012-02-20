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
 * Copyright 2010 Develer S.r.l. (http://www.develer.com/)
 *
 * -->
 *
 * \brief strtod(), strtof(), atof() test.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include <cfg/test.h>
#include <cfg/debug.h>
#include <cpu/detect.h>

#include <stdlib.h>

int strtod_testSetup(void);
int strtod_testRun(void);
int strtod_testTearDown(void);

int strtod_testSetup(void)
{
	kdbg_init();
	return 0;
}

int strtod_testRun(void)
{
	ASSERT(atof("0") == 0.0);
	ASSERT(atof("+0") == 0.0);
	ASSERT(atof("-0") == 0.0);
	ASSERT(atof("2") == 2.0);
	ASSERT(atof("+2") == 2.0);
	ASSERT(atof("-2") == -2.0);
	ASSERT(atof("0.5") - 0.5 < 1e-6);
	ASSERT(atof("     +0.5") - 0.5 < 1e-6);
	ASSERT(atof("     -0.5") - -0.5 < 1e-6);
	ASSERT(atof("5e-1") - 0.5 < 1e-6);
	ASSERT(atof("5e-00001") - 0.5 < 1e-6);
	ASSERT(atof("0.05e+00001") - 0.5 < 1e-6);
	ASSERT(atof("0.0521342134124e+1") == atof("0.0521342134124e1"));
	ASSERT(atof("1.25E-1") - 0.125 < 1e-6);
	ASSERT(atof("0.123456789e-13") - 0.123456789E-13 < 1e-20);
	ASSERT(atof("123456.789E-13") - 123456.789E-13 < 1e-14);
	ASSERT(atof("123456.789e-13aaaaaa") - 123456.789E-13 < 1e-14);
	ASSERT(atof("123456.789 E-13") - 123456.789 < 1e-3);
	ASSERT(atof("123.hello789e-13") - 123.0 < 1e-6);
	ASSERT(atof("   002r23.hello789E-13") == 2.0);
	ASSERT(atof("hello") == 0);
	ASSERT(atof("123.456.999") - 123.456 < 1e-6);
	#if !CPU_AVR
		ASSERT(atof("2.2250738585072012e-308") - 2.2250738585072012e-308 < 1e-314);
	#endif
	ASSERT(atof("-0.000e-16") == 0.0);
	char test_str[] = "1.23456789e13";
	char *endptr;
	ASSERT(strtod(test_str, &endptr) - 1.23456789e13 < 1e-6);
	ASSERT(*endptr == '\0');
	ASSERT(endptr == &test_str[13]);
	char test_str1[] = "123.456.999";
	ASSERT(strtod(test_str1, &endptr) - 123.456 < 1e-6);
	ASSERT(*endptr == '.');
	ASSERT(endptr == &test_str1[7]);
	char test_str2[] = "error";
	ASSERT(strtod(test_str2, &endptr) == 0.0);
	ASSERT(*endptr == 'e');
	ASSERT(endptr == test_str2);
	char test_str3[] = ".error";
	ASSERT(strtod(test_str3, &endptr) == 0.0);
	ASSERT(*endptr == '.');
	ASSERT(endptr == test_str3);
	char test_str4[] = ".000er";
	ASSERT(strtod(test_str4, &endptr) == 0.0);
	ASSERT(*endptr == 'e');
	ASSERT(endptr == &test_str4[4]);
	char test_str5[] = "-.123.456.999";
	ASSERT(strtod(test_str5, &endptr) - -0.123 < 1e-9);
	ASSERT(*endptr == '.');
	ASSERT(endptr == &test_str5[5]);

	return 0;
}

int strtod_testTearDown(void)
{
	return 0;
}

TEST_MAIN(strtod);
