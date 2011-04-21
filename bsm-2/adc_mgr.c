/**
 * \file
 * <!--
 * This file is part of BeRTOS.
 *
 * Bertos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 *
 * Copyright 2006 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \brief ADC manager.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */


#include "adc_mgr.h"
#include "hw/hw_pin.h"

#include <cpu/irq.h>

#include <net/afsk.h>

#define LOG_LEVEL LOG_LVL_INFO
#include <cfg/log.h>

#include <io/arm.h>
#include <cfg/compiler.h>
#include <cfg/macros.h>


#define SAMPLE_RATE             CONFIG_AFSK_DAC_SAMPLERATE
#define CONFIG_ADC_CLOCK        (SAMPLE_RATE * 520)
#define CONFIG_ADC_STARTUP_TIME 20
#define CONFIG_ADC_SHTIME       1600

#define ADC_COMPUTED_PRESCALER    ((CPU_FREQ / (2 * CONFIG_ADC_CLOCK)) - 1)
#define ADC_COMPUTED_STARTUPTIME  (((CONFIG_ADC_STARTUP_TIME * CONFIG_ADC_CLOCK) / 8000000UL) - 1)
#define ADC_COMPUTED_SHTIME       (uint32_t)((((uint64_t)CONFIG_ADC_SHTIME * CONFIG_ADC_CLOCK + 500000000UL) / 1000000000UL) - 1)

// Set ADC_MGR_STROBE to 1 in order to enable debugging of adc isr duration.
#define ADC_MGR_STROBE 1
#if ADC_MGR_STROBE
	#warning "ADC_MGR_STROBE active"

	#define ADC_MGR_STROBE_LOW()  (PIOA_CODR = ADC_STROBE_PIN)
	#define ADC_MGR_STROBE_HIGH() (PIOA_SODR = ADC_STROBE_PIN)
	#define ADC_MGR_STROBE_INIT() \
	do { \
			PIOA_OER = ADC_STROBE_PIN; \
			PIOA_SODR = ADC_STROBE_PIN; \
			PIOA_PER = ADC_STROBE_PIN; \
	} while (0)
#else
	#define ADC_MGR_STROBE_LOW()
	#define ADC_MGR_STROBE_HIGH()
	#define ADC_MGR_STROBE_INIT()
#endif


static Afsk *afsk_ctx;
bool hw_afsk_dac_isr;

#define ADC_CHANNELS  8

#define EXT_MUX_MASK (BV(EXT_MUX_START) | BV(EXT_MUX_START+1) | BV(EXT_MUX_START+2))

INLINE void adc_mux_sel(unsigned ch)
{
	ASSERT(ch < ADC_CHANNELS);
	uint32_t tmp = PIOA_ODSR;
	tmp &= ~EXT_MUX_MASK;

	PIOA_ODSR = tmp | ((uint32_t)ch << EXT_MUX_START);
}

INLINE void adc_mux_init(void)
{
	PIOA_OER = EXT_MUX_MASK;
	PIOA_PER = EXT_MUX_MASK;
	PIOA_OWER = EXT_MUX_MASK;
	adc_mux_sel(0);
}

static unsigned curr_ch;
static unsigned afsk_ch;

static void adc_selectChannel(unsigned ch)
{
	//Disable all channels
	ADC_CHDR = ADC_CH_MASK;
	adc_mux_sel(ch);

	ADC_CHER = BV(ADC_MUX_CH) | BV(afsk_ch);
}

static void adc_selectNextCh(void)
{
	if (++curr_ch >= ADC_CHANNELS)
		curr_ch = 0;

	adc_selectChannel(curr_ch);
}

typedef struct Iir
{
	int32_t x[2];
	int32_t y[2];
} Iir;

static Iir filters[ADC_CHANNELS];

#define FILTER_ENABLE 0
#define IIR_FREQ 8
#if (IIR_FREQ == 8)
	#define GAIN 4.873950141e+01
	#define IIR_SHIFT 9
	#define IIR_CONST 0.9589655220

	#define IIR_GAIN  ((int)((BV(IIR_SHIFT) / GAIN)))
#else
	#error "Filter constants for this frequency not defined"
#endif

uint16_t adc_mgr_read(AdcChannels ch)
{
	ASSERT(ch < ADC_CHANNELS);

	#if FILTER_ENABLE
		/*
		 * The shift (>> IIR_SHIFT) is needed in order to get the integer part
		 * (filter state uses IIR_SHIFT bits fixed point arithmetic).
		 */
		return filters[ch].y[1] >> IIR_SHIFT;
	#else
		#warning "ADC filter disabled"
		return filters[ch].y[1];
	#endif
}


