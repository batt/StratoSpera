#include "measures.h"
#include "gps.h"
#include "sensors.h"
#include "hadarp.h"
#include "radio.h"

#include <kern/sem.h>

#include <drv/i2c.h>
#include <drv/lm75.h>
#include <drv/mma845x.h>

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
		return (acc * 9.81 * 4.0) / 512;
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

void measures_init(void)
{
	sem_init(&i2c_sem);
	i2c_init(&i2c_bus, I2C_BITBANG0, CONFIG_I2C_FREQ);
	bool ret = mma845x_init(&i2c_bus, 0, MMADYN_4G);
	ASSERT(ret);
}
