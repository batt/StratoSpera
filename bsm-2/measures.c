#include "measures.h"
#include "gps.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

static char workbuf[256];

const char *measures_format(void)
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

	snprintf(workbuf, sizeof(workbuf), "/%02d%02d%02dh%s/%s>%ld;%s",
	t->tm_hour, t->tm_min, t->tm_sec,
	lat, lon,
	gps_info()->altitude,
	"other measures");

	return workbuf;
}

