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
 * \brief Cutoff control.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "cutoff.h"

#include "gps.h"
#include "sensors.h"
#include "hw/hw_pin.h"

#include <net/nmea.h>
#include <cfg/compiler.h>
#include <drv/timer.h>
#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>

#include <math.h>

#define CUTOFF_OFF()  do { PIOA_CODR = CUTOFF_PIN; } while (0)
#define CUTOFF_ON()   do { PIOA_SODR = CUTOFF_PIN; } while (0)
#define CUTOFF_INIT() do { CUTOFF_OFF(); PIOA_PER = CUTOFF_PIN; PIOA_OER = CUTOFF_PIN; } while (0)

static float start_lat;
static float start_lon;
static float max_dist;


#define PI 3.14159265358979323846

INLINE float deg2rad(float deg)
{
	return deg * PI / 180;
}

INLINE float rad2deg(float rad)
{
	return rad * 180 / PI;
}


/**
 * Use the Haversine formula to calculate great-circle distances between the
 * two points.
 *
 * The Haversine formula remains particularly well-conditioned for numerical
 * computation even at small distances, unlike calculations based on the
 * spherical law of cosines.
 */
static float distance(float lat1, float lon1, float lat2, float lon2)
{
	const float PLANET_RADIUS = 6371000;
	float d_lat = deg2rad(lat2 - lat1);
	float d_lon = deg2rad(lon2 - lon1);

	float a = sin(d_lat / 2) * sin(d_lat / 2) +
			cos(deg2rad(lat1)) * cos(deg2rad(lat2)) *
			sin(d_lon / 2) * sin(d_lon / 2);
	float c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return PLANET_RADIUS * c;
}

static ticks_t maxdist_timeout;
static bool dist_ok = true;
static void dist_reset(void)
{
	dist_ok = true;
}

static bool cutoff_checkDist(void)
{
	static ticks_t dist_ko_time;

	if (gps_fixed())
	{
		float lat = gps_info()->latitude / 1000000.0;
		float lon = gps_info()->longitude / 1000000.0;

		float curr_dist = distance(start_lat, start_lon, lat, lon);
		if (curr_dist > max_dist)
		{
			static bool logged = false;

			if (dist_ok)
			{
				LOG_INFO("Distance from base: %.0fm; limit %.0fm, starting %lds timeout\n", curr_dist, max_dist, ticks_to_ms(maxdist_timeout) / 1000);
				dist_ok = false;
				dist_ko_time = timer_clock();
				logged = false;
			}
			else if (timer_clock() - dist_ko_time > maxdist_timeout)
			{
				if (!logged)
				{
					LOG_INFO("Maximum distance from base exceeded and timeout expired\n");
					logged = true;
				}
				return false;
			}

			return true;
		}
	}

	if (!dist_ok)
		LOG_INFO("Distance from base ok\n");

	dist_ok = true;
	return true;
}

#define PRESS_PERIOD 60
#define MAX_PRESS 1500

static float delta_press;
static ticks_t delta_timeout;
static float press[PRESS_PERIOD];
static float press_sum;
static float press_min = MAX_PRESS;
static int press_idx;
static int press_cnt = 0;
static bool press_ok = true;


#define NEXT_IDX(idx) (((idx + 1) >= PRESS_PERIOD) ? 0 : idx + 1)

static void press_reset(void)
{
	press_cnt = 0;
	press_ok = true;
	press_idx = 0;
	press_sum = 0;
	press_min = MAX_PRESS;
}

