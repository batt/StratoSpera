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
 * All Rights Reserved.
 * -->
 *
 * \brief Logging.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "logging.h"

#include <kern/sem.h>
#include <io/kfile.h>
#include <fs/fat.h>

#include <string.h>
#include <stdio.h>

FatFile logfile, msgfile;
Semaphore log_sem;

static bool logopen, msgopen;

static void rotate_file(FatFile *f, const char *prefix)
{
	char name[13];
	ASSERT(strlen(prefix) == 3);
	for (int i = 0; i < 100000; i++)
	{
		snprintf(name, sizeof(name), "%s%05d.csv", prefix, i);
		if (fatfile_open(f, name, FA_OPEN_EXISTING | FA_WRITE) != FR_OK)
		{
			kprintf("Logging on file %s\n", name);
			ASSERT(fatfile_open(f, name, FA_OPEN_ALWAYS | FA_WRITE) == FR_OK);
			break;
		}
		kfile_close(&f->fd);
	}

	//Seek to the end
	kfile_seek(&f->fd, 0, KSM_SEEK_END);
	kfile_printf(&f->fd, "Logging start...\n");
}

void logging_rotate(void)
{
	sem_obtain(&log_sem);

	if (logopen)
	{
		kfile_close(&logfile.fd);
		logopen = false;
	}
	rotate_file(&logfile, "log");
	logopen = true;

	kfile_printf(&logfile.fd, "GPS time;GPS FIX;GPS lat;GPS lon;GPS alt (m);"
		"Ext T1 (°C); Ext T2 (°C);Pressure (mBar);Humidity (%%);Internal Temp (°C);"
		"Vsupply (V);+5V;+3.3V;Current (mA);"
		"Acc X (m/s^2);Acc Y (m/s^2);Acc Z (m/s^2);Geiger counter (cpm)\n");

	if (msgopen)
	{
		kfile_close(&msgfile.fd);
		msgopen = false;
	}
	rotate_file(&msgfile, "msg");
	msgopen = true;

	sem_release(&log_sem);
}

void logging_init(void)
{
	sem_init(&log_sem);
	logging_rotate();
}
