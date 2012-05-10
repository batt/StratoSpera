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
 * Copyright 2006 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \brief HADARP
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "hadarp.h"
#include "gps.h"

#include <drv/ser.h>
#include <kern/proc.h>

#define LOG_LEVEL LOG_LVL_WARN
#include <cfg/log.h>

#include <stdlib.h>
#include <stdio.h>

static Serial ser;
static int hadarp_cnt;

static void NORETURN hadarp_process(void)
{
	char buf[16];

	while (1)
	{
		struct tm *t;
		time_t tim;

		/* Send time to Polifemo */
		tim = gps_time();
		t = gmtime(&tim);
		kfile_printf(&ser.fd, "%02d%02d%02d", t->tm_hour, t->tm_min, t->tm_sec);

		if (kfile_gets(&ser.fd, buf, sizeof(buf)) == EOF)
		{
			hadarp_cnt = -1;
			kfile_clearerr(&ser.fd);
			continue;
		}

		int hadarp_raw = atoi(buf);
		if (hadarp_raw > 10000 || hadarp_raw < 0)
			hadarp_cnt = -1;
		else
		{
			/* Compensate for geiger tube dead time */
			float cps = hadarp_raw / 60.0;
			hadarp_cnt = ABS(cps / (1.0 - (cps * 1.9e-4)) * 60.0 + 0.5);
		}
		LOG_INFO("HADARP cnt:%d\n", hadarp_cnt);
	}
}

int hadarp_read(void)
{
	return hadarp_cnt;
}

void hadarp_init(unsigned port, unsigned long baudrate)
{
	ser_init(&ser, port);
	ser_setbaudrate(&ser, baudrate);
	hadarp_cnt = -1;

	Process *p = proc_new(hadarp_process, NULL, KERN_MINSTACKSIZE * 3, NULL);
	ASSERT(p);
}
