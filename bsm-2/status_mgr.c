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
#include "uplink.h"

#include "hw/hw_pin.h"

#include <algo/moving_avg.h>
#include <mware/config.h>

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

	#include "logging.h"
	#undef LOG_INFO
	#define LOG_INFO(...) logging_msg(__VA_ARGS__)
	#define RESET() \
		do { \
			RSTC_CR = RSTC_KEY | BV(RSTC_EXTRST) | BV(RSTC_PERRST) | BV(RSTC_PROCRST); \
		} while(0)

#else
	#define CAMPULSE_OFF()  do {  } while (0)
	#define CAMPULSE_ON()   do {  } while (0)
	#define CAMPULSE_INIT() do {  } while (0)
	#define RESET() do { LOG_INFO("********RESET********\n"); } while(0)

#endif

static ticks_t mission_start_ticks;

static void status_reload(void);

DECLARE_CONF(status, status_reload,
	CONF_INT(ground_alt, 10, 9000, 1500), //meters
	CONF_INT(tropopause_alt, 9000, 20000, 12500), //meters
	CONF_INT(landing_alt, 10, 100000, 3600), //meters
	CONF_FLOAT(rate_up, 0, +20, +2.00), // m/s
	CONF_FLOAT(rate_down, -20, +0, -2.00) //  m/s (should be negative!)
);

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
		radio_printf("Changing status to %s\n", status_names[new_status]);
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
static const mtime_t cam_pulse[] =
{
	NO_PULSE, // BSM2_NOFIX
	NO_PULSE, // BSM2_GROUND_WAIT,
	NO_PULSE, // BSM2_TAKEOFF,
	100, // BSM2_STRATOPHERE_UP,
	300, // BSM2_STRATOPHERE_FALL,
	500, // BSM2_FALLING,
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

#define DELTA_MEAN_TIME 60 //seconds

#define DELTA_MEAN_LEN (DELTA_MEAN_TIME / STATUS_CHECK_INTERVAL)

static int32_t prev_alt;
static MOVING_AVG_DEFINE(int32_t, alt_delta, DELTA_MEAN_LEN);

static float prev_press;
static MOVING_AVG_DEFINE(float, press_delta, DELTA_MEAN_LEN);

static VertDir curr_vdir = HOVERING;

static void status_reset(void)
{
	LOG_INFO("Resetting status control data\n");

	status_set(BSM2_NOFIX);

	MOVING_AVG_RESET(&alt_delta);
	MOVING_AVG_RESET(&press_delta);
	curr_vdir = HOVERING;
}

static VertDir vertical_dir(float rate)
{
	float up_limit = rate_up;
	float down_limit = rate_down;

	switch (curr_vdir)
	{
		case HOVERING:
			break;
		case UP:
			up_limit = 0;
			break;
		case DOWN:
			down_limit = 0;
			break;
		default:
			ASSERT(0);
	}

	if (rate > up_limit)
		curr_vdir = UP;
	else if (rate < down_limit)
		curr_vdir = DOWN;
	else
		curr_vdir = HOVERING;

	return curr_vdir;
}

static int32_t GROUND_ALT(void)
{
	return ground_alt;
}

#define ALT_HIST  750

static int32_t TROPOPAUSE_ALT(void)
{
	if (curr_status == BSM2_STRATOPHERE_UP)
		return tropopause_alt - ALT_HIST;
	if (curr_status == BSM2_FALLING)
		return tropopause_alt + ALT_HIST;
	else
		return tropopause_alt;
}

static int32_t LANDING_ALT(void)
{
	if (curr_status == BSM2_LANDING)
		return landing_alt + ALT_HIST;
	else
		return landing_alt;
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
		if (curr_alt >= GROUND_ALT())
			rate = MOVING_AVG_GET(&alt_delta, float) / STATUS_CHECK_INTERVAL;
		else
			// If pressure decreases of 1 mBar we have gained ~9 meters in height.
			rate = -9.0 * (MOVING_AVG_GET(&press_delta) / STATUS_CHECK_INTERVAL);

		//LOG_INFO("Ascent rate %.2f m/s\n", rate);

		if (vertical_dir(rate) == HOVERING
			&& curr_alt < GROUND_ALT())
		{
			status_set(BSM2_GROUND_WAIT);
		}
		else if (vertical_dir(rate) == HOVERING
			&& curr_alt >= GROUND_ALT())
		{
			status_set(BSM2_HOVERING);
		}
		else if (vertical_dir(rate) == UP
			&& curr_alt < TROPOPAUSE_ALT())
		{
			status_set(BSM2_TAKEOFF);
		}
		else if (vertical_dir(rate) == UP
			&& curr_alt >= TROPOPAUSE_ALT())
		{
			status_set(BSM2_STRATOPHERE_UP);
		}
		else if (vertical_dir(rate) == DOWN
			&& curr_alt >= TROPOPAUSE_ALT())
		{
			status_set(BSM2_STRATOPHERE_FALL);
		}
		else if (vertical_dir(rate) == DOWN
			&& curr_alt < LANDING_ALT())
		{
			status_set(BSM2_LANDING);
		}
		else if (vertical_dir(rate) == DOWN
			&& curr_alt < TROPOPAUSE_ALT())
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
	radio_printf("Mission start at %ld\n", (long)ticks);
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

static void status_reload(void)
{
	LOG_INFO("Setting status configuration\n");
	LOG_INFO(" max ground altitude: %ld m\n", (long)ground_alt);
	LOG_INFO(" tropopause altitude: %ld m\n", (long)tropopause_alt);
	LOG_INFO(" landing altitude: %ld m\n", (long)landing_alt);
	LOG_INFO(" ascent rate (UP): %.2f m/s\n", rate_up);
	LOG_INFO(" descent rate (DOWN): %.2f m/s\n", rate_down);
	status_reset();
}

static bool mission_start(long l)
{
	(void)l;
	status_missionStart();
	return true;
}

static bool board_reset(long code)
{
	if (code == 0xdead)
	{
		radio_printf("Resetting board, bye bye!\n");
		timer_delay(1000);
		RESET();
	}
	LOG_INFO ("Wrong reset code: %lx\n", code);
	return false;
}

void status_init(void)
{
	config_register(&status);
	config_load(&status);

	uplink_registerCmd("mission_start", mission_start);
	uplink_registerCmd("reset", board_reset);

	CAMPULSE_INIT();
	status_missionStart();
	#if !(ARCH & ARCH_UNITTEST)
		LOG_INFO("Starting status check process\n");
		proc_new(status_process, NULL, KERN_MINSTACKSIZE * 5, NULL);
		LOG_INFO("Starting camera communication process\n");
		proc_new(camera_process, NULL, KERN_MINSTACKSIZE * 2, NULL);
	#endif
}
