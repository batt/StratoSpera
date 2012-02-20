#ifndef BSM2_RADIO_H
#define BSM2_RADIO_H

#include <drv/timer.h>

void radio_time(char *time_str, size_t size);
#if !(ARCH & ARCH_UNITTEST)
	int radio_printf(const char * fmt, ...);
#else
	#define radio_printf(...) LOG_INFO(__VA_ARGS__)
#endif

void radio_sendTelemetry(void);
void radio_init(void);

#endif
