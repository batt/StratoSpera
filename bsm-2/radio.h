#ifndef BSM2_RADIO_H
#define BSM2_RADIO_H

#include <drv/timer.h>

void radio_time(char *time_str, size_t size);
int radio_printf(const char * fmt, ...);
void radio_sendTelemetry(void);
void radio_init(void);

#endif
