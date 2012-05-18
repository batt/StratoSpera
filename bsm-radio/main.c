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

static void init(void)
{
	IRQ_ENABLE;
	kdbg_init();
	timer_init();
	spi_init();

	timer_delay(1);

	cc1101_init(ping_low_baud_868);
	kputs("cc1101 started!\n");
}
uint8_t tmp[64];
int main(void)
{
	init();

	uint32_t ping = 0;
	uint32_t prev_ping = 0;
	int ret;
	int id = radio_id();
	kprintf("%s [%d]\n", id == RADIO_MASTER ? "MASTER" : "SLAVE", id);
	while (1)
	{

		if (id == RADIO_MASTER)
		{
			radio_send(&ping, sizeof(ping));
			kprintf("Ping tx %ld\n", ping);

			if ((ret = radio_recv(&ping, sizeof(ping), 1000)) < 0)
			{
				kprintf("Err whire recv[%d]..\n", ret);
				continue;
			}

			kprintf("Ping rx %ld\n", ping);

			timer_delay(60000);
		}
		else
		{
			if ((ret = radio_recv(&ping, sizeof(ping), -1)) < 0)
			{
				kprintf("Err whire recv[%d]..\n", ret);
				continue;
			}
			
			kprintf("Ping rx %ld\n", ping);
			ping++;

			radio_send(&ping, sizeof(ping));
			kprintf("Ping tx %ld\n", ping);
			prev_ping = ping;
		}

	}

	return 0;
}

