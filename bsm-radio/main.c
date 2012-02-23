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
 * \brief BSM-RADIO main.
 *
 * \author Daniele Basile <asterix@develer.com>
 */
#include <cfg/debug.h>

#include <cpu/irq.h>
#include <cpu/types.h>
#include <cpu/power.h>

#include <drv/timer.h>
#include <drv/spi_bitbang.h>


#define CC1101_REG_MARCSTATE    0x35

#define CC1101_READ_BIT     0x1
#define CC1101_WRITE_BIT    0x0
#define CC1101_BURTS_BIT    0x2

#define STATUS_RDY(status)               (((status) & 0x80) >> 7)
#define STATUS_STATE(status)             (((status) & 0x70) >> 4)
#define STATUS_FIFO_AVAIL(status)        ((status) & 0xF)

#define UNPACK_STATUS(status) \
	STATUS_RDY(status), \
	STATUS_STATE(status), \
	STATUS_FIFO_AVAIL(status)


static void cc1101_init(void)
{
	uint8_t data = 0x3; //CC1101_READ_BIT | CC1101_BURTS_BIT | CC1101_REG_MARCSTATE;
	while(STATUS_RDY(spi_sendRecv(data)))
		cpu_relax();
}

static void init(void)
{
	IRQ_ENABLE;
	kdbg_init();
	timer_init();
	spi_init();

	cc1101_init();
}

int main(void)
{
	init();
	while (1)
	{
		uint8_t data = 0xF5; //CC1101_READ_BIT | CC1101_BURTS_BIT | CC1101_REG_MARCSTATE;
		uint8_t recv = spi_sendRecv(data);
		kprintf("Sent %0x, state %x\n", data, recv & 0x1F);
		timer_delay(500);
	}

	return 0;
}

