#include "config.h"

#include <string.h> // strncmp
#include <stdlib.h> // strtol
#include <mware/parser.h> // get_word
#include <errno.h> // errno
#include <limits.h> // LONG_MIN, LONG_MAX
#include <stdio.h> // snprintf
#include <mware/ini_reader.h>

#define LOG_LEVEL   LOG_LVL_INFO
#include <cfg/log.h>

#define INT_VAR(var)   *((int *) (var))
#define FLOAT_VAR(var)   *((float *) (var))

static List config;
static KFile *fd;

void config_init(KFile *_fd)
{
    ASSERT(_fd);
    fd = _fd;
	LIST_INIT(&config);
}

void config_register(ConfigMetadata *cfg)
{
	ADDTAIL(&config, (Node *)cfg);
}

const ConfigEntry *config_findEntry(ConfigMetadata **cfg, const char *name)
{
	const ConfigEntry *e = NULL;
	FOREACH_NODE(*cfg, &config)
	{
		for (size_t i = 0; i < (*cfg)->tbl_size; ++i)
		{
			if (!strncmp((*cfg)->table[i].name, name, CFG_NAME_MAX))
			{
				e = &((*cfg)->table[i]);
				goto end;
			}
		}
	}

end:
	return e;
}

void config_load(ConfigMetadata *cfg)
{
	char input_string[MAX_VAL];

	for (size_t i = 0; i < cfg->tbl_size; ++i)
	{
		const ConfigEntry *e = &(cfg->table[i]);
        if (ini_getString(fd, cfg->name, e->name, e->default_val, input_string, sizeof(input_string)) != 0)
			LOG_WARN("An error occurred while reading key \"%s\"\n", e->name);
		e->setp(e, input_string, true);
	}

	if (cfg->reload)
		cfg->reload();
}


SetPRetVals config_setInt(const struct ConfigEntry *e, char *val, bool use_default)
{
	long conv = 0;
	char *endptr;
	LOG_INFO("Value to be converted: %s\n", val);
	errno = 0;
	conv = strtol(val, &endptr, 0);
	if ((errno == ERANGE && (conv == LONG_MAX || conv == LONG_MIN))
		|| (errno != 0 && conv == 0)
		|| (endptr == val))
	{
		if (use_default)
		{
			LOG_INFO("Using default value: %s\n", e->default_val);
			conv = strtol(e->default_val, &endptr, 0);
			ASSERT(*endptr == 0);
			INT_VAR(e->parms[1].p) = (int) conv;
		}
		LOG_INFO("Error in conversion\n");
		return SRV_CONV_ERR;
	}

	/* Validate value limits */
	INT_VAR(e->parms[1].p) = (int) conv;
	LOG_INFO("Value converted (long): %ld\n", conv);
	LOG_INFO("Clamping value between [%d] and [%d]\n", e->parms[0].i, e->parms[2].i);
	conv = MINMAX(e->parms[0].i, (int)conv, e->parms[2].i);
	LOG_INFO("Value before %d, value after %ld\n", INT_VAR(e->parms[1].p), conv);

	if (INT_VAR(e->parms[1].p) != (int) conv)
	{
		INT_VAR(e->parms[1].p) = (int) conv;
		LOG_INFO("Value clamped\n");
		return SRV_OK_CLAMPED;
	}
	return SRV_OK;
}

SetPRetVals config_setFloat(const struct ConfigEntry *e, char *val, bool use_default)
{
	float conv = 0.0;
	char *endptr;
	LOG_INFO("Value to be converted: %s\n", val);

	conv = strtod(val, &endptr);

	if (conv == 0.0 && endptr == val)
	{
		if (use_default)
		{
			LOG_INFO("Using default value: %s\n", e->default_val);
			conv = strtod(e->default_val, &endptr);
			ASSERT(*endptr == 0);
			FLOAT_VAR(e->parms[1].p) = conv;
		}
		LOG_INFO("Error in conversion\n");
		return SRV_CONV_ERR;
	}

	/* Validate value limits */
	FLOAT_VAR(e->parms[1].p) = conv;
	LOG_INFO("Value converted: %g\n", conv);
	LOG_INFO("Clamping value between [%g] and [%g]\n", e->parms[0].f, e->parms[2].f);
	conv = MINMAX(e->parms[0].f, conv, e->parms[2].f);
	LOG_INFO("Value before %g, value after %g\n", FLOAT_VAR(e->parms[1].p), conv);

	if (FLOAT_VAR(e->parms[1].p) != conv)
	{
		FLOAT_VAR(e->parms[1].p) = conv;
		LOG_INFO("Value clamped\n");
		return SRV_OK_CLAMPED;
	}
	return SRV_OK;
}

