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

#define CTRL_REG1 0x2A
#define CTRL_REG2 0x2B
#define CTRL_REG3 0x2C
#define CTRL_REG4 0x2D
#define CTRL_REG5 0x2E


static bool mma845x_writeReg(I2c *i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
	dev_addr &= 1;
	dev_addr = (dev_addr | MMA845x_DEV_ADDR) << 1;

	i2c_start_w(i2c, dev_addr, 2, I2C_STOP);
	i2c_putc(i2c, reg_addr);
	i2c_putc(i2c, data);
	return !i2c_error(i2c);
}

static int mma845x_readReg(I2c *i2c, uint8_t dev_addr, uint8_t reg_addr)
{
	uint8_t data;
	dev_addr &= 1;
	dev_addr = (dev_addr | MMA845x_DEV_ADDR) << 1;

	i2c_start_w(i2c, dev_addr, 1, I2C_STOP);
	i2c_putc(i2c, reg_addr);
	i2c_start_r(i2c, dev_addr, 1, I2C_STOP);
	i2c_read(i2c, &data, sizeof(data));

	if (i2c_error(i2c))
		return EOF;
	else
		return (int)(uint8_t)data;
}

#define ACTIVE_BIT BV(0)

bool mma845x_enable(I2c *i2c, uint8_t addr, bool state)
{
	int ctrl_reg1 = mma845x_readReg(i2c, addr, CTRL_REG1);
	if (ctrl_reg1 == EOF)
		return false;

	ctrl_reg1 &= ~ACTIVE_BIT;
	ctrl_reg1 |= state ? ACTIVE_BIT : 0;
	return mma845x_writeReg(i2c, addr, CTRL_REG1, ctrl_reg1);
}

#define DATARATE_SHIFT 3
#define DATARATE_MASK (BV(DATARATE_SHIFT) | BV(DATARATE_SHIFT+1) | BV(DATARATE_SHIFT+2))

bool mma845x_datarate(I2c *i2c, uint8_t addr, Mma845xDataRate rate)
{
	ASSERT(rate < MMADR_CNT);

	int ctrl_reg1 = mma845x_readReg(i2c, addr, CTRL_REG1);
	if (ctrl_reg1 == EOF)
		return false;

	ctrl_reg1 &= ~DATARATE_MASK;
	ctrl_reg1 |= rate << DATARATE_SHIFT;
	return mma845x_writeReg(i2c, addr, CTRL_REG1, ctrl_reg1);
}

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
		int val = (((int8_t)data[i * 2]) << 2 | data[i * 2 + 1] >> 6);
		acc[i] = val * 2 * 9.81 / 512;
	}

	return true;
}

bool mma845x_init(I2c *i2c, uint8_t addr)
{

}
