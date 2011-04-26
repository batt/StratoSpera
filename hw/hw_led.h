#ifndef HW_LED_H
#define HW_LED_H

#include "hw_pin.h"

#include <io/arm.h>
#include <cfg/compiler.h>

INLINE void ledr(bool val)
{
	if (val)
		PIOA_SODR = LEDR;
	else
		PIOA_CODR = LEDR;
}

INLINE void ledg(bool val)
{
	if (val)
		PIOA_SODR = LEDG;
	else
		PIOA_CODR = LEDG;
}

INLINE void led_init(void)
{
	PIOA_CODR = LEDR | LEDG;
	PIOA_PER = LEDR | LEDG;
	PIOA_OER = LEDR | LEDG;
	ledr(true);
}

#endif
