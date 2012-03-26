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

#ifndef CUSTOM_CUSTOM_H
#define CUSTOM_CUSTOM_H

#include <drv/ser.h>
#include <drv/timer.h>
#include <kern/proc.h>

enum
{
	INPUT,
	OUTPUT,
};

enum
{
	LOW = 0,
	HIGH,
};

#define PIN_MODE(pin, mode) do { \
	PIOA_PER = BV(pin); \
	if (mode == INPUT) \
		PIOA_ODR = BV(pin); \
	else \
		PIOA_OER = BV(pin); \
} while (0)

#define PIN_PULLUP(pin, on) do { \
	if (on) \
		PIOA_PUER = BV(pin); \
	else \
		PIOA_PUDR = BV(pin); \
} while (0)

#define DIGITAL_WRITE(pin, level) do { \
	if (level == HIGH) \
		PIOA_SODR = BV(pin); \
	else \
		PIOA_CODR = BV(pin); \
} while (0)

#define DIGITAL_READ(pin) ((PIOA_PDSR & BV(pin)) ? HIGH : LOW)

void custom_init(void);

#endif
