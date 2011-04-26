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
 * \brief Moving Average algorith macros.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#ifndef ALGO_MOVING_AVG_H
#define ALGO_MOVING_AVG_H

#include <cfg/compiler.h>
#include <cfg/macros.h>

#define MOVING_AVG_TYPE(name) struct name##MovingAverage

#define MOVING_AVG_DEFINE(type, name, size) \
	MOVING_AVG_TYPE(name) \
	{ \
		type values[size]; \
		type sum; \
		unsigned cnt; \
		unsigned idx; \
	} name

#define MOVING_AVG_RESET(avg) \
	do { \
		(avg)->sum = 0; \
		(avg)->cnt = 0; \
		(avg)->idx = 0; \
	} while(0)

#define MOVING_AVG_COUNT(avg) ((avg)->cnt)
#define MOVING_AVG_EMPTY(avg) (MOVING_AVG_COUNT(avg) == 0)
#define MOVING_AVG_FULL(avg)  (MOVING_AVG_COUNT(avg) >= countof((avg)->values))


#define MOVING_AVG_PUSH(avg, val) \
	do { \
		if ((avg)->cnt >= countof((avg)->values)) \
			(avg)->sum -= (avg)->values[(avg)->idx]; \
		(avg)->values[(avg)->idx] = (val); \
		(avg)->sum += (val); \
		(avg)->idx = (avg)->idx + 1 >= countof((avg)->values) ? 0 : (avg)->idx + 1; \
		if ((avg)->cnt < countof((avg)->values)) \
			(avg)->cnt++; \
	} while (0)

#define MOVING_AVG_GET(avg, ...) \
	({ \
		ASSERT(!MOVING_AVG_EMPTY(avg)); \
		PP_CAT(MOVING_AVG_GET_, COUNT_PARMS(__VA_ARGS__))(avg, ## __VA_ARGS__); \
	})

#define MOVING_AVG_GET_0(avg) ((avg)->sum / (int)(avg)->cnt)
#define MOVING_AVG_GET_1(avg, type) (((type)(avg)->sum) / (int)(avg)->cnt)

#endif /* ALGO_MOVING_AVG_H */
