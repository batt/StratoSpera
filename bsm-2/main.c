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

	char inibuf[64];

	/* Set ADC sensor calibration */
	for (int i = 0; i < ADC_CHANNELS; i++)
	{
		char calib[16];
		SensorCalibrationSet set;

		snprintf(calib, sizeof(calib), "calib%02d", i);
		calib[sizeof(calib) - 1] = '\0';

		if (ini_getString(&conf.fd, calib, "p1x", "0", inibuf, sizeof(inibuf)) != 0)
			continue;
		set.p1.x = atoi(inibuf);

		if (ini_getString(&conf.fd, calib, "p1y", "0", inibuf, sizeof(inibuf)) != 0)
			continue;
		set.p1.y = atoi(inibuf) / 1000.0;

		if (ini_getString(&conf.fd, calib, "p2x", "1023", inibuf, sizeof(inibuf)) != 0)
			continue;
		set.p2.x = atoi(inibuf);

		if (ini_getString(&conf.fd, calib, "p2y", "1023", inibuf, sizeof(inibuf)) != 0)
			continue;
		set.p2.y = atoi(inibuf) / 1000.0;

		LOG_INFO("Calibration loaded for channel %d\n", i);
		sensor_setCalibration(i, set);
	}

	StatusCfg status_cfg;
	ini_getString(&conf.fd, "status", "ground_alt", "1500", inibuf, sizeof(inibuf));
	status_cfg.ground_alt = atoi(inibuf);
	ini_getString(&conf.fd, "status", "landing_alt", "3600", inibuf, sizeof(inibuf));
	status_cfg.landing_alt = atoi(inibuf);
	ini_getString(&conf.fd, "status", "tropopause_alt", "12500", inibuf, sizeof(inibuf));
	status_cfg.tropopause_alt = atoi(inibuf);
	ini_getString(&conf.fd, "status", "rate_up", "2", inibuf, sizeof(inibuf));
	status_cfg.rate_up = atoi(inibuf) / 100.0;
	ini_getString(&conf.fd, "status", "rate_down", "-2", inibuf, sizeof(inibuf));
	status_cfg.rate_down = atoi(inibuf) / 100.0;

	status_init(&status_cfg);

	CutoffCfg cutoff_cfg;
	ini_getString(&conf.fd, "cutoff", "mission_timeout", "8400", inibuf, sizeof(inibuf));
	cutoff_cfg.mission_timeout = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "delta_altitude", "500", inibuf, sizeof(inibuf));
	cutoff_cfg.delta_altitude = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "altitude_timeout", "30", inibuf, sizeof(inibuf));
	cutoff_cfg.altitude_timeout = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "start_latitude", "43606414", inibuf, sizeof(inibuf));
	cutoff_cfg.start_latitude = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "start_longitude", "11311832", inibuf, sizeof(inibuf));
	cutoff_cfg.start_longitude = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "dist_max_meters", "80000", inibuf, sizeof(inibuf));
	cutoff_cfg.dist_max_meters = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "dist_timeout", "300", inibuf, sizeof(inibuf));
	cutoff_cfg.dist_timeout = atoi(inibuf);
	ini_getString(&conf.fd, "cutoff", "pwm_duty", "32768", inibuf, sizeof(inibuf));
	cutoff_cfg.pwm_duty = atoi(inibuf);

	cutoff_init(&cutoff_cfg);

	ini_getString(&conf.fd, "landing_buz", "buz_timeout", "9000", inibuf, sizeof(inibuf));
	uint32_t buz_timeout_seconds = atoi(inibuf);

	landing_buz_init(buz_timeout_seconds);

	RadioCfg radio_cfg;
	ini_getString(&conf.fd, "logging", "aprs_interval", "60", inibuf, sizeof(inibuf));
	radio_cfg.aprs_interval = atoi(inibuf);
	ini_getString(&conf.fd, "logging", "send_call", "STSP2", inibuf, sizeof(inibuf));
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
			log_start = timer_clock();
			//monitor_report();

			measures_logFormat(msg, sizeof(msg));
			kprintf("%s\n", msg);
			logging_data("%s\n", msg);
		}
	}
	return 0;
}
