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
 * This parser can handle two kind of messages: generic command messages and
 * configuration ones.
 * For commands, the format is:
 * <cmd> <param>
 * Where <cmd> is the command and <param> is a simple integer parameter.
 *
 * In case of configuration messages, the format is:
 * cfg <module> <param1>=<val1> <param2>=<val2> ...
 * 'cfg' is a header which tells the parser that this is configuration message,
 * then follows the <module> which needs to be configured and then a list of
 * <param>=<val> pairs for the parameters to be set.
 * At the end of the list of parameters, if at least one parameter has been
 * found, the configuration of <module> is reloaded.
 *
 * \author Francesco Sacchi <batt@develer.com>
 */

#include "uplink.h"

#include <mware/config.h>
#include <mware/find_token.h>
#include <struct/list.h>

#include <string.h>
#include <stdlib.h>

#define LOG_LEVEL     LOG_LVL_INFO
#include <cfg/log.h>
#include "logging.h"

#define MAX_LEN 33
static char module[MAX_LEN];
static char param[MAX_LEN];
static char val[MAX_LEN];

static List commands;

bool uplink_parse(const char *buf, size_t len)
{
	const char *end = buf + len;
	bool err = false;
	bool found = false;

	buf = find_token(buf, end - buf, module, MAX_LEN, " \t,:;(=");
	if (strcmp(module, "cfg") == 0)
	{
		LOG_INFO("Config message received\n");
		buf = find_token(buf, end - buf, module, MAX_LEN, " \t\n,;");
		LOG_INFO("Module '%s'\n", module);
		do
		{
			buf = find_token(buf, end - buf, param, MAX_LEN, ":= \t");
			buf = find_token(buf, end - buf, val, MAX_LEN, " \t\n,;");
			if (!*module || !*param || !*val)
				break;

			found = true;
			LOG_INFO("Param '%s', val '%s'\n", param, val);
			bool cfg_ok = config_set(module, param, val);
			LOG_INFO("Setting '%s': %s\n", param, cfg_ok ? "OK" : "FAIL");
			err = err || !cfg_ok;
		}
		while (1);

		if (found)
		{
			LOG_INFO("Reloading configuration for module '%s'\n", module);
			config_reload(module);
		}
	}
	else
	{
		LOG_INFO("Searching for command '%s'...\n", module);
		if (*module)
		{
			UplinkCmd *cmd;
			FOREACH_NODE(cmd, &commands)
			{
				if (strcmp(cmd->name, module) == 0)
				{
					found = true;
					find_token(buf, end - buf, val, MAX_LEN, " \t)");
					LOG_INFO("Executing command '%s', val '%s'\n", module, val);
					err = !cmd->cmd(strtol(val, NULL, 0));
					LOG_INFO("Command '%s' done.\n", module);
					break;
				}
			}
		}
		if (!found)
			LOG_INFO("Command '%s' not found.\n", module);
	}

	return found && !err;
}

void uplink_RegisterCommandPrivate(UplinkCmd *cmd)
{
	ADDTAIL(&commands, &cmd->node);
}


void uplink_init(void)
{
	LIST_INIT(&commands);
}
