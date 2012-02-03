#ifndef CONFIG_H
#define CONFIG_H

/**
 * Configuration for various modules.
 *
 * Each module may have a couple of variables
 * which are needed for normal functionality.
 * Each module declares a configuration fragment that is registered at
 * startup into a global configuration table. The fragment is composed of
 * metadata ConfigMetadata and one or more ConfigEntry, which may be a single
 * variable or an array.
 *
 * The user defines a configuration section in his *.c files with the following
 * syntax:
 * \code
 * DECLARE_CONF(acquisition_cfg, NULL, \
 * 	CONF_BOOLARRAY(acq_channels, VALID_CHANNELS, true) \
 * );
 * \endcode
 * Then the fragment must be registered with a call to config_register().
 *
 * See below for explanation of the various fields and functions.
 */

#include <cfg/for.h>
#include <cfg/macros.h>
#include <struct/list.h>
#include <io/kfile.h>

#define CFG_NAME_MAX 32
#define MAX_PARMS 4
#define MAX_VAL CFG_NAME_MAX
struct ConfigEntry;

typedef void (*ReloadFunc_t) (void);

typedef enum SetPRetVals
{
	SRV_OK,
	SRV_OK_CLAMPED,
	SRV_CONV_ERR,
} SetPRetVals;

/**
 * Prototype for the set parameter function.
 *
 * The function converts the value from a string and validates the converted
 * value with limits present into \a e.
 *
 * \param e Entry to be modified.
 * \param val String containing the value to be set.
 * \param default_on_err true if have to use default value on converion errors.
 * \return true if the value \a val should be written to DLI, false otherwise
 */
typedef SetPRetVals (*SetParamFunc_t) (const struct ConfigEntry *e, char *val, bool default_on_err);

/**
 * Metadata for a configuration fragment.
 * \param n List node
 * \param name Name of the configuration section for the module inside the DLI.
 * \param table Entries of this configuration fragment.
 * \param tbl_size Size of the table
 * \param reload Function to reload the configuration. May be NULL if not needed.
 */
typedef struct ConfigMetadata
{
	Node n;
	char name[CFG_NAME_MAX+1];
	const struct ConfigEntry *table;
	size_t tbl_size;
	ReloadFunc_t reload;
} ConfigMetadata;

typedef union ConfigParam
{
	void *p;
	int i;
	float f;
} ConfigParam;

typedef struct ConfigEntry
{
	char name[CFG_NAME_MAX+1];
	char default_val[MAX_VAL+1];
	ConfigParam parms[MAX_PARMS];
	SetParamFunc_t setp;
} ConfigEntry;

#define CONF_INT(name, min, max, def) \
	(VAR, int, name, _, PP_STRINGIZE(name), PP_STRINGIZE(def), {.i = min}, {.p = &name}, {.i = max}, {.p = NULL}, config_setInt)

#define CONF_BOOL(name, def) \
	(VAR, bool, name, _, PP_STRINGIZE(name), PP_STRINGIZE(def), {.p = NULL}, {.p = &name}, {.p = NULL}, {.p = NULL}, config_setBool)

#define CONF_BOOLARRAY(name, size, def) \
	(ARRAY, bool, name, size, PP_STRINGIZE(name), def, {.p = NULL}, {.p = name}, {.i = size}, {.p = NULL}, config_setBoolArray)

#define CONF_STRING(name, size, def) \
	(ARRAY, char, name, size, PP_STRINGIZE(name), def, {.p = NULL}, {.p = name}, {.i = size}, {.p = NULL}, config_setString)

#define CONF_FLOAT(name, min, max, def) \
	(VAR, float, name, _, PP_STRINGIZE(name), PP_STRINGIZE(def), {.f = min}, {.p = &name}, {.f = max}, {.p = NULL}, config_setFloat)

#define CONF_INT_NODECLARE(name, var, min, max, def) \
	(EMPTY, _, _, _, PP_STRINGIZE(name), PP_STRINGIZE(def), {.i = min}, {.p = &var}, {.i = max}, {.p = NULL}, config_setInt)


#define DECLARE_ARRAY(type, name, size, ...) static type name[size];
#define DECLARE_VAR(type, name, ...) static type name;
#define DECLARE_EMPTY(type, name, ...) /* NOP */
#define DECLARE(array, type, name, size, ...) IDENTITY(PP_CAT(DECLARE_, array) (type, name, size))

#define DECLARE_ENTRY(_1, _2, _3, _4, str_name, def, par0, par1, par2, par3, setp) \
	{str_name, def, {par0, par1, par2, par3}, setp},

#define DECLARE_CONF(mod_name, reload, ...) \
	/* Declare static vars */ \
	FOR(DECLARE, __VA_ARGS__) \
	/* Declare configurations table */ \
	static const struct ConfigEntry mod_name ## _table[] = { \
		FOR(DECLARE_ENTRY, __VA_ARGS__) \
	}; \
	/* Declare Metadata structure */ \
	static struct ConfigMetadata mod_name = \
	{ \
		{NULL, NULL}, \
		PP_STRINGIZE(mod_name), \
		mod_name ## _table, \
		countof(mod_name ## _table), \
		reload, \
	}; \


void config_init(KFile *fd);
void config_register(ConfigMetadata *cfg);
void config_load(ConfigMetadata *cfg);
const ConfigEntry *config_findEntry(ConfigMetadata **cfg, const char *name);
SetPRetVals config_setBoolArray(const struct ConfigEntry *e, char *val, bool use_default);
SetPRetVals config_setString(const struct ConfigEntry *e, char *val, bool use_default);
SetPRetVals config_setFloat(const struct ConfigEntry *a, char *b, bool use_default);
SetPRetVals config_setInt(const struct ConfigEntry *a, char *b, bool use_default);
SetPRetVals config_setBool(const struct ConfigEntry *a, char *b, bool use_default);
bool config_set(const char *module, const char *param, const char *val);

#endif // CONFIG_H
