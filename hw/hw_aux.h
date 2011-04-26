#ifndef HW_AUX_H
#define HW_AUX_H

#include "hw_pin.h"

#include <io/arm.h>
#include <cfg/compiler.h>

INLINE void aux_out(bool val)
{
	if (val)
		PIOA_SODR = AUX_OUT;
	else
		PIOA_CODR = AUX_OUT;
}

INLINE void aux_init(void)
{
	PIOA_CODR = AUX_OUT;
	PIOA_PER = AUX_OUT;
	PIOA_OER = AUX_OUT;
}

#endif
