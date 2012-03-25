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

#include <drv/timer.h>
#include <drv/spi_bitbang.h>

uint8_t cc1101_read(uint8_t addr)
{

	SS_ACTIVE();
	while(!IS_MISO_HIGH());

    uint8_t x = spi_sendRecv(addr);

	SS_INACTIVE();
    return x;
}

uint8_t cc1101_write(uint8_t addr, uint8_t data)
{
	SS_ACTIVE();
	while(!IS_MISO_HIGH());

    uint8_t x = spi_sendRecv(addr);
    x = spi_sendRecv(data);

	SS_INACTIVE();
    return x;
}

uint8_t cc1101_strobe(uint8_t addr)
{
	SS_ACTIVE();
	while(!IS_MISO_HIGH());

    uint8_t x = spi_sendRecv(addr);

	SS_INACTIVE();
    return x;
}

void cc1101_readBurst(uint8_t addr, uint8_t* buf, size_t len)
{
	SS_ACTIVE();
	while(!IS_MISO_HIGH());

    spi_sendRecv(addr | 0xc0);
	spi_read(buf, len);

	SS_INACTIVE();
}

void cc1101_writeBurst(uint8_t addr, uint8_t* buf, size_t len)
{
	SS_ACTIVE();
	while(!IS_MISO_HIGH());

    spi_sendRecv(addr | 0x40);
	spi_write(buf, len);

	SS_INACTIVE();
}

void cc1101_powerOnReset(void)
{
	SS_INACTIVE();
	timer_udelay(1);
	SS_ACTIVE();
	timer_udelay(1);
	SS_INACTIVE();
	timer_udelay(41);
	SS_ACTIVE();

	while(!IS_MISO_HIGH());
	timer_udelay(50);

    spi_sendRecv(CC1101_SRES);

	while(!IS_MISO_HIGH());
	timer_udelay(50);

	SS_INACTIVE();
}

void cc1101_setup(const Setting* settings)
{
	cc1101_write(settings->addr, settings->data);
}
