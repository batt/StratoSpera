#ifndef STATUS_MGR_H
#define STATUS_MGR_H

#include <cfg/compiler.h>
#include <drv/timer.h>

typedef struct StatusCfg
{
	int32_t ground_alt;
	int32_t tropopause_alt;
	int32_t landing_alt;
	int32_t delta_up;
	int32_t delta_down;
} StatusCfg;


typedef enum Bsm2Status
{
	BSM2_NOFIX = 0,
	BSM2_GROUND_WAIT,
	BSM2_TAKEOFF,
	BSM2_STRATOPHERE_UP,
	BSM2_CUTOFF,
	BSM2_STRATOPHERE_FALL,
	BSM2_FALLING,
	BSM2_LANDING,
	BSM2_HOVERING,

	BSM2_CNT
} Bsm2Status;

void status_missionStart(void);
ticks_t status_missionStartTicks(void);
void status_check(bool fix, int32_t curr_alt);
void status_init(void);

#endif
