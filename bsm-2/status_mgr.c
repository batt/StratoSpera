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

#include "status_mgr.h"

#include "cutoff.h"
#include "gps.h"
#include "landing_buz.h"
#include "radio.h"
#include "testmode.h"
#include "sensors.h"

#include "hw/hw_pin.h"

#include <algo/moving_avg.h>

#include <cfg/compiler.h>
#include <cfg/module.h>
#include <drv/timer.h>

#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>

#include <string.h>

#if !(ARCH & ARCH_UNITTEST)
	#define CAMPULSE_OFF()  do { PIOA_CODR = CAMPULSE_PIN; } while (0)
	#define CAMPULSE_ON()   do { PIOA_SODR = CAMPULSE_PIN; } while (0)
	#define CAMPULSE_INIT() \
		do { \
			CAMPULSE_OFF(); \
			PIOA_PER = CAMPULSE_PIN; \
			PIOA_OER = CAMPULSE_PIN; \
		} while (0)
#else
	#define CAMPULSE_OFF()  do {  } while (0)
	#define CAMPULSE_ON()   do {  } while (0)
	#define CAMPULSE_INIT() do {  } while (0)
#endif

static ticks_t mission_start_ticks;

static StatusCfg cfg;
static Bsm2Status curr_status;

static const char *status_names[] =
{
	"NOFIX",
	"GROUND_WAIT",
	"TAKEOFF",
	"STRATOPHERE_UP",
	"STRATOPHERE_FALL",
	"FALLING",
	"LANDING",
	"HOVERING",
};

void status_setTestStatus(Bsm2Status new_status)
{
	if (testmode())
	{
		curr_status = new_status;
		LOG_INFO("Changing status to %s\n", status_names[new_status]);
	}
}

STATIC_ASSERT(countof(status_names) == BSM2_CNT);

static void status_set(Bsm2Status new_status)
{
	ASSERT(new_status < BSM2_CNT);
	if (new_status != curr_status)
	{
		LOG_INFO("Changing status to %s\n", status_names[new_status]);
		#if !(ARCH & ARCH_UNITTEST)
			radio_printf("Changing status to %s", status_names[new_status]);
		#endif
	}

	curr_status = new_status;

	if (curr_status == BSM2_LANDING)
		landing_buz_start();
}

Bsm2Status status_currStatus(void)
{
	return curr_status;
}

#define NO_PULSE -1
static mtime_t cam_pulse[] =
{
	NO_PULSE, // BSM2_NOFIX
	NO_PULSE, // BSM2_GROUND_WAIT,
	NO_PULSE, // BSM2_TAKEOFF,
	100, // BSM2_STRATOPHERE_UP,
	200, // BSM2_STRATOPHERE_FALL,
	400, // BSM2_FALLING,
	800, // BSM2_LANDING,
	NO_PULSE, //BSM2_HOVERING,
};

STATIC_ASSERT(countof(cam_pulse) == BSM2_CNT);

static void NORETURN camera_process(void)
{
	while (1)
	{
		timer_delay(1000);
		if (cam_pulse[curr_status] != NO_PULSE)
		{
			CAMPULSE_ON();
			timer_delay(cam_pulse[curr_status]);
			CAMPULSE_OFF();
		}
	}
}

#define STATUS_CHECK_INTERVAL 10 //seconds
#define DELTA_MEAN_TIME 60 //seconds

#define DELTA_MEAN_LEN (DELTA_MEAN_TIME / STATUS_CHECK_INTERVAL)

static int32_t prev_alt;
static MOVING_AVG_DEFINE(int32_t, alt_delta, DELTA_MEAN_LEN);

static float prev_press;
static MOVING_AVG_DEFINE(float, press_delta, DELTA_MEAN_LEN);


static void status_reset(void)
{
	LOG_INFO("Resetting status control data\n");

	status_set(BSM2_NOFIX);

	MOVING_AVG_RESET(&alt_delta);
	MOVING_AVG_RESET(&press_delta);
}

static int32_t rate_up(void)
{
	if (curr_status == BSM2_TAKEOFF
	 || curr_status == BSM2_STRATOPHERE_UP)
		return 0;
	else
		return cfg.rate_up;
}

static int32_t rate_down(void)
{
	if (curr_status == BSM2_STRATOPHERE_FALL
	 || curr_status == BSM2_FALLING
	 || curr_status == BSM2_LANDING)
		return 0;
	else
		return cfg.rate_down;
}

static int32_t ground_alt(void)
{
	return cfg.ground_alt;
}

#define ALT_HIST  750

static int32_t tropopause_alt(void)
{
	if (curr_status == BSM2_STRATOPHERE_UP)
		return cfg.tropopause_alt - ALT_HIST;
	if (curr_status == BSM2_FALLING)
		return cfg.tropopause_alt + ALT_HIST;
	else
		return cfg.tropopause_alt;
}

