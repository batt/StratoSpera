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
 * \brief Custom settings for CC1101.
 *
 * \author Daniele Basile <asterix@develer.com>
 */

#include <drv/cc1101.h>

Setting ping_low_baud_868[] = {
  {CC1101_IOCFG0,      0x06 /* GDO0 Output Pin Configuration */},
  {CC1101_FIFOTHR,     0x47 /* RX FIFO and TX FIFO Thresholds */},
  {CC1101_PKTLEN,      0x04 /* Packet Length */},
  {CC1101_PKTCTRL0,    0x05 /* Packet Automation Control */},
  {CC1101_PKTCTRL1,    0x0C /* Packet automation control. */ },
  {CC1101_FSCTRL1,     0x06 /* Frequency Synthesizer Control */},
  {CC1101_FREQ2,       0x21 /* Frequency Control Word, High Byte */},
  {CC1101_FREQ1,       0x62 /* Frequency Control Word, Middle Byte */},
  {CC1101_FREQ0,       0x76 /* Frequency Control Word, Low Byte */},
  {CC1101_MDMCFG4,     0xF6 /* Modem Configuration */},
  {CC1101_MDMCFG3,     0x83 /* Modem Configuration */},
  {CC1101_MDMCFG2,     0x93 /* Modem Configuration */},
  {CC1101_DEVIATN,     0x15 /* Modem Deviation Setting */},
  {CC1101_MCSM0,       0x18 /* Main Radio Control State Machine Configuration */},
  {CC1101_FOCCFG,      0x16 /* Frequency Offset Compensation Configuration */},
  {CC1101_WORCTRL,     0xFB /* Wake On Radio Control */},
  {CC1101_FSCAL3,      0xE9 /* Frequency Synthesizer Calibration */},
  {CC1101_FSCAL2,      0x2A /* Frequency Synthesizer Calibration */},
  {CC1101_FSCAL1,      0x00 /* Frequency Synthesizer Calibration */},
  {CC1101_FSCAL0,      0x1F /* Frequency Synthesizer Calibration */},
  {CC1101_TEST2,       0x81 /* Various Test Settings */},
  {CC1101_TEST1,       0x35 /* Various Test Settings */},
  {CC1101_TEST0,       0x09 /* Various Test Settings */},
  { 0xff, 0xff },
};
