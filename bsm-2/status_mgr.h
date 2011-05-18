#ifndef STATUS_MGR_H
#define STATUS_MGR_H

#include <cfg/compiler.h>
#include <drv/timer.h>

typedef struct StatusCfg
{
	int32_t ground_alt;     // meters
	int32_t tropopause_alt; // meters
	int32_t landing_alt;    // meters
	float rate_up;  // m/s
	float rate_down; // m/s (should be negative!)
} StatusCfg;

typedef enum VertDir
{
	HOVERING,
	UP,
	DOWN,
} VertDir;

typedef enum Bsm2Status
{
	BSM2_NOFIX = 0,
	BSM2_GROUND_WAIT,
	BSM2_TAKEOFF,
	BSM2_STRATOPHERE_UP,
	BSM2_STRATOPHERE_FALL,
	BSM2_FALLING,
	BSM2_LANDING,
	BSM2_HOVERING,

	BSM2_CNT
} Bsm2Status;

#define STATUS_CHECK_INTERVAL 10 //seconds

void status_missionStartAt(ticks_t ticks);
void status_missionStart(void);
ticks_t status_missionStartTicks(void);
mtime_t status_missionTime(void);
Bsm2Status status_currStatus(void);
void status_setTestStatus(Bsm2Status new_status);
void status_check(bool fix, int32_t curr_alt, float curr_press);
void status_setCfg(StatusCfg *_cfg);
void status_init(StatusCfg *cfg);

#endif
