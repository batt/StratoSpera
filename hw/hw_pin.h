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
 * Copyright 2003, 2004, 2005, 2006, 2008 Develer S.r.l. (http://www.develer.com/)
 * Copyright 2000 Bernie Innocenti
 * All Rights Reserved.
 * -->
 *
 * \brief I/O pin hardware-specific definitions
 *
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#ifndef HW_PIN_H
#define HW_PIN_H

#ifdef DEMO_BOARD
	#define LAND_PIN     BV(3)
	#define CUTOFF_PIN   BV(4)

	#define LEDR         BV(6)
	#define LEDG         BV(7)
	#define BUZZER_BIT   BV(8)

	#define START_BTN    BV(15)
#else
	#define LAND_PIN     BV(0)
	#define CUTOFF_PIN   BV(1)

	#define LEDR         BV(19)
	#define LEDG         BV(18)
	#define BUZZER_BIT   BV(7)

	#define START_BTN    BV(16)
#endif

#endif /* HW_BUZZER_H */