static bool decode_boolArray(const char *str, size_t size, bool *array)
{
	int b = 0;
	for (size_t i = 0; i < size; i += 2)
	{
		if (str[i] != '0' && str[i] != '1')
			return false;

		/* check for correct spacing until eol */
		if (str[i + 1] != ' ' && (i + 1 < size))
			return false;

		array[b++] = (str[i] == '1');
	}
	return true;
}

SetPRetVals config_setBoolArray(const struct ConfigEntry *e, char *val, bool use_default)
{
	bool *ba = (bool *)e->parms[1].p;
	size_t sz = (size_t)e->parms[2].i;
	bool tmp_array[sz];
	bool ret = SRV_OK;
	size_t val_len = strlen(val);
	size_t expected_len = sz * 2 - 1;

	LOG_INFO("BoolArray val[%s]\n", val);
	if (val_len < expected_len)
		goto error;
	else if (val_len > expected_len)
		ret = SRV_OK_CLAMPED;

	val[expected_len] = '\0';

	if (!decode_boolArray(val, expected_len, tmp_array))
		goto error;

	memcpy(ba, tmp_array, sz);
	return ret;

error:
	if (use_default)
	{
		LOG_INFO("Error decoding bool array, using default [%s]\n", e->default_val);
		decode_boolArray(e->default_val, expected_len, ba);
	}
	return SRV_CONV_ERR;
}

/**
 * Set a value into the configuration string.
 *
 * \param e ConfigEntry.
 * \param val Configuration value to be written (0 terminated).
 * \param len Len available in the input string.
 */
SetPRetVals config_setString(const struct ConfigEntry *e, char *val, bool use_default)
{
	size_t sz = (size_t)e->parms[2].i;
	char *p = (char *)e->parms[1].p;
	bool ret = SRV_OK;

	size_t val_len = strlen(val);

	if (val_len > sz)
		ret = SRV_OK_CLAMPED;
	p[sz] = '\0';

	if (val == NULL || val_len == 0)
	{
		if (use_default)
		{
			LOG_INFO("Using default: %s\n", e->default_val);
			strncpy(p, e->default_val, sz);
		}
		return SRV_CONV_ERR;
	}

	LOG_INFO("setString, val: %s\n", val);
	strncpy(p, val, sz);
	return ret;
}


SetPRetVals config_setBool(const struct ConfigEntry *e, char *_val, bool use_default)
{
	bool *b = (bool *)e->parms[1].p;
	const char *val = _val;
	SetPRetVals ret = SRV_OK;

	do
	{
		if (strcasecmp(val, "true") == 0 ||
		    strcasecmp(val, "1") == 0 ||
		    strcasecmp(val, "on") == 0 ||
		    strcasecmp(val, "enable") == 0)
		{
			*b = true;
			break;
		}
		else if (strcasecmp(val, "false") == 0 ||
		         strcasecmp(val, "0") == 0 ||
		         strcasecmp(val, "off") == 0 ||
		         strcasecmp(val, "disable") == 0)
		{
			*b = false;
			break;
		}
		else
		{
			ret = SRV_CONV_ERR;
			if (use_default && val != e->default_val)
				val = e->default_val;
			else
				break;
		}
	}
	while (1);

	return ret;
}

bool config_set(const char *module, const char *param, const char *val)
{
	const ConfigEntry *e;
	ConfigMetadata *cfg;

	FOREACH_NODE(cfg, &config)
	{
		if (strcmp(cfg->name, module) == 0)
		{
			for (unsigned i = 0; i < cfg->tbl_size; i++)
			{
				if (strcmp(cfg->table[i].name, param) == 0)
				{
					e = &cfg->table[i];
					return e->setp(e, CONST_CAST(char *, val), false) != SRV_CONV_ERR;
				}
			}
		}
	}

	return false;
}

void config_reload(const char *module)
{
	ConfigMetadata *cfg;

	FOREACH_NODE(cfg, &config)
	{
		if (strcmp(cfg->name, module) == 0)
		{
			if (cfg->reload)
				cfg->reload();
			return;
		}
	}
}
