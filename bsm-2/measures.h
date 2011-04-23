#ifndef BSM_2_MEASURES
#define BSM_2_MEASURES

#include <cfg/compiler.h>

typedef enum AccAxis
{
	ACC_X,
	ACC_Y,
	ACC_Z,
} AccAxis;

float measures_intTemp(void);
float measures_acceleration(AccAxis axis);
void measures_aprsFormat(char *buf, size_t len);
void measures_logFormat(char *buf, size_t len);
void measures_init(void);

#endif /* BSM_2_MEASURES */
