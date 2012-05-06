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
#include "testmode.h"
#include "uplink.h"

#include "hw/hw_pin.h"
#include "cfg/cfg_afsk.h"

#include <mware/config.h>
#include <net/nmea.h>
#include <cfg/compiler.h>
#include <drv/timer.h>
#include <kern/proc.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>
#include "logging.h"

#include <math.h>
#include <string.h>



#define N_CUTOFF 2

#if !(ARCH & ARCH_UNITTEST)

	#ifdef DEMO_BOARD
		#define CUTOFF_ENABLE(ch, en) \
		do { \
			if (ch == 0) \
			{ \
				if (en) \
					PIOA_SODR = CUTOFF_PIN; \
				else \
					PIOA_CODR = CUTOFF_PIN; \
			} \
		} while (0)

		#define CUTOFF_INIT() do { CUTOFF_OFF(); PIOA_PER = CUTOFF_PIN; PIOA_OER = CUTOFF_PIN; } while (0)
	#else
		#include <drv/pwm.h>
		static Pwm cutoff_ctx[N_CUTOFF];

		/* Table which maps cutoff channels to PWM channels */
		static const int pwm_ch_map[] = {1, 2};
		STATIC_ASSERT(countof(pwm_ch_map) == N_CUTOFF);

		#define CUTOFF_ENABLE(ch, en) \
		{ \
			pwm_setDuty(&cutoff_ctx[ch], pwm_duty[ch]); \
			pwm_enable(&cutoff_ctx[ch], en); \
		}

		#define CUTOFF_INIT() \
		do { \
			for (unsigned i = 0; i < N_CUTOFF; i++) \
			{ \
				pwm_init(&cutoff_ctx[i], pwm_ch_map[i]); \
				pwm_setFrequency(&cutoff_ctx[i], CONFIG_AFSK_DAC_SAMPLERATE / 8); \
			} \
		} while (0)
	#endif
#else
	#define CUTOFF_ENABLE(ch, en) do { (void)ch; (void)en; } while (0)
	#define CUTOFF_INIT() do {  } while (0)
	#define testmode() false
#endif

static void cutoff_reload(void);

static int pwm_duty[N_CUTOFF];

DECLARE_CONF(cutoff, cutoff_reload,
	CONF_INT(mission_timeout, 10, 86400, 8400), //seconds
	CONF_INT(delta_altitude, 10, 5000, 500), //meters
	CONF_INT(altitude_timeout, 0, 300, 30), //seconds
	CONF_FLOAT(start_latitude,   -90,  +90, 43.606414), //decimal degrees
	CONF_FLOAT(start_longitude, -180, +180, 11.311832), //decimal degrees
	CONF_INT(dist_max_meters, 1000, 1000000, 80000), //meters
	CONF_INT(dist_timeout, 0, 1800, 300), // seconds
	CONF_INT(altmax_meters, 20, 500000, 50000), //meters
	CONF_INT(altmax_timeout, 0, 1800, 300), //seconds
	CONF_INT_NODECLARE(pwm_duty1, pwm_duty[0], 0, 0xffff, 0x8000), // Number from 0 to 0xffff
	CONF_INT_NODECLARE(pwm_duty2, pwm_duty[1], 0, 0xffff, 0x8000) // Number from 0 to 0xffff
);

