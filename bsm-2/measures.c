#include "measures.h"
#include "gps.h"
#include "sensors.h"
#include "hadarp.h"

#include <drv/i2c.h>
#include <drv/lm75.h>

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

static I2c i2c_bus;

#define LM75_ADDR 0

void measures_aprsFormat(char *buf, size_t len)
{
	struct tm t;
	time_t tim;

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

	tim = gps_time();
	gmtime_r(&tim, &t);

	#warning "fixme: read these values."
	float acc = 9.81;

	snprintf(buf, len, "/%02d%02d%02dh%s/%s>%ld;%.1f;%.0f;%.0f;%.1f;%.2f;%.2f;%d",
		t.tm_hour, t.tm_min, t.tm_sec,
		lat, lon,
		gps_info()->altitude,
		sensor_read(ADC_T1),
		sensor_read(ADC_PRESS),
		sensor_read(ADC_HUMIDITY),
		DEG_T_TO_FLOATDEG(lm75_read(&i2c_bus, LM75_ADDR)),
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

	#warning "fixme: read these values."
	float acc_x = 9.81;
	float acc_y = 9.81;
	float acc_z = 9.81;

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
		DEG_T_TO_FLOATDEG(lm75_read(&i2c_bus, LM75_ADDR)),
		sensor_read(ADC_VIN),
		sensor_read(ADC_5V),
		sensor_read(ADC_3V3),
		sensor_read(ADC_CURR),
		acc_x,
		acc_y,
		acc_z,
		hadarp_read()
	);

	buf[len - 1] = '\0';
}

void measures_init(void)
{
	i2c_init(&i2c_bus, I2C_BITBANG0, CONFIG_I2C_FREQ);
}
