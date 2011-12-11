/*
 * $test$: cp bertos/cfg/cfg_kfile.h $cfgdir/
 * $test$: echo "#undef CONFIG_KFILE_GETS" >> $cfgdir/cfg_kfile.h
 * $test$: echo "#define CONFIG_KFILE_GETS 1" >> $cfgdir/cfg_kfile.h
 */

#include <cfg/test.h>
#include <emul/kfile_posix.h>

#include "config.h"

#include <string.h>

static const char ini_file[] = "./test/config.ini";
static KFilePosix kf;

static void reload(void);

DECLARE_CONF(system, reload,
	CONF_BOOL(test0, true),
	CONF_BOOL(test1, true),
	CONF_BOOL(test2, true),
	CONF_BOOL(test3, true),
	CONF_BOOL(test4, true),
	CONF_BOOL(test5, true),
	CONF_BOOL(test6, false),
	CONF_BOOL(test7, false),
	CONF_BOOL(test8, false),
	CONF_BOOL(test9, false),
	CONF_BOOL(test10, false),
	CONF_INT(int0, 0, 100, 50),
	CONF_INT(int1, 0, 100, 50),
	CONF_INT(int2, 0, 100, 50),
	CONF_INT(int3, 0, 100, 50),
	CONF_STRING(string0, 11, "zero"),
	CONF_FLOAT(float0, 0, 100, 50),
	CONF_FLOAT(float1, 5e-37, -10e-37, +10e-37)
);

static void reload(void)
{
	kprintf("Reloading:\n");
	kprintf("test0 %d\n", test0);
	kprintf("test1 %d\n", test1);
	kprintf("test2 %d\n", test2);
	kprintf("test3 %d\n", test3);
	kprintf("test4 %d\n", test4);
	kprintf("test5 %d\n", test5);

	kprintf("test6 %d\n", test6);
	kprintf("test7 %d\n", test7);
	kprintf("test8 %d\n", test8);
	kprintf("test9 %d\n", test9);
	kprintf("test10 %d\n", test10);

	kprintf("int0 %d\n", int0);
	kprintf("int1 %d\n", int1);
	kprintf("int2 %d\n", int2);
	kprintf("int3 %d\n", int3);
	kprintf("string0 %s\n", string0);
	kprintf("float0 %g\n", float0);
	kprintf("float1 %g\n", float1);
}


int config_testSetup(void)
{
	kdbg_init();
	config_init(&kf.fd);
	if (!kfile_posix_init(&kf, ini_file, "r"))
	{
		kprintf("No test file found\n");
		return -1;
	}
	config_register(&system);
	config_load(&system);
	return 0;
}


int config_testRun(void)
{
	ASSERT(test0 == false);
	ASSERT(test1 == false);
	ASSERT(test2 == false);
	ASSERT(test3 == false);
	ASSERT(test4 == false);
	ASSERT(test5 == true);
	ASSERT(test6 == true);
	ASSERT(test7 == true);
	ASSERT(test8 == true);
	ASSERT(test9 == true);
	ASSERT(test10 == false);
	ASSERT(config_set("test0", "true"));
	ASSERT(test0 == true);
	ASSERT(!config_set("test0", "erro"));
	ASSERT(test0 == true);

	ASSERT(int0 == 1);
	ASSERT(int1 == 0);
	ASSERT(int2 == 100);
	ASSERT(int3 == 50);
	ASSERT(config_set("int0", "23"));
	ASSERT(int0 == 23);
	ASSERT(!config_set("int0", "error"));
	ASSERT(int0 == 23);
	ASSERT(config_set("int0", "1200"));
	ASSERT(int0 == 100);
	ASSERT(config_set("int0", "-111"));
	ASSERT(int0 == 0);

	ASSERT(strcmp(string0, "string zero") == 0);
	ASSERT(config_set("string0", "set"));
	ASSERT(strcmp(string0, "set") == 0);
	ASSERT(config_set("string0", "very very long string"));
	ASSERT(strcmp(string0, "very very l") == 0);

	ASSERT(float0 - 2.3 < 1e-6);
	ASSERT(config_set("float0", "23"));
	ASSERT(float0 - 23 < 1e-6);
	ASSERT(!config_set("float0", "error"));
	ASSERT(float0 - 23 < 1e-6);
	ASSERT(config_set("float0", "1200"));
	ASSERT(float0 - 100 < 1e-6);
	ASSERT(config_set("float0", "-111"));
	ASSERT(float0 - 0 < 1e-6);

	ASSERT(float1 - -1.235e-37 < 1e-43);

	return 0;
}


int config_testTearDown(void)
{
	return 0;
}


TEST_MAIN(config);
