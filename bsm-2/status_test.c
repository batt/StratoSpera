#include "status_mgr.h"
#include "cutoff.h"
#include "landing_buz.h"

#include <cfg/debug.h>
#include <cfg/test.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int status_testSetup(void)
{
	kdbg_init();

	CutoffCfg cutoff_cfg;
	cutoff_cfg.altitude_timeout = 60;
	cutoff_cfg.delta_altitude = 500;
	cutoff_cfg.dist_max_meters = 80000;
	cutoff_cfg.dist_timeout = 300;
	cutoff_cfg.mission_timeout = 8400;
	cutoff_cfg.start_latitude = 43606414;
	cutoff_cfg.start_longitude = 11311832;
	cutoff_setCfg(&cutoff_cfg);

	landing_buz_setCfg(9000);

	StatusCfg status_cfg;
	status_cfg.ground_alt = 1500;
	status_cfg.landing_alt = 3600;
	status_cfg.rate_down = -2;
	status_cfg.rate_up = 2;
	status_cfg.tropopause_alt = 12500;

	status_setCfg(&status_cfg);

	return 0;
}

char line[256];

#define DELAY 10
int status_testRun(void)
{
	FILE *fp = fopen("log.csv", "r");
	ASSERT(fp);
	int status_start = 0;

	while (fgets(line, sizeof(line), fp))
	{
		int t = atoi(strtok(line, ";"));

		if (t - status_start > DELAY * 2
			|| t - status_start < 0)
			status_start = 0;

		int fix = atoi(strtok(NULL, ";"));
		float lat = atof(strtok(NULL, ";"));
		float lon = atof(strtok(NULL, ";"));
		int alt = atoi(strtok(NULL, ";"));
		int press = atoi(strtok(NULL, ";"));

		kprintf("%d %d %f %f %d %d\n", t, fix, lat, lon, alt, press);
		if (status_start == 0)
		{
			status_start = t;
			status_missionStartAt(ms_to_ticks(t * 1000));
		}

		cutoff_checkAltitude(alt, ms_to_ticks(t * 1000));
		cutoff_checkDist(lat * 1000000, lon * 1000000, ms_to_ticks(t * 1000));
		cutoff_checkTime(ms_to_ticks(t * 1000));
		landing_buz_check(ms_to_ticks(t * 1000));

		if (t - status_start > DELAY)
		{
			status_check(fix, alt, press); //1013.25 * pow(1 - 2.25577e-5 * alt, 5.25588));
			status_start += DELAY;
		}
	}

	fclose(fp);
	return 0;
}

int status_testTearDown(void)
{
	return 0;
}

TEST_MAIN(status);