static int32_t landing_alt(void)
{
	if (curr_status == BSM2_LANDING)
		return cfg.landing_alt + ALT_HIST;
	else
		return cfg.landing_alt;
}

void status_check(bool fix, int32_t curr_alt, float curr_press)
{
	if (fix)
	{
		if (MOVING_AVG_EMPTY(&alt_delta))
		{
			prev_alt = curr_alt;
			prev_press = curr_press;
		}

		int32_t delta_alt = curr_alt - prev_alt;
		prev_alt = curr_alt;

		float delta_press = curr_press - prev_press;
		prev_press = curr_press;

		MOVING_AVG_PUSH(&alt_delta, delta_alt);
		MOVING_AVG_PUSH(&press_delta, delta_press);

		if (!MOVING_AVG_FULL(&alt_delta))
		{
			// Stay in the previuos state until we have a good
			// approximation of the ascent rate.
			return;
		}

		// In order to compute ascent rate, at low altitudes we use the
		// pressure sensor which is more accurate than the GPS.
		float rate;
		if (curr_alt >= ground_alt())
			rate = MOVING_AVG_GET(&alt_delta, float) / STATUS_CHECK_INTERVAL;
		else
			// If pressure decreases of 1 mBar we have gained ~9 meters in height.
			rate = -9.0 * (MOVING_AVG_GET(&press_delta) / STATUS_CHECK_INTERVAL);

		//LOG_INFO("Ascent rate %.2f m/s\n", rate);

		if (rate < rate_up()
			&& rate >= rate_down()
			&& curr_alt < ground_alt())
		{
			status_set(BSM2_GROUND_WAIT);
		}
		else if (rate < rate_up()
			&& rate >= rate_down()
			&& curr_alt >= ground_alt())
		{
			status_set(BSM2_HOVERING);
		}
		else if (rate >= rate_up()
			&& curr_alt < tropopause_alt())
		{
			status_set(BSM2_TAKEOFF);
		}
		else if (rate >= rate_up()
			&& curr_alt >= tropopause_alt())
		{
			status_set(BSM2_STRATOPHERE_UP);
		}
		else if (rate < rate_down()
			&& curr_alt >= tropopause_alt())
		{
			status_set(BSM2_STRATOPHERE_FALL);
		}
		else if (rate < rate_down()
			&& curr_alt < landing_alt())
		{
			status_set(BSM2_LANDING);
		}
		else if (rate < rate_down()
			&& curr_alt < tropopause_alt())
		{
			status_set(BSM2_FALLING);
		}
		else
		{
			// Should never fall here, unknown state
			ASSERT(0);
			status_reset();
		}
	}
	else
		status_reset();
}

static void NORETURN status_process(void)
{
	while (1)
	{
		timer_delay(STATUS_CHECK_INTERVAL * 1000);
		if (!testmode())
			status_check(gps_fixed(), gps_info()->altitude, sensor_read(ADC_PRESS));
	}
}

void status_missionStartAt(ticks_t ticks)
{
	LOG_INFO("Mission start at %ld\n", (long)ticks);
	#if !(ARCH & ARCH_UNITTEST)
		radio_printf("Mission start");
	#endif
	mission_start_ticks = ticks;
	status_reset();
	landing_buz_reset();
	cutoff_reset();
}

void status_missionStart(void)
{
	status_missionStartAt(timer_clock());
}

mtime_t status_missionTime(void)
{
	return ticks_to_ms(timer_clock() - mission_start_ticks);
}

ticks_t status_missionStartTicks(void)
{
	return mission_start_ticks;
}

void status_setCfg(StatusCfg *_cfg)
{
	memcpy(&cfg, _cfg, sizeof(cfg));
	LOG_INFO("Setting status configuration\n");
	LOG_INFO(" max ground altitude: %ld m\n", (long)cfg.ground_alt);
	LOG_INFO(" tropopause altitude: %ld m\n", (long)cfg.tropopause_alt);
	LOG_INFO(" landing altitude: %ld m\n", (long)cfg.landing_alt);
	LOG_INFO(" ascent rate (UP): %ld m/s\n", (long)cfg.rate_up);
	LOG_INFO(" descent rate (DOWN): %ld m/s\n", (long)cfg.rate_down);
}

void status_init(StatusCfg *cfg)
{
	status_setCfg(cfg);
	CAMPULSE_INIT();
	status_missionStart();
	LOG_INFO("Starting status check process\n");
	proc_new(status_process, NULL, KERN_MINSTACKSIZE * 3, NULL);
	LOG_INFO("Starting camera communication process\n");
	proc_new(camera_process, NULL, KERN_MINSTACKSIZE * 2, NULL);
}
