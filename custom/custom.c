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

		/* Write on the serial port */
		uint8_t buf[] = {0, 1, 2, 3, 4, 5};
		kfile_write(&ser.fd, buf, 6);

		/* Read 6 bytes, this function will block until all 6 bytes
		 * have been correctly received or there is a timeout. */
		uint8_t ret[6];
		int n = kfile_read(&ser.fd, ret, 6);

		if (n != 6)
		{
			LOG_INFO("Error receiving from the serial port, error code %d\n", kfile_error(&ser.fd));

			/* Always remember to clear errors! */
			kfile_clearerr(&ser.fd);
		}

		/* Put a single byte on the serial port */
		kfile_putc('a', &ser.fd);

		/* Read a single char from the serial port */
		int c = kfile_getc(&ser.fd);

		/* kfile_getc() returns EOF on errors */
		if (c == EOF)
		{
			LOG_INFO("Error receiving from the serial port, error code %d\n", kfile_error(&ser.fd));

			/* Always remember to clear errors! */
			kfile_clearerr(&ser.fd);
		}

		/* You can also use a printf-like API */
		kfile_printf(&ser.fd, "Writing on the serial port usign printf format: %d\n", 123);
	}
}

#define RXTIMEOUT 1000 // 1 second
#define TXTIMEOUT 1000 // 1 second

void custom_init(void)
{
	ser_init(&ser, SER_UART1);
	ser_setbaudrate(&ser, 9600);
	/* Set timeouts for serial port */
	ser_settimeouts(&ser, RXTIMEOUT, TXTIMEOUT);
	PIN_MODE(AUX_OUT, OUTPUT);

	Process *p = proc_new(custom_process, NULL, KERN_MINSTACKSIZE * 5, NULL);
	ASSERT2(p, "Error creating custom process!\n");
	LOG_INFO("Custom process initialized\n");
}
