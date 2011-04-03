// Emtpy main.c file generated by the wizard

#include "landing.h"
#include "cutoff.h"
#include "gps.h"
#include "adc_mgr.h"
#include "sensors.h"
#include "logging.h"

#include "hw/hw_pin.h"
#include <cpu/irq.h>
#include <cfg/debug.h>

#include <kern/proc.h>
#include <kern/monitor.h>

#include <drv/timer.h>
#include <drv/buzzer.h>
#include <drv/ser.h>
#include <drv/kbd.h>
#include <drv/sd.h>
#include <drv/spi_dma_at91.h>

#include <mware/ini_reader.h>

#include <fs/fat.h>

#include <net/afsk.h>

#include <net/nmea.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

static Afsk afsk;
static Serial ser;
static SpiDmaAt91 spi_dma;
static Sd sd;
static FATFS fs;

INLINE void ledr(bool val)
{
	if (val)
		PIOA_SODR = LEDR;
	else
		PIOA_CODR = LEDR;
}

INLINE void ledg(bool val)
{
	if (val)
		PIOA_SODR = LEDG;
	else
		PIOA_CODR = LEDG;
}

static void init(void)
{
	IRQ_ENABLE;
	kdbg_init();
	timer_init();
	buz_init();
	proc_init();
	afsk_init(&afsk, 4, 2);
	ser_init(&ser, SER_UART0);
	ser_setbaudrate(&ser, 4800);
	spi_dma_init(&spi_dma);
	spi_dma_setclock(20000000L);
	gps_init(&ser.fd);
	kbd_init();

	PIOA_CODR = LEDR | LEDG;
	PIOA_PER = LEDR | LEDG;
	PIOA_OER = LEDR | LEDG;

	ledr(true);
	ASSERT(sd_init(&sd, &spi_dma.fd, false));
	ASSERT(f_mount(0, &fs) == FR_OK);
	FatFile conf;
	ASSERT(fatfile_open(&conf, "conf.ini", FA_OPEN_EXISTING | FA_READ) == FR_OK);

	char inibuf[64];

	ini_getString(&conf.fd, "cutoff", "mission_time", "8400", inibuf, sizeof(inibuf));
	uint32_t max_seconds = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "delta_press", "100", inibuf, sizeof(inibuf));
	float delta_press = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "delta_timeout", "60", inibuf, sizeof(inibuf));
	uint32_t delta_timeout = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "base_lat", "43606414", inibuf, sizeof(inibuf));
	udegree_t base_lat = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "base_lon", "11311832", inibuf, sizeof(inibuf));
	udegree_t base_lon = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "max_dist", "80000", inibuf, sizeof(inibuf));
	uint32_t max_meters = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "dist_timeout", "300", inibuf, sizeof(inibuf));
	uint32_t maxdist_timeout = atoi(inibuf);
	
	cutoff_init(max_seconds, delta_press, delta_timeout, base_lat, base_lon, max_meters, maxdist_timeout);

	ini_getString(&conf.fd, "landing", "landing_alt", "3600", inibuf, sizeof(inibuf));
	int32_t landing_meters = atoi(inibuf);
	ini_getString(&conf.fd, "landing", "count_limit", "20", inibuf, sizeof(inibuf));
	int count_limit = atoi(inibuf);
	ini_getString(&conf.fd, "landing", "buz_timeout", "9000", inibuf, sizeof(inibuf));
	uint32_t buz_timeout_seconds = atoi(inibuf);

	landing_init(landing_meters, count_limit, buz_timeout_seconds);
	kfile_close(&conf.fd);

	logging_init();
	ledr(false);
}


#define SHORT_DELAY 5000L
#define LONG_DELAY  120000L
#define START_DELAY ms_to_ticks(120000L)


int main(void)
{
	init();
	char buf[256];
	udegree_t lat, lon;
	int32_t altitude;
	struct tm *t;
	time_t tim;
	bool led_on = true;
	int cnt = 0;


	while (1)
	{
		timer_delay(500);

		bool fix = gps_fixed();

		if (fix && led_on)
			ledg(true);
		else if (!fix && led_on)
			ledr(true);
		else
		{
			ledg(false);
			ledr(false);
		}

		led_on = !led_on;

		if (kbd_peek() & K_START)
		{
			landing_reset();
			cutoff_reset();
			ledg(true);
			ledr(true);
			logging_rotate();
		}

		if (++cnt < 6)
			continue;

		cnt = 0;

		if (fix)
		{
			lat = gps_info()->latitude;
			lon = gps_info()->longitude;
			altitude = gps_info()->altitude;
		}
		else
		{
			lat = 0;
			lon = 0;
			altitude = 0;
		}

		tim = gps_time();
		t = gmtime(&tim);

		snprintf(buf, sizeof(buf), "%02d:%02d:%02d;%s;%02ld.%.06ld;%03ld.%.06ld;%ld;%.1f;%.1f;%.0f;%.2f\n",
		t->tm_hour, t->tm_min, t->tm_sec,
		fix ? "FIX" : "NOFIX",
		lat/1000000, ABS(lat)%1000000, lon/1000000, ABS(lon)%1000000, altitude,
		sensor_temp(INT_TEMP), sensor_temp(EXT_TEMP), sensor_press(), sensor_supply());
		kprintf("%s", buf);
		logging_data("%s", buf);
	}

	return 0;
}

