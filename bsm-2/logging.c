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
#include "gps.h"

#include <kern/sem.h>
#include <io/kfile.h>
#include <fs/fat.h>
#include <drv/timer.h>

#include <cfg/module.h>

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

static FatFile logfile;
static FatFile msgfile;
static FatFile accfile;
static FatFile powerfile;

static Semaphore log_sem;
static char logging_buf[16];

static bool log_open;
bool logging_initialized = false;


static void rotate_file(FatFile *f, const char *prefix, const char *ext)
{
	char name[13];
	ASSERT(strlen(prefix) <= 3);
	ASSERT(strlen(ext) <= 3);
	for (int i = 0; i < 100000; i++)
	{
		snprintf(name, sizeof(name), "%s%05d.%s", prefix, i, ext);
		if (fatfile_open(f, name, FA_OPEN_EXISTING | FA_WRITE) != FR_OK)
		{
			kprintf("Logging on file %s\n", name);
			if (fatfile_open(f, name, FA_OPEN_ALWAYS | FA_WRITE) != FR_OK)
			{
				kprintf("Error opening file!\n");
				return;
			}
			break;
		}
		kfile_close(&f->fd);
	}

	//Seek to the end
	kfile_seek(&f->fd, 0, KSM_SEEK_END);

	time_t tim = gps_time();
	struct tm *t = gmtime(&tim);

	kfile_printf(&f->fd, "%02d:%02d:%02d-Logging started @%ld\n",t->tm_hour, t->tm_min, t->tm_sec, (long)timer_clock());
}

void logging_rotate(void)
{
	if (!logging_initialized)
		return;

	sem_obtain(&log_sem);

	if (log_open)
	{
		kfile_close(&logfile.fd);
		kfile_close(&msgfile.fd);
		kfile_close(&accfile.fd);
		log_open = false;
	}
	rotate_file(&logfile, "log", "csv");
	rotate_file(&msgfile, "msg", "txt");
	rotate_file(&accfile, "acc", "dat");
	log_open = true;

	kfile_printf(&logfile.fd, "GPS time;GPS FIX;GPS lat;GPS lon;GPS alt (m);"
		"Ext T1 (°C); Ext T2 (°C);Pressure (mBar);Humidity (%%);Internal Temp (°C);"
		"Vsupply (V);+5V;+3.3V;Current (mA);"
		"Acc X (m/s^2);Acc Y (m/s^2);Acc Z (m/s^2);HADARP counter (cpm)\n");

	sem_release(&log_sem);
}

int logging_vmsg(const char *fmt, va_list ap)
{
	if (!logging_initialized)
	{
		/* Fallback to serial logging */
		kvprintf(fmt, ap);
		return 0;
	}

	struct tm *t;
	time_t tim;

	sem_obtain(&log_sem);
	tim = gps_time();
	t = gmtime(&tim);

	snprintf(logging_buf, sizeof(logging_buf), "%02d:%02d:%02d-",t->tm_hour, t->tm_min, t->tm_sec);
	kprintf("%s", logging_buf);

	va_list aq;
	va_copy(aq, ap);
	kvprintf(fmt, ap);
	va_end(aq);

	kfile_printf(&msgfile.fd, "%s", logging_buf);
	va_copy(aq, ap);
	int len = kfile_vprintf(&msgfile.fd, fmt, ap);
	va_end(aq);
	kfile_flush(&msgfile.fd);
	sem_release(&log_sem);

	return len;
}

int logging_msg(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int len = logging_vmsg(fmt, ap);
	va_end(ap);

	return len;
}

int logging_data(const char *fmt, ...)
{
	if (!logging_initialized)
		return 0;

	sem_obtain(&log_sem);
	va_list ap;
	va_start(ap, fmt);
	int len = kfile_vprintf(&logfile.fd, fmt, ap);
	va_end(ap);
	kfile_flush(&logfile.fd);
	sem_release(&log_sem);
	return len;
}

int logging_acc(void *acc, size_t size)
{
	if (!logging_initialized)
		return 0;

	sem_obtain(&log_sem);
	int len = kfile_write(&accfile.fd, acc, size);
	kfile_flush(&accfile.fd);
	sem_release(&log_sem);
	return len;
}

#define ABOUT_TO_TURN_ON_POWER "POWER FAIL"
#define POWER_TURNED_ON_CORRECTLY "POWER OK"
#define POWER_OK_SIZE (sizeof(POWER_TURNED_ON_CORRECTLY)-1)
#define POWER_FAIL_SIZE (sizeof(ABOUT_TO_TURN_ON_POWER)-1)

bool logging_checkPreviousPowerStatus(void)
{
	if (!logging_initialized)
		return false;

	char buf[POWER_OK_SIZE];

	sem_obtain(&log_sem);
	kfile_seek(&powerfile.fd, 0, KSM_SEEK_SET);
	size_t len = kfile_read(&powerfile.fd, buf, POWER_OK_SIZE);
	sem_release(&log_sem);

	return (len == POWER_OK_SIZE
		&& strncmp(buf, POWER_TURNED_ON_CORRECTLY, POWER_OK_SIZE) == 0);
}

void logging_setPendingPowerFlag(bool pending)
{
	if (!logging_initialized)
		return;

	const char *status = pending ? ABOUT_TO_TURN_ON_POWER : POWER_TURNED_ON_CORRECTLY;
	size_t size = pending ? POWER_FAIL_SIZE : POWER_OK_SIZE;

	sem_obtain(&log_sem);
	kfile_seek(&powerfile.fd, 0, KSM_SEEK_SET);
	kfile_write(&powerfile.fd, status, size);
	f_truncate(&powerfile.fat_file);
	kfile_flush(&powerfile.fd);
	sem_release(&log_sem);
}

static void logging_initPower(void)
{
	#define PWR_FILE "pwr_stat.log"

	if (fatfile_open(&powerfile, PWR_FILE, FA_OPEN_EXISTING | FA_READ | FA_WRITE) != FR_OK)
	{
		kprintf("Creating file %s\n", PWR_FILE);
		if (fatfile_open(&powerfile, PWR_FILE, FA_OPEN_ALWAYS | FA_READ | FA_WRITE) != FR_OK)
		{
			kprintf("Error creating file %s!\n", PWR_FILE);
			return;
		}
		logging_setPendingPowerFlag(false);
	}
}

void logging_init(void)
{
	sem_init(&log_sem);
	logging_initialized = true;
	logging_rotate();

	sem_obtain(&log_sem);
	logging_initPower();
	sem_release(&log_sem);
}