static void cutoff_reload(void)
{
	LOG_INFO("Setting cutoff configuration\n");
	LOG_INFO(" mission timeout: %ld seconds\n", (long)mission_timeout);
	LOG_INFO(" max delta altitude: %ld m\n", (long)delta_altitude);
	LOG_INFO(" delta altitude timeout: %ld seconds\n", (long)altitude_timeout);
	LOG_INFO(" base coordinates: %8.06f %9.06f\n", start_latitude, start_longitude);
	LOG_INFO(" max distance from base: %ld meters\n", (long)dist_max_meters);
	LOG_INFO(" max distance timeout: %ld seconds\n", (long)dist_timeout);
	LOG_INFO(" max altitude: %ld meters\n", (long)altmax_meters);
	LOG_INFO(" max altitude timeout: %ld seconds\n", (long)altmax_timeout);
	for (unsigned i = 0; i < N_CUTOFF; i++)
		LOG_INFO(" pwm duty%d 0x%04X\n", i+1, pwm_duty[i]);
	cutoff_reset();
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

static bool maxalt_ok = true;

static bool cutoff_checkMaxalt(int32_t curr_alt, ticks_t now)
{
	static ticks_t maxalt_ko_time;

	if (status_currStatus() != BSM2_GROUND_WAIT
		&& status_currStatus() != BSM2_NOFIX)
	{
		if (curr_alt > altmax_meters)
		{
			static bool logged = false;

			if (maxalt_ok)
			{
				radio_printf("CUTOFF:altitude over range\n");
				radio_printf("CUTOFF in %lds\n", (long)altmax_timeout);
				LOG_INFO("Altitude: %ldm; limit %ldm, starting %lds timeout\n",
					(long)curr_alt, (long)altmax_meters, (long)altmax_timeout);
				maxalt_ok = false;
				maxalt_ko_time = now;
				logged = false;
			}
			else if (now - maxalt_ko_time > ms_to_ticks(altmax_timeout * 1000))
			{
				if (!logged)
				{
					radio_printf("CUTOFF:altitude timeout\n");
					logged = true;
				}
				return false;
			}

			return true;
		}
	}

	if (!maxalt_ok)
		LOG_INFO("Maximum altitude ok\n");

	maxalt_ok = true;
	return true;
}


static bool dist_ok = true;

static bool cutoff_checkDist(udegree_t lat, udegree_t lon, ticks_t now)
{
	static ticks_t dist_ko_time;

	if (status_currStatus() != BSM2_GROUND_WAIT
		&& status_currStatus() != BSM2_NOFIX)
	{
		float curr_dist = distance(start_latitude, start_longitude, lat / 1e6, lon / 1e6);
		if (curr_dist > dist_max_meters)
		{
			static bool logged = false;

			if (dist_ok)
			{
				radio_printf("CUTOFF:distance over range\n");
				radio_printf("CUTOFF in %lds\n", (long)dist_timeout);
				LOG_INFO("Current position %8.06f %9.06f, distance from base: %.0fm; limit %ldm, starting %lds timeout\n",
					lat / 1e6, lon / 1e6, curr_dist, (long)dist_max_meters, (long)dist_timeout);
				dist_ok = false;
				dist_ko_time = now;
				logged = false;
			}
			else if (now - dist_ko_time > ms_to_ticks(dist_timeout * 1000))
			{
				if (!logged)
				{
					radio_printf("CUTOFF:distance timeout\n");
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


static bool cutoff_checkAltitude(int32_t curr_alt, ticks_t now)
{
	static ticks_t alt_ko_time;

	if (status_currStatus() != BSM2_GROUND_WAIT
		&& status_currStatus() != BSM2_NOFIX)
	{
		alt_max = MAX(alt_max, curr_alt);

		if (alt_max - curr_alt > delta_altitude)
		{
			static bool logged = false;

			if (alt_ok)
			{
				radio_printf("CUTOFF:burst detected\n");
				radio_printf("CUTOFF in %lds\n", (long)altitude_timeout);
				LOG_INFO("Current altitude %ld, peak altitude %ld; current altitude lower than delta, starting %ld s timeout\n",
					(long)curr_alt, (long)alt_max, (long)altitude_timeout);
				alt_ok = false;
				logged = false;
				alt_ko_time = now;
			}
			else if (now - alt_ko_time > ms_to_ticks(altitude_timeout * 1000))
			{
				if (!logged)
				{
					radio_printf("CUTOFF:burst timeout\n");
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

static bool cutoff_checkTime(ticks_t now)
{
	static bool logged = false;
	static bool warn_logged = false;

	if (now - status_missionStartTicks() > ms_to_ticks(mission_timeout * 1000))
	{
		if (!logged)
		{
			radio_printf("CUTOFF:mission timeout\n");
			logged = true;
		}
		return false;
	}
	else if (now - status_missionStartTicks() > ms_to_ticks(mission_timeout * 1000) - ms_to_ticks(5*60*1000))
	{
		if (!warn_logged)
		{
			radio_printf("CUTOFF:mission end in %ds\n", mission_timeout - ticks_to_ms(now - status_missionStartTicks()) / 1000);
			warn_logged = true;
		}
		return true;
	}
	else
	{
		logged = false;
		warn_logged = false;
		return true;
	}
}

void cutoff_test_cut(bool on)
{
	if (testmode())
	{
		if (on)
		{
			for (int i = 0; i < N_CUTOFF; i++)
				CUTOFF_ENABLE(i, true);
		}
		else
		{
			for (int i = 0; i < N_CUTOFF; i++)
				CUTOFF_ENABLE(i, false);
		}
	}
}

static bool cutoff_procedure(long code)
{
	if (code == 0xdead)
	{
		#if !(ARCH & ARCH_UNITTEST)
			radio_printf("---CUTOFF ACTIVATED---\n");
			for (int i = 0; i < 3; i++)
			{
				for (int c = 0; c < N_CUTOFF; c++)
				{
					radio_printf("CUTOFF%d pulse %d\n", c+1, i+1);
					CUTOFF_ENABLE(c, true);
					timer_delay(10000);
					CUTOFF_ENABLE(c, false);
					radio_printf("CUTOFF%d pulse done\n", c+1);
					timer_delay(5000);
				}
			}
			radio_printf("Cutoff completed.\n");
		#else
			LOG_INFO("---CUTOFF ACTIVATED---\n");
		#endif

		return true;
	}
	else
	{
		LOG_INFO("Wrong cutoff code: %lx\n", code);
		return false;
	}
}

static bool cut = false;
static void cutoff_cut(void)
{
		if (!cut && !testmode())
		{
			cut = true;
			cutoff_procedure(0xdead);
		}
}

#define PAUSE_TIME (5*60*1000)
static ticks_t cutoff_pause_time = 0;

bool cutoff_check(ticks_t now, int32_t curr_alt, udegree_t lat, udegree_t lon)
{
	bool cutoff =(!cutoff_checkTime(now) ||
		!cutoff_checkDist(lat, lon, now) ||
		!cutoff_checkAltitude(curr_alt, now) ||
		!cutoff_checkMaxalt(curr_alt, now));

	static bool logged = false;
	if (cutoff)
	{
		if (cutoff_pause_time && timer_clock() - cutoff_pause_time < ms_to_ticks(PAUSE_TIME))
		{
			if (!logged)
			{
				radio_printf("CUTOFF pending!\n");
				logged = true;
			}
		}
		else
		{
			cutoff_pause_time = 0;
			logged = false;
			cutoff_cut();
		}
	}
	else
	{
		if (logged)
			radio_printf("Pending CUTOFF cancelled\n");
		logged = false;
	}

	return cutoff;
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

		cutoff_check(now, curr_alt, lat, lon);
	}
}

void cutoff_reset(void)
{
	LOG_INFO("Resetting cutoff procedure\n");
	alt_reset();
	dist_ok = true;
	maxalt_ok = true;
	cutoff_pause_time = 0;

	cut = false;
}

/*
 * Reset the cutoff state machine:
 *  - Burst detector data
 *  - Burst detector timeout
 *  - Max distance timeout
 *  - Max altitude timeout
 *  - Cutoff is resumed if paused
 * NOTE: mission timeout is NOT reset! (use mission_start to reset all)
 */
static bool cmd_cutoff_reset(long l)
{
	(void)l;
	cutoff_reset();
	return true;
}

/*
 * Pause the cutoff for PAUSE_TIME.
 *
 * Useful if you have to take critical decisions and the cutoff is about to
 * activate.
 * The cutoff state machine will be still active, but no wire is cut for the following
 * PAUSE_TIME milliseconds after this command is issued.
 * After this time, if a cutoff condition is still present, the cutoff procedure
 * will be executed.
 *
 * NOTE: you can call cutoff_pause repeatedly, each time the timeout is reset or
 *  you can call cutoff_resume to cancel the pause in advance.
 */
static bool cmd_cutoff_pause(long l)
{
	(void)l;
	cutoff_pause_time = timer_clock();
	return true;
}

/*
 * Call cutoff_resume to cancel a pause in advance.
 * If there is no pause this function will have no effect.
 */
static bool cmd_cutoff_resume(long l)
{
	(void)l;
	cutoff_pause_time = 0;
	return true;
}

void cutoff_init(void)
{
	CUTOFF_INIT();
	config_register(&cutoff);
	config_load(&cutoff);
	uplink_registerCmd("cutoff", cutoff_procedure);
	uplink_registerCmd("cutoff_reset", cmd_cutoff_reset);
	uplink_registerCmd("cutoff_pause", cmd_cutoff_pause);
	uplink_registerCmd("cutoff_resume", cmd_cutoff_resume);

	#if !(ARCH & ARCH_UNITTEST)
		//start process
		LOG_INFO("Starting cutoff process\n");
		Process *p = proc_new(cutoff_process, NULL, KERN_MINSTACKSIZE * 6, NULL);
		ASSERT(p);
	#endif
}
