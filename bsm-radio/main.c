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

#include "hw/hw_cc1101.h"
#include "hw/hw_spi.h"

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



static void init(void)
{
	IRQ_ENABLE;
	kdbg_init();
	timer_init();
	spi_init();

	stm32_gpioPinConfig(GPIO_BASE, BV(11), GPIO_MODE_IN_FLOATING, GPIO_SPEED_50MHZ);

	timer_delay(1);

	cc1101_powerOnReset();
	//kputs("cc1101 Reset!\n");
	cc1101_setup(ping_low_baud_868);
//	kputs("cc1101 started!\n");
}
uint8_t tmp[64];
int main(void)
{
	init();
	uint8_t status = 0;
	while (1)
	{

		#if 1
		status = cc1101_strobe(CC1101_SNOP);
		kprintf("N %d %d %d\n", UNPACK_STATUS(status));

		status = cc1101_write(CC1101_TXFIFO, 'c');
		status = cc1101_write(CC1101_TXFIFO, 'a');
		status = cc1101_write(CC1101_TXFIFO, 'f');
		status = cc1101_write(CC1101_TXFIFO, 'e');
		kprintf("T %d %d %d\n", UNPACK_STATUS(status));

		status = cc1101_strobe(CC1101_STX);
		kprintf("T %d %d %d\n", UNPACK_STATUS(status));

		status = cc1101_strobe(CC1101_SNOP);
		kprintf("N %d %d %d\n", UNPACK_STATUS(status));
		#else

		status = cc1101_strobe(CC1101_SNOP);
		kprintf("N %d %d %d\n", UNPACK_STATUS(status));

		status = cc1101_strobe(CC1101_SFRX);
		kprintf("F %d %d %d\n", UNPACK_STATUS(status));

		status = cc1101_strobe(CC1101_SRX);
		kprintf("R %d %d %d\n", UNPACK_STATUS(status));
		while (!stm32_gpioPinRead(GPIO_BASE, BV(11)))
			cpu_relax();

		uint8_t len = cc1101_read(CC1101_RXBYTES) & 0x7F;
		kprintf("len[%d] in rx_fifo\n", len);

		cc1101_readBurst(CC1101_RXFIFO, tmp, len);

		kputs("[ ");
		for (int i = 0; i < len; i++)
			kprintf("%x ", tmp[i]);
		kputs(" ]\n");
		kputs("[ ");
		for (int i = 0; i < len; i++)
			kprintf("%c ", tmp[i]);
		kputs(" ]\n");

		uint8_t rssi_dec = cc1101_read(CC1101_RSSI);
		int16_t rssi_dBm;
		uint8_t rssi_offset = 74; // CC1101 at 868/900 MHz

		if (rssi_dec >= 128)
			rssi_dBm = (int16_t)((int16_t)( rssi_dec - 256) / 2) - rssi_offset;
		else
			rssi_dBm = (rssi_dec / 2) - rssi_offset;

		kprintf("RSSI[%d]dBm\n",rssi_dBm);

		/*
		while (1)
		{
			if (stm32_gpioPinRead(GPIO_BASE, BV(11)))
			{
				while (stm32_gpioPinRead(GPIO_BASE, BV(11)))
					cpu_relax();

				break;
			}
			cpu_relax();
		}
		*/
		#endif
		timer_delay(500);

	}

	return 0;
}

