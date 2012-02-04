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

#include <mware/config.h>

#define LOG_LEVEL LOG_LVL_ERR
#include <cfg/log.h>

typedef struct SensorCalibrationPoint
{
	int x;
	float y;
} SensorCalibrationPoint;

typedef struct SensorCalibrationSet
{
	SensorCalibrationPoint p1, p2;
} SensorCalibrationSet;

/**
 * Default calibration for all analog channels
 */
static SensorCalibrationSet sens_cal[ADC_CHANNELS];

DECLARE_CONF(sensor_cal, NULL,
	//ADC_VIN V
	CONF_INT_NODECLARE(  vin_p1x, sens_cal[0].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(vin_p1y, sens_cal[0].p1.y, 0, 20, 0),
	CONF_INT_NODECLARE(  vin_p2x, sens_cal[0].p2.x, 0, 1023, 1023),
	CONF_FLOAT_NODECLARE(vin_p2y, sens_cal[0].p2.y, 0, 20, 12.64),

	//ADC_5V V
	CONF_INT_NODECLARE(  5v_p1x, sens_cal[1].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(5v_p1y, sens_cal[1].p1.y, 0, 10, 0),
	CONF_INT_NODECLARE(  5v_p2x, sens_cal[1].p2.x, 0, 1023, 1023),
	CONF_FLOAT_NODECLARE(5v_p2y, sens_cal[1].p2.y, 0, 10, 6.3),

	//ADC_3V3 V
	CONF_INT_NODECLARE(  3v3_p1x, sens_cal[2].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(3v3_p1y, sens_cal[2].p1.y, 0, 5, 0),
	CONF_INT_NODECLARE(  3v3_p2x, sens_cal[2].p2.x, 0, 1023, 1023),
	CONF_FLOAT_NODECLARE(3v3_p2y, sens_cal[2].p2.y, 0, 5, 3.523),

	//ADC_CURR mA
	CONF_INT_NODECLARE(  curr_p1x, sens_cal[3].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(curr_p1y, sens_cal[3].p1.y, 0, 5000, 0),
	CONF_INT_NODECLARE(  curr_p2x, sens_cal[3].p2.x, 0, 1023, 1023),
	CONF_FLOAT_NODECLARE(curr_p2y, sens_cal[3].p2.y, 0, 5000, 2000),

	//ADC_PRESS mBar
	CONF_INT_NODECLARE(  press_p1x, sens_cal[4].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(press_p1y, sens_cal[4].p1.y, 0, 2000, 0),
	CONF_INT_NODECLARE(  press_p2x, sens_cal[4].p2.x, 0, 1023, 1023),
	CONF_FLOAT_NODECLARE(press_p2y, sens_cal[4].p2.y, 0, 2000, 1126.0),

	//ADC_T1 °C
	CONF_INT_NODECLARE(  t1_p1x, sens_cal[5].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(t1_p1y, sens_cal[5].p1.y, -200, +200, -105.13),
	CONF_INT_NODECLARE(  t1_p2x, sens_cal[5].p2.x, 0, 1023, 1023),
	CONF_FLOAT_NODECLARE(t1_p2y, sens_cal[5].p2.y, -200, +200, +49.27),

	//ADC_T2 °C
	CONF_INT_NODECLARE(  t2_p1x, sens_cal[6].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(t2_p1y, sens_cal[6].p1.y, -200, +200, -105.13),
	CONF_INT_NODECLARE(  t2_p2x, sens_cal[6].p2.x, 0, 1023, 1023),
	CONF_FLOAT_NODECLARE(t2_p2y, sens_cal[6].p2.y, -200, +200, +49.27),

	//ADC_HUMIDITY %
	CONF_INT_NODECLARE(  hum_p1x, sens_cal[7].p1.x, 0, 1023, 0),
	CONF_FLOAT_NODECLARE(hum_p1y, sens_cal[7].p1.y, 0, 120, 0),
	CONF_INT_NODECLARE(  hum_p2x, sens_cal[7].p2.x, 0, 1023, 853),
	CONF_FLOAT_NODECLARE(hum_p2y, sens_cal[7].p2.y, 0, 120, 92)
);

float sensor_read(AdcChannels ch)
{
	int x = adc_mgr_read(ch);

	int   x1 = sens_cal[ch].p1.x;
	int   x2 = sens_cal[ch].p2.x;
	float y1 = sens_cal[ch].p1.y;
	float y2 = sens_cal[ch].p2.y;
	float y = (((x - x1) * (y2 - y1)) / (x2 - x1)) + y1;
	LOG_INFO("Reading ch%d, x %d, x1 %d, x2 %d, y1 %f, y2 %f, y=%f\n", ch, x, x1, x2, y1, y2, y);
	return y;
}

void sensor_init(void)
{
	config_register(&sensor_cal);
	config_load(&sensor_cal);
}
