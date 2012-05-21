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
 * Copyright 2012 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \brief CC1101 transceiver
 *
 * \author Daniele Basile <asterix@develer.com>
 */

#ifndef HW_CC1101_H
#define HW_CC1101_H

#include <cfg/macros.h>

#include <io/stm32.h>
#include <cpu/types.h>
#include <cpu/power.h>

#include <drv/gpio_stm32.h>
#include <drv/cc1101.h>

#define GPIO_BASE_A       ((struct stm32_gpio *)GPIOA_BASE)
#define GPIO_BASE_B       ((struct stm32_gpio *)GPIOB_BASE)

extern const Setting ping_low_baud_868[];

/*
 * Radio ids table.
 */
#define RADIO_MASTER    0

/*
 * Radio errors
 */
#define RADIO_TX_ERR        -1
#define RADIO_RX_ERR        -2
#define RADIO_RX_TIMEOUT    -3

/*
 * Get the device id
 */
INLINE uint8_t radio_id(void)
{
	return stm32_gpioPinRead(GPIO_BASE_B, BV(5) | BV(6)) >> 5;
}

int radio_send(const void *buf, size_t len);
int radio_recv(void *buf, size_t len, mtime_t timeout);
uint8_t radio_status(void);
void radio_sleep(void);
int radio_rssi(void);
int radio_lqi(void);


#define CC1101_HW_INIT() \
do { \
	RCC->APB2ENR |= RCC_APB2_GPIOB;			\
	stm32_gpioPinConfig(GPIO_BASE, BV(11), GPIO_MODE_IN_FLOATING, GPIO_SPEED_50MHZ); \
	stm32_gpioPinConfig(GPIO_BASE_B, BV(5) | BV(6), GPIO_MODE_IPU, GPIO_SPEED_50MHZ); \
} while (0)

#endif /* HW_CC1101_H */