static bool cutoff_checkPress(void)
{
	static ticks_t press_ko_time;

	float curr_press = sensor_press();
	press[press_idx] = curr_press;
	ASSERT(press[press_idx] < MAX_PRESS);

	press_sum += press[press_idx];
	press_idx = NEXT_IDX(press_idx);

	if (press_cnt < PRESS_PERIOD)
		press_cnt++;

	float press_avg = press_sum / press_cnt;
	if (press_cnt >= PRESS_PERIOD)
		press_sum -= press[press_idx];

	press_min = MIN(press_min, press_avg);
	//LOG_INFO("press_cnt %d Current pressure %.0f, current average pressure %.0f, min pressure %.0f\n", press_cnt, curr_press, press_avg, press_min);

	if (press_avg - press_min > delta_press)
	{
		static bool logged = false;

		if (press_ok)
		{
			LOG_INFO("Current pressure %.0f, current average pressure %.0f, min pressure %.0f; avg pressure greater than delta, starting %ld s timeout\n",
			curr_press, press_avg, press_min, ticks_to_ms(delta_timeout) / 1000);
			press_ok = false;
			logged = false;
			press_ko_time = timer_clock();
		}
		else if (timer_clock() - press_ko_time > delta_timeout)
		{
			if (!logged)
			{
				LOG_INFO("Average pressure greater than delta pressure and timeout expired\n");
				logged = true;
			}
			return false;
		}
		return true;
	}

	if (!press_ok)
		LOG_INFO("Average pressure ok\n");
	press_ok = true;
	return true;
}


static ticks_t start_time;
static ticks_t mission_time;
static bool cutoff_checkTime(void)
{
	static bool logged = false;

	if (timer_clock() - start_time < mission_time)
	{
		logged = false;
		return true;
	}
	else
	{
		if (!logged)
		{
			LOG_INFO("Maximum mission time expired\n");
			logged = true;
		}
		return false;
	}
}

static bool cut = false;
static void cutoff_cut(void)
{
		if (!cut)
		{
			cut = true;
			LOG_INFO("---CUTOFF ACTIVATED---\n");
			for (int i = 0; i < 3; i++)
			{
				LOG_INFO("Cutoff pulse %d\n", i+1);
				CUTOFF_ON();
				timer_delay(5000);
				CUTOFF_OFF();
				LOG_INFO("Cutoff pulse %d done\n", i+1);
				timer_delay(5000);
			}
			LOG_INFO("Cutoff procedure finished.\n");
		}
}

static void NORETURN cutoff_process(void)
{
	while (1)
	{
		timer_delay(1000);

		if (!cutoff_checkTime()
		 || !cutoff_checkDist()
		 || !cutoff_checkPress())
			cutoff_cut();

	}
}

void cutoff_reset(void)
{
	LOG_INFO("Resetting cutoff procedure\n");
	start_time = timer_clock();
	press_reset();
	dist_reset();

	cut = false;
}

void cutoff_init(uint32_t max_seconds, float _delta_press, uint32_t _delta_timeout, udegree_t _start_lat, udegree_t _start_lon, uint32_t max_meters, uint32_t _maxdist_timeout)
{
	CUTOFF_INIT();
	mission_time = ms_to_ticks(max_seconds * 1000);
	maxdist_timeout = ms_to_ticks(_maxdist_timeout * 1000);
	start_lat = _start_lat / 1000000.0;
	start_lon = _start_lon / 1000000.0;
	max_dist = max_meters;
	delta_press = _delta_press;
	delta_timeout = ms_to_ticks(_delta_timeout * 1000);

	LOG_INFO("Starting cutoff:\n");
	LOG_INFO(" max mission time: %ld seconds\n", max_seconds);
	LOG_INFO(" max delta pressure: %.0f mbar\n", delta_press);
	LOG_INFO(" delta pressure timeout: %ld seconds\n", _delta_timeout);
	LOG_INFO(" base coordinates: %02ld.%.06ld %03ld.%.06ld\n",
		_start_lat/1000000, ABS(_start_lat)%1000000, _start_lon/1000000, ABS(_start_lon)%1000000);
	LOG_INFO(" max distance from base: %ld meters\n", max_meters);
	LOG_INFO(" max distance timeout: %ld seconds\n", _maxdist_timeout);
	cutoff_reset();
	//start process
	proc_new(cutoff_process, NULL, KERN_MINSTACKSIZE * 3, NULL);
}
