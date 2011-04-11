#include "measures.h"
#include "gps.h"
#include "sensors.h"
#include "hadarp.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

void measures_aprsFormat(char *buf, size_t len)
{
	struct tm *t;
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
	t = gmtime(&tim);

	snprintf(buf, len, "/%02d%02d%02dh%s/%s>%ld;%.1f;%.0f;%.0f;%.1f;%.2f;%.2f;%d",
	t->tm_hour, t->tm_min, t->tm_sec,
	lat, lon,
	gps_info()->altitude,
	sensor_read(ADC_T1),
	sensor_read(ADC_PRESS),
	sensor_read(ADC_HUMIDITY),
	10.23, //Internal temp
	sensor_read(ADC_VIN),
	9.81, //accelerometer
	hadarp_read()
	);

	buf[len - 1] = '\0';
}

void measures_logFormat(char *buf, size_t len)
{
	struct tm *t;
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
	t = gmtime(&tim);

	snprintf(buf, len, "/%02d%02d%02dh%s/%s>%ld;%.1f;%.0f;%.0f;%.1f;%.2f;%.2f;%d",
	t->tm_hour, t->tm_min, t->tm_sec,
	lat, lon,
	gps_info()->altitude,
	sensor_read(ADC_T1),
	sensor_read(ADC_PRESS),
	sensor_read(ADC_HUMIDITY),
	10.23, //Internal temp
	sensor_read(ADC_VIN),
	9.81, //accelerometer
	hadarp_read()
	);

	buf[len - 1] = '\0';
}
