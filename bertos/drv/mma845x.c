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

#define OUT_X_MSB 0x01

#define CTRL_REG1 0x2A

#define DATARATE_SHIFT 3
#define ACTIVE_BIT BV(0)

#define CTRL_REG2 0x2B
#define HIGH_RES_MODE 0x02

#define CTRL_REG3 0x2C
#define CTRL_REG4 0x2D
#define CTRL_REG5 0x2E

#define XYZ_DATA_CFG 0x0E


static bool mma845x_writeReg(I2c *i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
	dev_addr &= 1;
	dev_addr = (dev_addr | MMA845x_DEV_ADDR) << 1;

	i2c_start_w(i2c, dev_addr, 2, I2C_STOP);
	i2c_putc(i2c, reg_addr);
	i2c_putc(i2c, data);
	return (i2c_error(i2c) == 0);
}

static int mma845x_readReg(I2c *i2c, uint8_t dev_addr, uint8_t reg_addr)
{
	uint8_t data;
	dev_addr &= 1;
	dev_addr = (dev_addr | MMA845x_DEV_ADDR) << 1;

	i2c_start_w(i2c, dev_addr, 1, I2C_NOSTOP);
	i2c_putc(i2c, reg_addr);
	i2c_start_r(i2c, dev_addr, 1, I2C_STOP);
	i2c_read(i2c, &data, sizeof(data));

	if (i2c_error(i2c))
		return EOF;
	else
		return (int)(uint8_t)data;
}

bool mma845x_enable(I2c *i2c, uint8_t addr, bool state)
{
	int ctrl_reg1 = mma845x_readReg(i2c, addr, CTRL_REG1);
	if (ctrl_reg1 == EOF)
		return false;

	ctrl_reg1 &= ~ACTIVE_BIT;
	ctrl_reg1 |= state ? ACTIVE_BIT : 0;
	return mma845x_writeReg(i2c, addr, CTRL_REG1, ctrl_reg1);
}

int mma845x_read(I2c *i2c, uint8_t addr, Mma845xAxis axis)
{
	ASSERT(axis < MMA_AXIZ_CNT);

	int msb = mma845x_readReg(i2c, addr, OUT_X_MSB + axis * 2);
	if (msb == EOF)
		goto error;

	int lsb = mma845x_readReg(i2c, addr, OUT_X_MSB + axis * 2 + 1);
	if (lsb == EOF)
		goto error;

	return (((int8_t)msb) << 2 | lsb >> 6);

error:
	return MMA_ERROR;
}


bool mma845x_init(I2c *i2c, uint8_t addr, Mma845xDynamicRange dyn_range)
{
	ASSERT(dyn_range < MMADYN_CNT);

	if (!mma845x_writeReg(i2c, addr, XYZ_DATA_CFG, dyn_range))
		return false;
	/*
	 * Using a different data rate and/or mode other than the default one
	 * causes a -3% sentitivity error:
	 * http://www.freescale.com/files/sensors/doc/data_sheet/MMA8453Q.pdf
	 * page 6, small foot note #2.
	 *
	 * So we disable low sample rate and high res mode for now.
	 */
	#if 0
		if (!mma845x_writeReg(i2c, addr, CTRL_REG1, MMADR_1_56HZ << DATARATE_SHIFT))
			return false;

		if (!mma845x_writeReg(i2c, addr, CTRL_REG2, HIGH_RES_MODE))
			return false;
	#endif
	return mma845x_enable(i2c, addr, true);
}
