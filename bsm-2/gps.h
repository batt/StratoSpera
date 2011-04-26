#ifndef BSM2_GPS_H
#define BSM2_GPS_H

#include <cfg/compiler.h>
#include <net/nmea.h>


INLINE const char *gps_aprsLat(void)
{
	extern char aprs_lat[9];
	return aprs_lat;
}

INLINE const char *gps_aprsLon(void)
{
	extern char aprs_lon[10];
	return aprs_lon;
}

INLINE const char *gps_aprsTime(void)
{
	extern char aprs_time[8];
	return aprs_time;
}

INLINE const NmeaGga *gps_info(void)
{
	extern NmeaGga gps_gga;
	return &gps_gga;
}

INLINE time_t gps_time(void)
{
	extern time_t gps_clock;
	return gps_clock;
}

INLINE bool gps_fixed(void)
{
	extern bool gps_fix;
	return gps_fix;
}

void gps_setTestmode(bool mode);
void gps_init(unsigned port, unsigned long baudrate);

#endif
