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

void cutoff_reset(void);
bool cutoff_checkDist(bool fix, float lat, float lon, ticks_t now);
bool cutoff_checkAltitude(bool fix, int32_t curr_alt, ticks_t now);
bool cutoff_checkTime(ticks_t now);
bool cutoff_active(void);
void cutoff_init(uint32_t max_seconds, int32_t _delta_alt, uint32_t _delta_timeout, udegree_t _start_lat, udegree_t _start_lon, uint32_t max_meters, uint32_t _maxdist_timeout);
#endif
