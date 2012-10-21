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
#include "radio.h"

#include "status_mgr.h"
#include "hw/hw_pin.h"

#include <cfg/compiler.h>
#include <cfg/module.h>
#include <drv/timer.h>
#include <drv/buzzer.h>
#include <mware/config.h>

#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>
#include "logging.h"

#include <math.h>

#if !(ARCH & ARCH_UNITTEST)
	#define BUZ_START() buz_repeat_start(1000, 3000)
	#define BUZ_STOP() 	buz_repeat_stop();
#else
	#define BUZ_START() /**/
	#define BUZ_STOP()  /**/
#endif

static bool beep;
void landing_buz_start(void)
{
	if (!beep)
	{
		radio_printf("Beeper started\n");
		BUZ_START();
		beep = true;
	}
}

static void landing_buz_reload(void);

DECLARE_CONF(landing_buz, landing_buz_reload,
	CONF_INT(buz_timeout, 5, 86400, 9000) // seconds
);

static void landing_buz_reload(void)
{
	LOG_INFO("Buzzer timeout: %ld seconds\n", (long)buz_timeout);
	landing_buz_reset();
}


bool landing_buz_check(ticks_t now)
{
	static bool logging = false;

	if (now - status_missionStartTicks() > (buz_timeout * 1000))
	{
		if (!logging)
		{
			radio_printf("Beeper timeout expired\n");
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
	LOG_INFO("Resetting landing buzzer control data\n");
	beep = false;
	BUZ_STOP();
}

void landing_buz_init(void)
{
	config_register(&landing_buz);
	config_load(&landing_buz);
	landing_buz_reset();
	#if !(ARCH & ARCH_UNITTEST)
		buz_init();
		LOG_INFO("Starting landing buzzer control process:\n");
		Process *p = proc_new(landing_buz_process, NULL, KERN_MINSTACKSIZE * 5, NULL);
		ASSERT(p);
	#endif
}
