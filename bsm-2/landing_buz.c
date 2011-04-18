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

#include "landing_buz.h"

#include "status_mgr.h"
#include "hw/hw_pin.h"


#include <cfg/compiler.h>
#include <cfg/module.h>
#include <drv/timer.h>
#include <drv/buzzer.h>

#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>

#include <math.h>

static bool beep;
void landing_buz_start(void)
{
	if (!beep)
	{
		LOG_INFO("Starting the landing beeper\n");
		buz_repeat_start(1000, 3000);
		beep = true;
	}
}

static ticks_t buz_time;

bool landing_buz_check(ticks_t now)
{
	static bool logging = false;

	if (now - status_missionStartTicks() > buz_time)
	{
		if (!logging)
		{
			LOG_INFO("Buzzer timeout expired\n");
			logging = true;
		}

		return false;
	}
	else
	{
		logging = false;
		return true;
	}
}

static void NORETURN landing_buz_process(void)
{
	while (1)
	{
		timer_delay(1000);

		if (!landing_buz_check(timer_clock()))
			landing_buz_start();
	}
}


void landing_buz_reset(void)
{
	LOG_INFO("Resetting landing control data\n");
	beep = false;
	buz_repeat_stop();
}

void landing_buz_setCfg(uint32_t buz_timeout_seconds)
{
	buz_time = ms_to_ticks(buz_timeout_seconds * 1000);
	LOG_INFO("Buzzer timeout: %ld seconds\n", buz_timeout_seconds);
}

void landing_buz_init(uint32_t buz_timeout_seconds)
{
	MOD_CHECK(buzzer);
	landing_buz_setCfg(buz_timeout_seconds);
	landing_buz_reset();
	LOG_INFO("Starting landing buzzer control process:\n");
	proc_new(landing_buz_process, NULL, KERN_MINSTACKSIZE * 2, NULL);
}
