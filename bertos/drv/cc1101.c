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
 * \brief Texas radio chip CC1101 interface.
 *
 * \author Daniele Basile <asterix@develer.com>
 */

#include "cc1101.h"

#include "hw/hw_spi.h"

#include <cpu/types.h>
#include <cpu/power.h>

#include <drv/timer.h>
#include <drv/spi_bitbang.h>

#define STATUS_RDY(status)               (((status) & 0x80) >> 7)
#define STATUS_STATE(status)             (((status) & 0x70) >> 4)
#define STATUS_FIFO_AVAIL(status)        ((status) & 0xF)

#define UNPACK_STATUS(status) \
	STATUS_RDY(status), \
	STATUS_STATE(status), \
	STATUS_FIFO_AVAIL(status)

#define WAIT_SO_LOW()  \
	do { \
		while(IS_MISO_HIGH()) \
			cpu_relax(); \
	} while(0)


uint8_t cc1101_read(uint8_t addr)
{

	SS_ACTIVE();
	WAIT_SO_LOW();

    uint8_t x = spi_sendRecv(addr);

	SS_INACTIVE();
    return x;
}

uint8_t cc1101_write(uint8_t addr, uint8_t data)
{
	SS_ACTIVE();
	WAIT_SO_LOW();

    uint8_t x = spi_sendRecv(addr);
    x = spi_sendRecv(data);

	SS_INACTIVE();
    return x;
}

uint8_t cc1101_strobe(uint8_t addr)
{
	SS_ACTIVE();
	WAIT_SO_LOW();

    uint8_t x = spi_sendRecv(addr);

	SS_INACTIVE();
    return x;
}

void cc1101_readBurst(uint8_t addr, uint8_t* buf, size_t len)
{
	SS_ACTIVE();
	WAIT_SO_LOW();

    spi_sendRecv(addr | 0xc0);
	spi_read(buf, len);

	SS_INACTIVE();
}

void cc1101_writeBurst(uint8_t addr, uint8_t* buf, size_t len)
{
	SS_ACTIVE();
	WAIT_SO_LOW();

    spi_sendRecv(addr | 0x40);
	spi_write(buf, len);

	SS_INACTIVE();
}

void cc1101_powerOnReset(void)
{
	STROBE_ON();
	SS_INACTIVE();
	timer_udelay(1);
	STROBE_OFF();

	SS_ACTIVE();
	timer_udelay(1);
	SS_INACTIVE();
	STROBE_ON();

	timer_udelay(41);
	SS_ACTIVE();

	STROBE_OFF();
	WAIT_SO_LOW();

    spi_sendRecv(CC1101_SRES);

	WAIT_SO_LOW();
	timer_udelay(50);

	SS_INACTIVE();
}

void cc1101_setup(const Setting* settings)
{
	uint8_t x = 0;



	do
	{
		SS_ACTIVE();
		WAIT_SO_LOW();

		x = spi_sendRecv(CC1101_SNOP);

		SS_INACTIVE();

		kprintf("%d\n", STATUS_RDY(x));

	} while (STATUS_RDY(x));

	for (int i = 0; settings[i].addr != 0xFF && settings[i].data != 0xFF; i++)
	{

		SS_ACTIVE();
		WAIT_SO_LOW();

	    x = spi_sendRecv(settings[i].addr);
		x = spi_sendRecv(settings[i].data);

		SS_INACTIVE();

		kprintf("setup %d %d %d\n", UNPACK_STATUS(x));
	}
}
