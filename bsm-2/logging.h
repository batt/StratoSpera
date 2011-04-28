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

#ifndef LOGGING_H
#define LOGGING_H

#include "gps.h"

#include <kern/sem.h>
#include <io/kfile.h>
#include <fs/fat.h>

#include <time.h>
#include <stdio.h>

#define logging_data(fmt, ...) \
do \
{ \
	extern Semaphore log_sem; \
	extern FatFile logfile; \
	sem_obtain(&log_sem); \
	kfile_printf(&logfile.fd, fmt, ##__VA_ARGS__); \
	kfile_flush(&logfile.fd); \
	sem_release(&log_sem); \
} while (0)


#define logging_msg(fmt, ...) \
do \
{ \
	extern Semaphore log_sem; \
	extern FatFile msgfile; \
	struct tm *t; \
	time_t tim; \
	tim = gps_time(); \
	t = gmtime(&tim); \
	char buf[16]; \
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d:",t->tm_hour, t->tm_min, t->tm_sec); \
	kprintf("%s", buf); \
	kprintf(fmt, ##__VA_ARGS__); \
	sem_obtain(&log_sem); \
	kfile_printf(&msgfile.fd, "%s", buf); \
	kfile_printf(&msgfile.fd, fmt, ##__VA_ARGS__); \
	kfile_flush(&msgfile.fd); \
	sem_release(&log_sem); \
} while (0)

void logging_rotate(void);
void logging_init(void);

#endif
