#ifndef BSM_2_MEASURES
#define BSM_2_MEASURES

#include <cfg/compiler.h>
#include <drv/mma845x.h>

float measures_intTemp(void);
float measures_acceleration(Mma845xAxis axis);
void measures_aprsFormat(char *buf, size_t len);
void measures_logFormat(char *buf, size_t len);
void measures_init(void);

#endif /* BSM_2_MEASURES */
