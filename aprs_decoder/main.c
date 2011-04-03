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
 * Copyright 2010 Develer S.r.l. (http://www.develer.com/)
 *
 * -->
 *
 * \author Andrea Righi <arighi@develer.com>
 *
 * \brief Empty project.
 *
 * This is a minimalist project, it just initializes the hardware of the
 * supported board and proposes an empty main.
 */


#include <net/afsk.h>
#include <net/ax25.h>

#include <cfg/debug.h>
#include <cfg/kfile_debug.h>

#include <cpu/irq.h>
#include <cpu/power.h>

#include <drv/timer.h>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

#include <stdio.h>
#include <stdlib.h>


/* The sample type to use */
static const pa_sample_spec ss = {
	.format = PA_SAMPLE_U8,
	.rate = 9600,
	.channels = 1
};
static pa_simple *s = NULL;

static Afsk afsk_fd;
static AX25Ctx ax25;

static void message_hook(struct AX25Msg *msg)
{
	fprintf(stdout, "AFSK1200: fm %.6s-%d\n", msg->src.call, msg->src.ssid);
	fprintf(stdout, "%.*s\n", (int)msg->len, msg->info);
	fflush(stdout);
}


static void init(void)
{
	/* Initialize debugging module (allow kprintf(), etc.) */
	kdbg_init();
	/* Initialize system timer */
	timer_init();
	int error;

    /* Create the recording stream */
    if (!(s = pa_simple_new(NULL, "aprs_decoder", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        exit(-1);
    }
	afsk_init(&afsk_fd, 0 ,0);
	ax25_init(&ax25, &afsk_fd.fd, message_hook);
}
uint8_t buf[1024];

int main(void)
{
	init();

	int error;
	while (1)
	{
		/* Record some data ... */
		if (pa_simple_read(s, buf, sizeof(buf), &error) < 0) {
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			return -1;
		}

		for (unsigned i = 0; i < sizeof(buf); i++)
			afsk_adc_isr(&afsk_fd, (int8_t)(buf[i] - 128));

		ax25_poll(&ax25);
	}
}
