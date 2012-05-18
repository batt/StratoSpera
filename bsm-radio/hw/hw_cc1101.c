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

#include "hw_cc1101.h"

#include "cfg/cfg_cc1101.h"

// Define log settings for cfg/log.h.
#define LOG_LEVEL         CC1101_LOG_LEVEL
#define LOG_FORMAT        CC1101_LOG_FORMAT
#include <cfg/log.h>
#include <cfg/debug.h>

#include <drv/cc1101.h>

#include <string.h>

const Setting ping_low_baud_868[] =
{
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

uint8_t tmp_buf[256];

int radio_send(const uint8_t *buf, size_t len)
{
	uint8_t status;
	//Flush the data in the fifo
	status = cc1101_strobe(CC1101_SFRX);
	LOG_INFO("FlushTx: Rdy[%d] St[%d] FifoAvail[%d]\n", UNPACK_STATUS(status));

	memset(tmp_buf, 0, sizeof(tmp_buf));

	size_t tx_len = MIN(sizeof(tmp_buf) - 1, len + 1);

	/*
	 * In current configuration the first byte in the fifo is the
	 * packet len.
	 */
	tmp_buf[0] = tx_len - 1;
	memcpy(&tmp_buf[1], buf, tx_len);

	cc1101_write(CC1101_TXFIFO, tmp_buf, tx_len);

	status = cc1101_strobe(CC1101_STX);
	LOG_INFO("TxData: Rdy[%d] St[%d] FifoAvail[%d] TxLen[%d]\n", UNPACK_STATUS(status), tx_len);

	return tx_len;
}

int radio_recv(uint8_t *buf, size_t len)
{

	uint8_t status = cc1101_strobe(CC1101_SFRX);
	LOG_INFO("FlushRx: Rdy[%d] St[%d] FifoAvail[%d]\n", UNPACK_STATUS(status));

	status = cc1101_strobe(CC1101_SRX);
	LOG_INFO("RxData: Rdy[%d] St[%d] FifoAvail[%d]\n", UNPACK_STATUS(status));

	uint8_t rx_data[2];
	// Waiting data from air..
	WAIT_FIFO_AVAIL();

	cc1101_read(CC1101_RXFIFO, rx_data, 2);

	size_t rx_len = MIN((size_t)rx_data[1], len);
	LOG_INFO("RxData: Rdy[%d] St[%d] FifoAvail[%d] RxLen[%d]\n", UNPACK_STATUS(status), rx_len);
	LOG_INFO("RSSI[%d] dBm\n", cc1101_rssidBm(rx_data[0], 74));

	cc1101_read(CC1101_RXFIFO, buf, rx_len);

	return rx_len;
}
