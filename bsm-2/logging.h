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

#include "cfg/cfg_arch.h"
#include <cfg/compiler.h>
#include <stdarg.h>

int logging_data(const char *fmt, ...);
int logging_vmsg(const char *fmt, va_list ap);
int logging_msg(const char *fmt, ...);
void logging_rotate(void);
void logging_init(void);

INLINE bool logging_running(void)
{
	extern bool logging_initialized;
	return logging_initialized;
}

#if !(ARCH & ARCH_UNITTEST)
	#undef LOG_PRINT
	#define LOG_PRINT(str_level, str,...) logging_msg("%s: " str, str_level, ## __VA_ARGS__)
#endif

#endif
