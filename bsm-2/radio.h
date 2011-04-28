#ifndef BSM2_RADIO_H
#define BSM2_RADIO_H

#include <drv/timer.h>

typedef struct RadioCfg
{
	mtime_t aprs_interval;
	char send_call[7];
} RadioCfg;

void radio_time(char *time_str, size_t size);
int radio_printf(const char * fmt, ...);
void radio_sendTelemetry(void);
void radio_init(RadioCfg *_cfg);

#endif
