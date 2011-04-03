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
 * \brief Analog sensors conditioning.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "sensors.h"
#include "adc_mgr.h"

#include <drv/mpxx6115a.h>
#include <math.h>

#define NTC_A 1.129241E-3
#define NTC_B 2.341077E-4
#define NTC_C 8.775468E-8


float sensor_temp(unsigned idx)
{
	uint16_t val = adc_mgr_read(idx);

	float rntc = (val * 10000.0) / (1023.0 - val);
	float y = NTC_A + NTC_B * logf(rntc) + NTC_C * powf(logf(rntc), 3);
	return 1 / y - 273.15;
}

float sensor_press(void)
{
	return mpxx6115a_press(adc_mgr_read(PRES_CH), 775);
}

float sensor_altitude(void)
{
	float p = sensor_press();
	return 44330.8 - 4946.54 * powf(p * 100, 0.190263);
}

float sensor_supply(void)
{
	uint16_t val = adc_mgr_read(SUPPLY_CH);
	return (val * 3.3 * (1390.0 / 390.0)) / 1023;
}
