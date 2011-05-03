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
 * Copyright 2009 Develer S.r.l. (http://www.develer.com/)
 *
 * -->
 *
 * \brief LM75 sensor temperature family.
 *
 * \author Daniele Basile <asterix@develer.com>
 *
 */

#include "mma845x.h"

#include <cfg/debug.h>

#include <drv/i2c.h>

#define MMA845x_DEV_ADDR 0x1C

bool mma845x_read(I2c *i2c, uint8_t addr, float *acc)
{
	uint8_t data[6];
	addr &= 1;
	addr = (addr | MMA845x_DEV_ADDR) << 1;

	i2c_start_w(i2c, addr, 1, I2C_NOSTOP);
	i2c_putc(i2c, 0x01);
	i2c_start_r(i2c, addr, sizeof(data), I2C_STOP);
	i2c_read(i2c, data, sizeof(data));

	if (i2c_error(i2c))
		return false;

	for (int i = 0; i < 6; i++)
		kprintf("%02X ", data[i]);
	kprintf("\n");

	for (int i = 0; i < 3; i++)
	{
		int val = (data[i * 2] << 2 | data[i * 2 + 1] >> 6);
		acc[i] = val * 2 * 9.81 / 512;
	}

	return true;
}
