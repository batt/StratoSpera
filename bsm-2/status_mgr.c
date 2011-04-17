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
#include "hw/hw_pin.h"

#include <cfg/compiler.h>
#include <cfg/module.h>
#include <drv/timer.h>

#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>

#include <math.h>

#define CAMPULSE_OFF()  do { PIOA_CODR = CAMPULSE_PIN; } while (0)
#define CAMPULSE_ON()   do { PIOA_SODR = CAMPULSE_PIN; } while (0)
#define CAMPULSE_INIT() \
	do { \
		CAMPULSE_OFF(); \
		PIOA_PER = CAMPULSE_PIN; \
		PIOA_OER = CAMPULSE_PIN; \
	} while (0)

#define STATUS_CHECK_INTERVAL 10 //seconds
#define DELTA_MEAN_TIME 60 //seconds

#define DELTA_MEAN_LEN (DELTA_MEAN_TIME / STATUS_CHECK_INTERVAL)

static int32_t prev_alt;

static int32_t delta_values[DELTA_MEAN_LEN];
static int32_t delta_sum = 0;
static int delta_cnt = 0;
static int delta_idx = 0;

static ticks_t mission_start_ticks;

#define NEXT_IDX(idx) (((idx + 1) >= DELTA_MEAN_LEN) ? 0 : idx + 1)


static StatusCfg cfg;
static Bsm2Status curr_status;

static void status_set(Bsm2Status new_status)
{
	ASSERT(new_status < BSM2_CNT);
	if (new_status != curr_status)
		LOG_INFO("Changing status to %d\n", new_status);
	curr_status = new_status;
}

#define CAMPULSE_MIN_DELAY 300
#define CAMPULSE_INC 50

static void NORETURN camera_process(void)
{
	while (1)
	{
		timer_delay(1000);
		CAMPULSE_ON();
		timer_delay(CAMPULSE_MIN_DELAY + curr_status * CAMPULSE_INC);
		CAMPULSE_OFF();
	}
}

static void status_reset(void)
{
	LOG_INFO("Resetting status control data\n");

	status_set(BSM2_NOFIX);

	delta_cnt = 0;
	delta_idx = 0;
	delta_sum = 0;
}


void status_check(bool fix, int32_t curr_alt)
{
	if (fix)
	{
		if (delta_cnt == 0)
			prev_alt = curr_alt;

		int32_t delta = curr_alt - prev_alt;
		prev_alt = curr_alt;

		delta_values[delta_idx] = delta;

		delta_sum += delta;
		delta_idx = NEXT_IDX(delta_idx);

		if (delta_cnt < DELTA_MEAN_LEN)
			delta_cnt++;

		int32_t delta_avg = delta_sum / delta_cnt;
		if (delta_cnt >= DELTA_MEAN_LEN)
			delta_sum -= delta_values[delta_idx];

		if (cutoff_active())
		{
			status_set(BSM2_CUTOFF);
		}
		else if (delta_avg < cfg.delta_up
			&& delta_avg > cfg.delta_down
			&& curr_alt < cfg.ground_alt)
		{
			status_set(BSM2_GROUND_WAIT);
		}
		else if (delta_avg < cfg.delta_up
			&& delta_avg > cfg.delta_down
			&& curr_alt >= cfg.ground_alt)
		{
			status_set(BSM2_HOVERING);
		}
		else if (delta_avg > cfg.delta_up
			&& curr_alt < cfg.tropopause_alt)
		{
			status_set(BSM2_TAKEOFF);
		}
		else if (delta_avg > cfg.delta_up
			&& curr_alt >= cfg.tropopause_alt)
		{
			status_set(BSM2_STRATOPHERE_UP);
		}
		else if (delta_avg < cfg.delta_down
			&& curr_alt >= cfg.tropopause_alt)
		{
			status_set(BSM2_STRATOPHERE_FALL);
		}
		else if (delta_avg < cfg.delta_down
			&& curr_alt < cfg.landing_alt)
		{
			status_set(BSM2_LANDING);
		}
		else if (delta_avg < cfg.delta_down
			&& curr_alt < cfg.tropopause_alt)
		{
			status_set(BSM2_STRATOPHERE_FALL);
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
		status_check(gps_fixed(), gps_info()->altitude);
	}
}

void status_missionStart(void)
{
	mission_start_ticks = timer_clock();
	status_reset();
}

ticks_t status_missionStartTicks(void)
{
	return mission_start_ticks;
}


void status_init()
{
	CAMPULSE_INIT();
	proc_new(status_process, NULL, KERN_MINSTACKSIZE * 3, NULL);
	proc_new(camera_process, NULL, KERN_MINSTACKSIZE, NULL);
	status_missionStart();
}
