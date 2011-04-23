#include "testmode.h"

#include "adc_mgr.h"
#include "gps.h"
#include "radio.h"
#include "cutoff.h"
#include "status_mgr.h"

#include "hw/hw_led.h"
#include "hw/hw_buzzer.h"
#include "hw/hw_aux.h"

#include <drv/timer.h>
#include <drv/kbd.h>

#include <cfg/debug.h>


#define TESTMODE_TIMEOUT ms_to_ticks(5*60*1000) //5 minutes

void testmode_run(void)
{
	kprintf("Entering test mode...\n");

	gps_setTestmode(true);
	radio_setTestmode(true);
	cutoff_setTestmode(true);
	status_setTestmode(true);

	ticks_t start = timer_clock();
	ticks_t adc_start = timer_clock();

	Bsm2Status status = 0;

	while (timer_clock() - start < TESTMODE_TIMEOUT)
	{
		if (timer_clock() - adc_start > ms_to_ticks(1000))
		{
			adc_start = timer_clock();

			for (int i = 0; i < ADC_CHANNELS; i++)
			{
				uint16_t val = adc_mgr_read(i);
				kprintf("CH%d %d\n", i, val);
			}
			kputchar('\n');
		}

		if (kbd_peek() & K_START)
		{
			ledr(true);
			ledg(true);
			BUZZER_ON;
			aux_out(true);
			cutoff_test_cut(true);
			status_setTestStatus(status++);
			if (status >= BSM2_CNT)
				status = 0;

			radio_printf("TEST");

			timer_delay(2500);

			aux_out(false);
			cutoff_test_cut(false);
			BUZZER_OFF;
			ledr(false);
			ledg(false);
		}

		timer_delay(100);
	}

	gps_setTestmode(false);
	radio_setTestmode(false);
	cutoff_setTestmode(false);
	status_setTestmode(false);
}
