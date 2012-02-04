// Emtpy main.c file generated by the wizard

#include "landing_buz.h"
#include "cutoff.h"
#include "gps.h"
#include "adc_mgr.h"
#include "sensors.h"
#include "logging.h"
#include "measures.h"
#include "hadarp.h"
#include "status_mgr.h"
#include "radio.h"
#include "testmode.h"

#include "hw/hw_pin.h"
#include "hw/hw_led.h"
#include "hw/hw_aux.h"

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
#include <mware/config.h>

#include <fs/fat.h>

#include <net/nmea.h>

#include <verstag.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_LEVEL LOG_LVL_INFO
#include <cfg/log.h>

static SpiDmaAt91 spi_dma;
static Sd sd;
static FATFS fs;

static ticks_t log_interval;

static void init(void)
{
	IRQ_ENABLE;
	led_init();
	aux_init();
	kdbg_init();
	kprintf("BSM-2, ver %s\n", vers_tag);
	timer_init();
	buz_init();
	proc_init();

	#if GPS_ENABLED
		gps_init(GPS_PORT, 4800);
	#else
		#warning "GPS process disabled."
	#endif

	#if HADARP_ENABLED
		hadarp_init(HADARP_PORT, 9600);
	#else
		#warning "HADARP process disabled."
	#endif

	spi_dma_init(&spi_dma);
	spi_dma_setclock(20000000L);
	kbd_init();
	measures_init();

	ASSERT(sd_init(&sd, &spi_dma.fd, false));
	ASSERT(f_mount(0, &fs) == FR_OK);
	FatFile conf;
	ASSERT(fatfile_open(&conf, "conf.ini", FA_OPEN_EXISTING | FA_READ) == FR_OK);
	logging_init();
	config_init(&conf.fd);

	char inibuf[64];

	sensor_init();
	LOG_INFO("Sensor calibration loaded\n");
	status_init();
	cutoff_init();

	ini_getString(&conf.fd, "landing_buz", "buz_timeout", "9000", inibuf, sizeof(inibuf));
	uint32_t buz_timeout_seconds = atoi(inibuf);

	landing_buz_init(buz_timeout_seconds);

	RadioCfg radio_cfg;
	ini_getString(&conf.fd, "logging", "aprs_interval", "60", inibuf, sizeof(inibuf));
	radio_cfg.aprs_interval = atoi(inibuf);
	ini_getString(&conf.fd, "logging", "send_call", "STSP3", inibuf, sizeof(inibuf));
	strncpy(radio_cfg.send_call, inibuf, sizeof(radio_cfg.send_call));
	radio_cfg.send_call[sizeof(radio_cfg.send_call) - 1] = '\0';
	radio_init(&radio_cfg);

	ini_getString(&conf.fd, "logging", "log_interval", "3", inibuf, sizeof(inibuf));
	log_interval = ms_to_ticks(atoi(inibuf) * 1000);

	ini_getString(&conf.fd, "system", "test_mode", "0", inibuf, sizeof(inibuf));
	kfile_close(&conf.fd);

	if (atoi(inibuf) != 0)
			testmode_run();

	ledr(false);
}

int main(void)
{
	init();
	bool led_on = true;

	ticks_t log_start = timer_clock();

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
			ledg(true);
			ledr(true);
			logging_rotate();
			status_missionStart();
		}

		if (timer_clock() - log_start > log_interval)
		{
			char msg[128];
			log_start += log_interval;
			//monitor_report();

			measures_logFormat(msg, sizeof(msg));
			kprintf("%s\n", msg);
			logging_data("%s\n", msg);
		}
	}
	return 0;
}