INLINE void afsk_handler(int8_t val)
{
	afsk_adc_isr(afsk_ctx, val);

	uint32_t tmp = PIOA_ODSR;
	tmp &= ~DAC_PIN_MASK;

	if (hw_afsk_dac_isr)
		PIOA_ODSR = tmp | ((afsk_dac_isr(afsk_ctx) << (DAC_PIN_START - 4)) & DAC_PIN_MASK);
	else
		/* Vdac / 2 */
		PIOA_ODSR = tmp | BV(DAC_PIN_START + 3);
}

static DECLARE_ISR(adc_mgr_isr)
{
	ADC_MGR_STROBE_LOW();
	unsigned old_ch = curr_ch;

	adc_selectNextCh();

	// Ack hw interrupt
	(void)ADC_LCDR;

	reg32_t *adc_res = &ADC_CDR0;
	afsk_handler((adc_res[afsk_ch] >> 2) - 128);

	uint16_t adc_val = adc_res[ADC_MUX_CH];
	#if FILTER_ENABLE
		/*
		 * This filter is designed to work at 1200Hz,
		 * update the coefficients if the number of channels
		 * is modified.
		 */
		STATIC_ASSERT(SAMPLE_RATE / ADC_CHANNELS == 1200);

		/*
		 * Read channel and apply a low pass filter, butterworth approximation,
		 * at about IIR_FREQ Hz.
		 * Filter state uses IIR_SHIFT bits fixed point arithmetic in order to
		 * be fast but retain sufficient precision and stability.
		 */
		filters[old_ch].x[0] = filters[old_ch].x[1];
		filters[old_ch].x[1] = adc_val * IIR_GAIN;
		filters[old_ch].y[0] = filters[old_ch].y[1];
		filters[old_ch].y[1] = filters[old_ch].x[0] + filters[old_ch].x[1] +
								INT_MULT(filters[old_ch].y[0], IIR_CONST, 8);
	#else
		filters[old_ch].y[1] = adc_val;
	#endif

	AIC_EOICR = 0;
	ADC_MGR_STROBE_HIGH();
}

void adc_mgr_init(int ch, struct Afsk * ctx)
{
	ADC_MGR_STROBE_INIT();

	LOG_INFO("prescaler[%ld], stup[%ld], shtim[%ld]\n", ADC_COMPUTED_PRESCALER,
								ADC_COMPUTED_STARTUPTIME, ADC_COMPUTED_SHTIME);

	ADC_MR = 0;

	afsk_ctx = ctx;
	afsk_ctx->adc_ch = ch;
	afsk_ch = ch;

	//Apply computed prescaler value
	ADC_MR &= ~ADC_PRESCALER_MASK;
	ADC_MR |= ((ADC_COMPUTED_PRESCALER << ADC_PRESCALER_SHIFT) & ADC_PRESCALER_MASK);

	//Apply computed start up time
	ADC_MR &= ~ADC_STARTUP_MASK;
	ADC_MR |= ((ADC_COMPUTED_STARTUPTIME << ADC_STARTUP_SHIFT) & ADC_STARTUP_MASK);

	//Apply computed sample and hold time
	ADC_MR &= ~ADC_SHTIME_MASK;
	ADC_MR |= ((ADC_COMPUTED_SHTIME << ADC_SHTIME_SHIFT) & ADC_SHTIME_MASK);

	// Disable all interrupt
	ADC_IDR = 0xFFFFFFFF;

	//Register interrupt vector
	AIC_SVR(ADC_ID) = adc_mgr_isr;
	AIC_SMR(ADC_ID) = AIC_SRCTYPE_INT_EDGE_TRIGGERED;
	AIC_IECR = BV(ADC_ID);

	/*
	 * Set convertion rate to SAMPLE_RATE (9600 Hz).
	 * Since we have 8 channels, each of them will be sampled at 1200Hz.
	 */
	PMC_PCER = BV(TC0_ID);
	TC_BMR = TC_NONEXC0;
	TC0_CCR = BV(TC_SWTRG) | BV(TC_CLKEN);
	TC0_CMR = BV(TC_WAVE);
	TC0_CMR |= (TC_WAVSEL_UP_RC_TRG | TC_ACPC_CLEAR_OUTPUT | TC_ACPA_SET_OUTPUT);
	TC0_RC = (CPU_FREQ / 2) / SAMPLE_RATE;
	TC0_RA = TC0_RC / 2;

	adc_mux_init();

	// Auto trigger enabled on TIOA channel 0
	ADC_MR |= BV(ADC_TRGEN);

	//Disable all channels
	ADC_CHDR = ADC_CH_MASK;

	// Start from ch0
	curr_ch = 0;
	adc_selectChannel(curr_ch);

	// This ensure IRQ will be triggered when all channels are done
	ADC_IER = BV(MAX(afsk_ch, (unsigned)ADC_MUX_CH));
}
