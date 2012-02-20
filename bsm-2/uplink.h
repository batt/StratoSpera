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
 * \brief Uplink command parser
 *
 * \author Francesco Sacchi <batt@develer.com>
 */
#ifndef BSM2_UPLINK_H
#define BSM2_UPLINK_H

#include <cfg/compiler.h>
#include <struct/list.h>

typedef bool (*uplink_cmd_hook_t)(long);

typedef struct UplinkCmd
{
	Node node;
	const char *name;
	uplink_cmd_hook_t cmd;
} UplinkCmd;


void uplink_RegisterCommandPrivate(UplinkCmd *cmd);

#if !(ARCH & ARCH_UNITTEST)
	#define uplink_registerCmd(str, callback) \
		do { \
			static UplinkCmd PP_CAT(_uplink_cmd_, __LINE__) = { .name = str, .cmd = callback }; \
			uplink_RegisterCommandPrivate(&PP_CAT(_uplink_cmd_, __LINE__)); \
		} while (0)
#else
	#define uplink_registerCmd(str, callback) \
		do { \
			(void)callback; \
			LOG_INFO("Registering command '%s'\n", str); \
		} while(0)
#endif

bool uplink_parse(const char *buf, size_t len);
void uplink_init(void);

#endif
