#include "testmode.h"

#include "adc_mgr.h"
#include "radio.h"
#include "cutoff.h"
#include "status_mgr.h"
#include "measures.h"
#include "sensors.h"
#include "hadarp.h"

#include "hw/hw_led.h"
#include "hw/hw_buzzer.h"
#include "hw/hw_aux.h"

#include <drv/timer.h>
#include <drv/kbd.h>

#include <cfg/debug.h>


#define TESTMODE_TIMEOUT ms_to_ticks(5*60*1000) //5 minutes

static bool test_mode;

bool testmode(void)
{
	return test_mode;
}

void testmode_run(void)
{
	kprintf("Entering test mode...\n");
	test_mode = true;

	ticks_t start = timer_clock();
	ticks_t adc_start = timer_clock();

	Bsm2Status status = 0;
	bool led = true;
	ledr(led);
	ledg(led);

	while (timer_clock() - start < TESTMODE_TIMEOUT)
	{
		if (timer_clock() - adc_start > ms_to_ticks(1000))
		{
			adc_start = timer_clock();
			ledr(led);
			ledg(!led);
			led = !led;

			for (int i = 0; i < ADC_CHANNELS; i++)
			{
				uint16_t val = adc_mgr_read(i);
				kprintf("CH%d ADC:%4d -> ", i, val);
				kprintf("%7.2f\n", sensor_read(i));
			}
			kprintf("LM75 temp %.1fC, acceleration X %.2f Y %.2f Z %.2f m/s^2\n",
				measures_intTemp(),
				measures_acceleration(ACC_X),
				measures_acceleration(ACC_Y),
				measures_acceleration(ACC_Z));
			kprintf("HADARP count:%d\n", hadarp_read());
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

			radio_printf("TEST MODE");
			radio_sendTelemetry();

			timer_delay(2500);

			aux_out(false);
			cutoff_test_cut(false);
			BUZZER_OFF;
			ledr(false);
			ledg(false);
		}

		timer_delay(100);
	}

	test_mode = false;
}
