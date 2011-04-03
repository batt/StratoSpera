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
 * \brief Landing control for camera and buzzer.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "landing.h"

#include "gps.h"
#include "hw/hw_pin.h"

#include <net/nmea.h>
#include <cfg/compiler.h>
#include <cfg/module.h>
#include <drv/timer.h>
#include <drv/buzzer.h>

#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>

#include <math.h>

#define LAND_OFF()  do { PIOA_CODR = LAND_PIN; } while (0)
#define LAND_ON()   do { PIOA_SODR = LAND_PIN; } while (0)
#define LAND_INIT() do { LAND_OFF(); PIOA_PER = LAND_PIN; PIOA_OER = LAND_PIN; } while (0)

static bool beep;
static void beep_start(void)
{
	if (!beep)
	{
		LOG_INFO("Starting the landing beeper\n");
		buz_repeat_start(1000, 3000);
		beep = true;
	}
}

static bool landing;
static void landing_pulse(void)
{
	if (!landing)
	{
		LOG_INFO("Sending to camera the landing pulse\n");
		LAND_ON();
		timer_delay(300);
		LAND_OFF();

		landing = true;
		beep_start();
	}
}


static int32_t landing_alt;
static int32_t prev_alt;
static int alt_cnt;
static int alt_cnt_limit;

static ticks_t buz_start_time;
static ticks_t buz_time;

static void landing_altReset(void)
{
	alt_cnt = 0;
	prev_alt = 0;
}

static void NORETURN landing_process(void)
{
	while (1)
	{
		timer_delay(20000);

		if (gps_fixed())
		{
			int32_t curr_alt = gps_info()->altitude;
			int32_t delta = curr_alt - prev_alt;
			prev_alt = curr_alt;

			if (delta < 0)
				alt_cnt++;
			else
			{
				if (alt_cnt > 0)
					alt_cnt--;
			}

			if (alt_cnt > alt_cnt_limit && curr_alt < landing_alt)
			{
				static bool logging;
				if (!logging)
				{
					LOG_INFO("Altitude descending and current altidude lower than %ld m, activating landing mode\n", landing_alt);
					logging = true;
				}
				landing_pulse();
			}
		}
		else
			landing_altReset();

		if (timer_clock() - buz_start_time > buz_time)
		{
			static bool logging;
			if (!logging)
			{
				LOG_INFO("Buzzer timeout expired\n");
				logging = true;
			}

			beep_start();
		}
	}
}


void landing_reset(void)
{
	LOG_INFO("Resetting landing control data\n");
	landing = false;
	beep = false;
	buz_repeat_stop();
	buz_start_time = timer_clock();
	landing_altReset();
}




void landing_init(int32_t landing_meters, int count_limit, uint32_t buz_timeout_seconds)
{
	LAND_INIT();
	MOD_CHECK(buzzer);
	alt_cnt_limit = count_limit;
	landing_alt = landing_meters;
	buz_time = ms_to_ticks(buz_timeout_seconds * 1000);

	LOG_INFO("Starting landing control process:\n");
	LOG_INFO(" Altitude meters: %ld meters\n", landing_alt);
	LOG_INFO(" Altidude descending counter limit: %d\n", alt_cnt_limit);
	LOG_INFO(" Buzzer timeout: %ld seconds\n", buz_timeout_seconds);
	landing_reset();

	proc_new(landing_process, NULL, KERN_MINSTACKSIZE * 2, NULL);
}
