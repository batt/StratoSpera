#include "measures.h"
#include "gps.h"
#include "sensors.h"
#include "hadarp.h"
#include "radio.h"
#include "uplink.h"

#include "hw/hw_aux.h"

#include <kern/sem.h>

#include <drv/i2c.h>
#include <drv/lm75.h>
#include <drv/mma845x.h>

#include <mware/config.h>

#define LOG_LEVEL     LOG_LVL_INFO
#define LOG_VERBOSITY LOG_FMT_VERBOSE
#include <cfg/log.h>
#include "logging.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static I2c i2c_bus;
static Semaphore i2c_sem;

#define LM75_ADDR 0

float measures_intTemp(void)
{
	sem_obtain(&i2c_sem);
	float res = DEG_T_TO_FLOATDEG(lm75_read(&i2c_bus, LM75_ADDR));
	sem_release(&i2c_sem);

	return res;
}

float measures_acceleration(Mma845xAxis axis)
{
	sem_obtain(&i2c_sem);
	int acc = mma845x_read(&i2c_bus, 0, axis);
	sem_release(&i2c_sem);

	if (acc == MMA_ERROR)
		return -99.9;
	else
		return (acc * 9.80665 * 4.0) / 512;
}

static void measures_printMeasures(char *buf, size_t len)
{
	float humidity = sensor_read(ADC_HUMIDITY);
	float ext_t = sensor_read(ADC_T1);

	/*
	 * Honeywell HIH-5030/5031 humidity sensor temperature compensation.
	 * See page 2, http://http://sensing.honeywell.com/index.php?ci_id=49692
	 */
	humidity /= (1.0546 - 0.00216 * ext_t);

	snprintf(buf, len, "%ld;%.1f;%.1f;%.0f;%.0f;%.1f;%.2f;%.2f;%.2f;%.0f;%.2f;%.2f;%.2f;%d",
		gps_info()->altitude,
		ext_t,
		sensor_read(ADC_T2),
		sensor_read(ADC_PRESS),
		humidity,
		measures_intTemp(),
		sensor_read(ADC_VIN),
		sensor_read(ADC_5V),
		sensor_read(ADC_3V3),
		sensor_read(ADC_CURR),
		measures_acceleration(MMA_X),
		measures_acceleration(MMA_Y),
		measures_acceleration(MMA_Z),
		hadarp_read()
	);

	buf[len - 1] = '\0';
}

void measures_aprsFormat(char *buf, size_t len)
{
	const char *lat, *lon;
	if (gps_fixed())
	{
		lat = gps_aprsLat();
		lon = gps_aprsLon();
	}
	else
	{
		lat = "0000.00N";
		lon = "00000.00W";
	}

	char time[7];
	radio_time(time, sizeof(time));
	size_t cnt = snprintf(buf, len, "/%.6sh%s/%s>", time, lat, lon);
	if (cnt < len)
	{
		buf += cnt;
		len -= cnt;
		measures_printMeasures(buf, len);
	}
	else
		buf[len-1] = '\0';
}

void measures_logFormat(char *buf, size_t len)
{
	struct tm *t;
	time_t tim;
	udegree_t lat, lon;

	bool fix = gps_fixed();
	if (fix)
	{
		lat = gps_info()->latitude;
		lon = gps_info()->longitude;
	}
	else
	{
		lat = 0;
		lon = 0;
	}

	tim = gps_time();
	t = gmtime(&tim);

	size_t cnt = snprintf(buf, len, "%02d:%02d:%02d;%s;%02ld.%.06ld;%03ld.%.06ld;",
		t->tm_hour, t->tm_min, t->tm_sec,
		fix ? "FIX" : "NOFIX",
		lat/1000000, ABS(lat)%1000000,
		lon/1000000, ABS(lon)%1000000);

	if (cnt < len)
	{
		buf += cnt;
		len -= cnt;
		measures_printMeasures(buf, len);
	}
	else
		buf[len-1] = '\0';
}


typedef struct RawAcc
{
	uint8_t acc[6];
} PACKED RawAcc;

static RawAcc acc_buf[128];
static unsigned acc_idx = 0;

#define ACC_SAMPLE_RATE 50

static void NORETURN acc_process(void)
{
	ticks_t start = timer_clock();
	mtime_t delay = 0;
	while (1)
	{
		sem_obtain(&i2c_sem);
		bool r = mma845x_rawAcc(&i2c_bus, 0, acc_buf[acc_idx].acc);
		sem_release(&i2c_sem);
		if (!r)
			kprintf("ACC error!\n");
		if (++acc_idx >= countof(acc_buf))
		{
			acc_idx = 0;
			logging_acc(acc_buf, sizeof(acc_buf));
		}

		/* Wait for the next sample adjusting for time spent above */
		delay += (1000 / ACC_SAMPLE_RATE);
		timer_delay(delay - ticks_to_ms(timer_clock() - start));
	}
}

static void measures_reload(void);

DECLARE_CONF(measures, measures_reload,
	CONF_BOOL(curr_check, true),
	CONF_INT(curr_limit, 10, 5000, 500) //mA
);

static bool curr_logged;
static bool curr_override;
static void NORETURN curr_process(void)
{
	while (1)
	{
		if (!curr_override)
		{
			if (curr_check)
			{
				if (sensor_read(ADC_CURR) > curr_limit)
				{
					aux_out(false);
					if (!curr_logged)
					{
						radio_printf("Current overrange!\n");
						radio_printf("Switching OFF aux devices\n");
						curr_logged = true;
					}
				}
			}
			else
				aux_out(true);
		}

		timer_delay(1000);
	}
}

static bool cmd_curr_override(long cmd)
{
	curr_override = true;
	aux_out(cmd);
	return true;
}

static bool cmd_curr_resume(long l)
{
	(void)l;
	curr_override = false;
	curr_logged = false;
	return true;
}

static void measures_reload(void)
{
	LOG_INFO("Setting measures configuration\n");
	LOG_INFO(" current check: %s\n", curr_check ? "ON" : "OFF");
	LOG_INFO(" current limit: %d mA\n", curr_limit);
	curr_logged = false;

	/* Enable powerswitch for aux devices */
	aux_out(true);
}


void measures_init(void)
{
	sem_init(&i2c_sem);
	i2c_init(&i2c_bus, I2C_BITBANG0, CONFIG_I2C_FREQ);
	bool ret = mma845x_init(&i2c_bus, 0, MMADYN_4G);
	ASSERT(ret);

	Process *p = proc_new(acc_process, NULL, KERN_MINSTACKSIZE * 4, NULL);
	ASSERT(p);

	aux_init();

	config_register(&measures);
	config_load(&measures);

	/* Start current check process */
	p = proc_new(curr_process, NULL, KERN_MINSTACKSIZE * 3, NULL);
	ASSERT(p);

	uplink_registerCmd("curr_override", cmd_curr_override);
	uplink_registerCmd("curr_resume", cmd_curr_resume);
}
