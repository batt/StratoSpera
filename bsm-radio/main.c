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
#include "hw/hw_adc.h"

#include <cfg/debug.h>

#include <cpu/irq.h>
#include <cpu/types.h>
#include <cpu/power.h>

#include <drv/timer.h>
#include <drv/adc.h>
#include <drv/spi_bitbang.h>

#include <string.h>

struct Beacon
{
	uint32_t count;
	uint32_t code;
	uint16_t temp;
	uint16_t vref;
};
static struct Beacon beacon;

static void init(void)
{
	IRQ_ENABLE;
	kdbg_init();
	timer_init();
	spi_init();
	adc_init();

	cc1101_init(ping_low_baud_868);
}

int main(void)
{
	init();
	int ret;
	int id = radio_id();
	int rssi;

	beacon.code = 0xdbf1;
	beacon.count = 0;

	kprintf("%s [%d]\n", id == RADIO_MASTER ? "MASTER" : "SLAVE", id);
	while (1)
	{

		if (id == RADIO_MASTER)
		{
			if ((ret = radio_recv(&beacon, sizeof(beacon), -1)) > 0)
			{
				uint8_t lqi = radio_lqi();
				if (lqi & BV(7))
					kprintf("%0lx,%ld,%d,%d,%d.%d,%d.%d\n", beacon.code, beacon.count, radio_rssi(), lqi & ~BV(7),
						 beacon.temp / 100, beacon.temp % 100,
						 beacon.vref / 1000, beacon.vref % 1000);
			}

			rssi = 0;
			memset(&beacon, 0, sizeof(struct Beacon));
		}
		else
		{
			beacon.temp = hw_readIntTemp();
			beacon.vref = hw_readVrefint();
			kprintf("%d,%d\n", beacon.vref, beacon.temp);
			radio_send(&beacon, sizeof(beacon));

			radio_sleep();
			beacon.count++;

			timer_delay(5000);
		}
	}

	return 0;
}

