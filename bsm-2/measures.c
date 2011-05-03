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

float measures_acceleration(AccAxis axis)
{
	ASSERT(axis < ACC_CNT);
	float acc[3];
	sem_obtain(&i2c_sem);
	mma845x_read(&i2c_bus, 1, acc);
	sem_release(&i2c_sem);

	return acc[axis];
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

	float x = measures_acceleration(ACC_X);
	float y = measures_acceleration(ACC_Y);
	float z = measures_acceleration(ACC_Z);

	float acc = sqrt(x * x + y * y + z * z);

	char time[7];
	radio_time(time, sizeof(time));

	snprintf(buf, len, "/%.6sh%s/%s>%ld;%.1f;%.0f;%.0f;%.1f;%.2f;%.2f;%d",
		time,
		lat, lon,
		gps_info()->altitude,
		sensor_read(ADC_T1),
		sensor_read(ADC_PRESS),
		sensor_read(ADC_HUMIDITY),
		measures_intTemp(),
		sensor_read(ADC_VIN),
		acc,
		hadarp_read()
	);

	buf[len - 1] = '\0';
}

void measures_logFormat(char *buf, size_t len)
{
	struct tm *t;
	time_t tim;
	udegree_t lat, lon;
	int32_t altitude;


	bool fix = gps_fixed();
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

	snprintf(buf, len, "%02d:%02d:%02d;%s;%02ld.%.06ld;%03ld.%.06ld;%ld;%.1f;%.1f;%.0f;%.0f;%.1f;%.2f;%.2f;%.2f;%.0f;%.2f;%.2f;%.2f;%d",
		t->tm_hour, t->tm_min, t->tm_sec,
		fix ? "FIX" : "NOFIX",
		lat/1000000, ABS(lat)%1000000,
		lon/1000000, ABS(lon)%1000000,
		altitude,
		sensor_read(ADC_T1),
		sensor_read(ADC_T2),
		sensor_read(ADC_PRESS),
		sensor_read(ADC_HUMIDITY),
		measures_intTemp(),
		sensor_read(ADC_VIN),
		sensor_read(ADC_5V),
		sensor_read(ADC_3V3),
		sensor_read(ADC_CURR),
		measures_acceleration(ACC_X),
		measures_acceleration(ACC_Y),
		measures_acceleration(ACC_Z),
		hadarp_read()
	);

	buf[len - 1] = '\0';
}

void measures_init(void)
{
	sem_init(&i2c_sem);
	i2c_init(&i2c_bus, I2C_BITBANG0, CONFIG_I2C_FREQ);
}
