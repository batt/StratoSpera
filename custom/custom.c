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
 * Copyright 2012 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \brief Custom firmware expansion.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "custom.h"

#define LOG_LEVEL LOG_LVL_INFO
#include <cfg/log.h>

static Serial ser;

#define AUX_OUT 23
#define START_BUTTON 16

static void NORETURN custom_process(void)
{
	int i = 0;
	while (1)
	{
		LOG_INFO("Custom test %d\n", i++);
		if (DIGITAL_READ(START_BUTTON) == HIGH)
			LOG_INFO("START button HIGH\n");
		else
			LOG_INFO("START button LOW\n");

		DIGITAL_WRITE(AUX_OUT, HIGH);
		timer_delay(1000);
		DIGITAL_WRITE(AUX_OUT, LOW);
		timer_delay(1000);
	}
}


void custom_init(void)
{
	ser_init(&ser, SER_UART1);
	ser_setbaudrate(&ser, 9600);
	PIN_MODE(AUX_OUT, OUTPUT);

	Process *p = proc_new(custom_process, NULL, KERN_MINSTACKSIZE * 5, NULL);
	ASSERT2(p, "Error creating custom process!\n");
	LOG_INFO("Custom process initialized\n");
}
