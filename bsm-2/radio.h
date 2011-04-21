#ifndef BSM2_RADIO_H
#define BSM2_RADIO_H

#include <drv/timer.h>

typedef struct RadioCfg
{
	mtime_t aprs_interval;
	char send_call[7];
} RadioCfg;

void radio_send(char *buf, size_t len);
void radio_init(RadioCfg *_cfg);

#endif
