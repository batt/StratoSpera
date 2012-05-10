#include "gps.h"

#include "testmode.h"

#include <cfg/debug.h>

#include <kern/proc.h>

#include <drv/timer.h>
#include <drv/ser.h>

#include <net/nmea.h>

#include <stdio.h>

static Serial ser;
static nmeap_context_t nmea;

char aprs_lat[9];
char aprs_lon[10];
char aprs_time[8];
NmeaGga gps_gga;
bool gps_fix = false;
time_t gps_clock;

static ticks_t last_heard;

static void aprs_gpgga(nmeap_context_t *context, void *data, void *userdata)
{
	(void)data;
	(void)userdata;

	if (testmode())
		kprintf("%.*s\n", NMEAP_MAX_SENTENCE_LENGTH, context->debug_input);

	last_heard = timer_clock();
	sprintf(aprs_time, "%.6sh", context->token[1]);
	sprintf(aprs_lat, "%.7s%c", context->token[2], *context->token[3]);
	sprintf(aprs_lon, "%.8s%c", context->token[4], *context->token[5]);

	if (gps_gga.quality > 0)
	{
		gps_fix = true;
		gps_clock = gps_gga.time;
	}
	else
		gps_fix = false;
}

static void NORETURN time_process(void)
{
	while (1)
	{
		/* Reset watchdog */
		WDT_CR = WDT_KEY | BV(WDT_WDRSTT);
		timer_delay(1000);

		if (gps_fix && (timer_clock() - last_heard > ms_to_ticks(10000)))
		{
			gps_fix = false;
			gps_clock += 9;
		}

		if (!gps_fix)
			gps_clock++;
	}
}


static void NORETURN gps_process(void)
{
	while (1)
		nmea_poll(&nmea, &ser.fd);
}

void gps_init(unsigned port, unsigned long baudrate)
{
	ser_init(&ser, port);
	ser_setbaudrate(&ser, baudrate);

	nmeap_init(&nmea, NULL);

	nmeap_addParser(&nmea, "GPGGA", nmea_gpgga, aprs_gpgga, &gps_gga);
	Process *p1 = proc_new(gps_process, NULL, KERN_MINSTACKSIZE * 3, NULL);
	Process *p2 = proc_new(time_process, NULL, KERN_MINSTACKSIZE * 2, NULL);
	ASSERT(p1);
	ASSERT(p2);
}



