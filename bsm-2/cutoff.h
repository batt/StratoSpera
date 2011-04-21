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

#ifndef CUTOFF_H
#define CUTOFF_H

#include <net/nmea.h>
#include <cfg/compiler.h>
#include <drv/pwm.h>

typedef struct CutoffCfg
{
	uint32_t mission_timeout; //seconds
	int32_t delta_altitude; //meters
	uint32_t altitude_timeout; //seconds
	udegree_t start_latitude; //micro degrees
	udegree_t start_longitude; //micro degrees
	uint32_t dist_max_meters; //meters
	uint32_t dist_timeout; //seconds
	pwm_duty_t pwm_duty;
} CutoffCfg;

void cutoff_reset(void);
bool cutoff_checkDist(udegree_t lat, udegree_t lon, ticks_t now);
bool cutoff_checkAltitude(int32_t curr_alt, ticks_t now);
bool cutoff_checkTime(ticks_t now);
void cutoff_setCfg(CutoffCfg *cfg);
void cutoff_init(CutoffCfg *cfg);
#endif
