/*
 * $test$: cp bertos/cfg/cfg_kfile.h $cfgdir/
 * $test$: echo "#undef CONFIG_KFILE_GETS" >> $cfgdir/cfg_kfile.h
 * $test$: echo "#define CONFIG_KFILE_GETS 1" >> $cfgdir/cfg_kfile.h
 */

#include "status_mgr.h"
#include "cutoff.h"
#include "landing_buz.h"

#include <fs/fat.h>
#include <emul/kfile_posix.h>
#include <mware/config.h>

#include <cfg/debug.h>
#include <cfg/test.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static const char ini_file[] = "conf.ini";
static KFilePosix kf;


int status_testSetup(void)
{
	kdbg_init();

	if (!kfile_posix_init(&kf, ini_file, "r"))
	{
		kprintf("No test file found\n");
		return -1;
	}

	config_init(&kf.fd);

	cutoff_init();
	landing_buz_setCfg(9000);
	status_init();

	return 0;
}

char line[256];

int status_testRun(void)
{
	FILE *fp = fopen("log.csv", "r");
	ASSERT(fp);
	int status_start = 0;

	while (fgets(line, sizeof(line), fp))
	{
		int t = atoi(strtok(line, ";"));

		if (t - status_start > STATUS_CHECK_INTERVAL * 2
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

		cutoff_check(ms_to_ticks(t * 1000), alt, lat * 1000000, lon * 1000000);

		landing_buz_check(ms_to_ticks(t * 1000));

		if (t - status_start > STATUS_CHECK_INTERVAL)
		{
			status_check(fix, alt, press); //1013.25 * pow(1 - 2.25577e-5 * alt, 5.25588));
			status_start += STATUS_CHECK_INTERVAL;
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
