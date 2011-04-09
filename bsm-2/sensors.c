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

#define LOG_LEVEL LOG_LVL_INFO
#include <cfg/log.h>

/**
 * Default calibration for all analog channels
 */
static SensorCalibrationSet sensor_calib[ADC_CHANNELS] =
{
	//ADC_VIN V
	{
		.p1 = { .x = 0, .y = 0 },
		.p2 = { .x = 1023, .y = 12.64 },
	},
	//ADC_5V V
	{
		.p1 = { .x = 0, .y = 0 },
		.p2 = { .x = 1023, .y = 6.3 },
	},
	//ADC_3V3 V
	{
		.p1 = { .x = 0, .y = 0 },
		.p2 = { .x = 1023, .y = 3.523 },
	},
	//ADC_CURR mA
	{
		.p1 = { .x = 0, .y = 0 },
		.p2 = { .x = 1023, .y = 2000.0 },
	},
	//ADC_PRESS mBar
	{
		.p1 = { .x = 0, .y = 0 },
		.p2 = { .x = 1023, .y = 1126.0 },
	},
	//ADC_T1 °C
	{
		.p1 = { .x = 0, .y = -105.13 },
		.p2 = { .x = 1023, .y = +49.27 },
	},
	//ADC_T2 °C
	{
		.p1 = { .x = 0, .y = -105.13 },
		.p2 = { .x = 1023, .y = +49.27 },
	},
	//ADC_HUMIDITY %
	{
		.p1 = { .x = 0, .y = 0 },
		.p2 = { .x = 1023, .y = 100 },
	},
};

float sensor_read(AdcChannels ch)
{
	int x = adc_mgr_read(ch);

	int   x1 = sensor_calib[ch].p1.x;
	int   x2 = sensor_calib[ch].p2.x;
	float y1 = sensor_calib[ch].p1.y;
	float y2 = sensor_calib[ch].p2.y;
	float y = (((x - x1) * (y2 - y1)) / (x2 - x1)) + y1;
	LOG_INFO("Reading ch%d, x %d, x1 %d, x2 %d, y1 %f, y2 %f, y=%f", ch, x, x1, x2, y1, y2, y);
	return y;
}

void sensor_setCalibration(AdcChannels channel, SensorCalibrationSet set)
{
	ASSERT(channel < ADC_CHANNELS);
	sensor_calib[channel] = set;
}
