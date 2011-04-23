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

#include "status_mgr.h"
#include "gps.h"
#include "sensors.h"
#include "radio.h"

#include "hw/hw_pin.h"
#include "cfg/cfg_afsk.h"

#include <net/nmea.h>
#include <cfg/compiler.h>
#include <drv/timer.h>
#include <drv/pwm.h>
#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>

#include <math.h>
#include <string.h>

#if !(ARCH & ARCH_UNITTEST)
	#ifdef DEMO_BOARD
		#define CUTOFF_OFF()  do { PIOA_CODR = CUTOFF_PIN; } while (0)
		#define CUTOFF_ON()   do { PIOA_SODR = CUTOFF_PIN; } while (0)
		#define CUTOFF_INIT(cfg) do { CUTOFF_OFF(); PIOA_PER = CUTOFF_PIN; PIOA_OER = CUTOFF_PIN; } while (0)
	#else
		static Pwm cutoff_pwm;

		#define CUTOFF_OFF()  pwm_enable(&cutoff_pwm, false)
		#define CUTOFF_ON()   pwm_enable(&cutoff_pwm, true)
		#define CUTOFF_INIT(cfg) do { \
			pwm_init(&cutoff_pwm, CUTOFF1_PWM); \
			pwm_setFrequency(&cutoff_pwm, CONFIG_AFSK_DAC_SAMPLERATE / 8); \
			pwm_setDuty(&cutoff_pwm, cfg->pwm_duty); \
		} while (0)
	#endif
#else
	#define CUTOFF_OFF()  do {  } while (0)
	#define CUTOFF_ON()   do {  } while (0)
	#define CUTOFF_INIT(cfg) do {  } while (0)
#endif

static CutoffCfg cfg;

static bool test_mode;

void cutoff_setTestmode(bool mode)
{
	test_mode = mode;
}

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
static float distance(udegree_t _lat1, udegree_t _lon1, udegree_t _lat2, udegree_t _lon2)
{
	float lat1 = _lat1 / 1000000.0;
	float lon1 = _lon1 / 1000000.0;
	float lat2 = _lat2 / 1000000.0;
	float lon2 = _lon2 / 1000000.0;
	const float PLANET_RADIUS = 6371000;
	float d_lat = deg2rad(lat2 - lat1);
	float d_lon = deg2rad(lon2 - lon1);

	float a = sin(d_lat / 2) * sin(d_lat / 2) +
			cos(deg2rad(lat1)) * cos(deg2rad(lat2)) *
			sin(d_lon / 2) * sin(d_lon / 2);
	float c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return PLANET_RADIUS * c;
}


static bool dist_ok = true;

bool cutoff_checkDist(udegree_t lat, udegree_t lon, ticks_t now)
{
	static ticks_t dist_ko_time;

	if (status_currStatus() != BSM2_GROUND_WAIT
		&& status_currStatus() != BSM2_NOFIX)
	{
		float curr_dist = distance(cfg.start_latitude, cfg.start_longitude, lat, lon);
		if (curr_dist > cfg.dist_max_meters)
		{
			static bool logged = false;

			if (dist_ok)
			{
				LOG_INFO("Distance from base: %.0fm; limit %ldm, starting %lds timeout\n",
					curr_dist, (long)cfg.dist_max_meters, (long)cfg.dist_timeout);
				dist_ok = false;
				dist_ko_time = now;
				logged = false;
			}
			else if (now - dist_ko_time > ms_to_ticks(cfg.dist_timeout * 1000))
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

#define MIN_ALTITUDE -2000

static int32_t alt_max = MIN_ALTITUDE;
static bool alt_ok = true;

static void alt_reset(void)
{
	alt_ok = true;
	alt_max = MIN_ALTITUDE;
}


bool cutoff_checkAltitude(int32_t curr_alt, ticks_t now)
{
	static ticks_t alt_ko_time;

	if (status_currStatus() != BSM2_GROUND_WAIT
		&& status_currStatus() != BSM2_NOFIX)
	{
		alt_max = MAX(alt_max, curr_alt);

		if (alt_max - curr_alt > cfg.delta_altitude)
		{
			static bool logged = false;

			if (alt_ok)
			{
				LOG_INFO("Current altitude %ld, max altitude %ld; current altitude lower than delta, starting %ld s timeout\n",
					(long)curr_alt, (long)alt_max, (long)cfg.altitude_timeout);
				alt_ok = false;
				logged = false;
				alt_ko_time = now;
			}
			else if (now - alt_ko_time > ms_to_ticks(cfg.altitude_timeout * 1000))
			{
				if (!logged)
				{
					LOG_INFO("Current altitude lower than delta and timeout expired\n");
					logged = true;
				}
				return false;
			}
			return true;
		}
	}

	if (!alt_ok)
		LOG_INFO("Current altitude ok\n");
	alt_ok = true;
	return true;
}

bool cutoff_checkTime(ticks_t now)
{
	static bool logged = false;

	if (now - status_missionStartTicks() < ms_to_ticks(cfg.mission_timeout * 1000))
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

void cutoff_test_cut(bool on)
{
	if (test_mode)
	{
		if (on)
			CUTOFF_ON();
		else
			CUTOFF_OFF();
	}
}

static bool cut = false;
static void cutoff_cut(void)
{
		if (!cut && !test_mode)
		{
			cut = true;
			LOG_INFO("---CUTOFF ACTIVATED---\n");
			radio_printf("---CUTOFF ACTIVATED---");
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

		ticks_t now = timer_clock();
		int32_t curr_alt = gps_info()->altitude;
		udegree_t lat = gps_info()->latitude;
		udegree_t lon = gps_info()->longitude;

		if (!cutoff_checkTime(now)
		 || !cutoff_checkDist(lat, lon, now)
		 || !cutoff_checkAltitude(curr_alt, now))
			cutoff_cut();

	}
}

void cutoff_reset(void)
{
	LOG_INFO("Resetting cutoff procedure\n");
	alt_reset();
	dist_ok = true;

	cut = false;
}

void cutoff_setCfg(CutoffCfg *_cfg)
{
	memcpy(&cfg, _cfg, sizeof(cfg));
	LOG_INFO("Setting cutoff configuration\n");
	LOG_INFO(" mission timeout: %ld seconds\n", (long)cfg.mission_timeout);
	LOG_INFO(" max delta altitude: %ld m\n", (long)cfg.delta_altitude);
	LOG_INFO(" delta pressure timeout: %ld seconds\n", (long)cfg.altitude_timeout);
	LOG_INFO(" base coordinates: %02ld.%.06ld %03ld.%.06ld\n",
		(long)cfg.start_latitude/1000000, (long)ABS(cfg.start_latitude)%1000000,
		(long)cfg.start_longitude/1000000, (long)ABS(cfg.start_longitude)%1000000);
	LOG_INFO(" max distance from base: %ld meters\n", (long)cfg.dist_max_meters);
	LOG_INFO(" max distance timeout: %ld seconds\n", (long)cfg.dist_timeout);
	LOG_INFO(" pwm duty 0x%04X\n", cfg.pwm_duty);
}


void cutoff_init(CutoffCfg *cfg)
{
	cutoff_setCfg(cfg);
	CUTOFF_INIT(cfg);
	cutoff_reset();
	//start process
	LOG_INFO("Starting cutoff process\n");
	proc_new(cutoff_process, NULL, KERN_MINSTACKSIZE * 3, NULL);
}
