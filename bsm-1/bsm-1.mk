#
# Wizard autogenerated makefile.
# DO NOT EDIT, use the bsm-1_user.mk file instead.
#

# Constants automatically defined by the selected modules
bsm-1_DEBUG = 1

# Our target application
TRG += bsm-1

bsm-1_PREFIX = "arm-none-eabi-"

bsm-1_SUFFIX = ""

bsm-1_SRC_PATH = bsm-1

bsm-1_HW_PATH = .

# Files automatically generated by the wizard. DO NOT EDIT, USE bsm-1_USER_CSRC INSTEAD!
bsm-1_WIZARD_CSRC = \
	bertos/net/afsk.c \
	bertos/mware/formatwr.c \
	bertos/cpu/arm/drv/timer_at91.c \
	bertos/net/nmea.c \
	bertos/mware/sprintf.c \
	bertos/io/kfile.c \
	bertos/cpu/arm/drv/ser_arm.c \
	bertos/algo/crc_ccitt.c \
	bertos/cpu/arm/drv/ser_at91.c \
	bertos/struct/heap.c \
	bertos/fs/fatfs/ff.c \
	bertos/drv/ser.c \
	bertos/drv/buzzer.c \
	bertos/mware/hex.c \
	bertos/net/nmeap/src/nmeap01.c \
	bertos/drv/timer.c \
	bertos/struct/kfile_fifo.c \
	bertos/kern/proc.c \
	bertos/drv/sd.c \
	bertos/mware/event.c \
	bertos/cpu/arm/drv/sysirq_at91.c \
	bertos/net/ax25.c \
	bertos/kern/monitor.c \
	bertos/cpu/arm/drv/timer_arm.c \
	bertos/fs/fat.c \
	#

# Files automatically generated by the wizard. DO NOT EDIT, USE bsm-1_USER_PCSRC INSTEAD!
bsm-1_WIZARD_PCSRC = \
	 \
	#

# Files automatically generated by the wizard. DO NOT EDIT, USE bsm-1_USER_CPPASRC INSTEAD!
bsm-1_WIZARD_CPPASRC = \
	bertos/cpu/arm/hw/switch_ctx_arm.S \
	#

# Files automatically generated by the wizard. DO NOT EDIT, USE bsm-1_USER_CXXSRC INSTEAD!
bsm-1_WIZARD_CXXSRC = \
	 \
	#

# Files automatically generated by the wizard. DO NOT EDIT, USE bsm-1_USER_ASRC INSTEAD!
bsm-1_WIZARD_ASRC = \
	 \
	#

bsm-1_CPPFLAGS = -D'CPU_FREQ=(48023000UL)' -D'ARCH=(ARCH_DEFAULT)' -D'WIZ_AUTOGEN' -I$(bsm-1_HW_PATH) -I$(bsm-1_SRC_PATH) $(bsm-1_CPU_CPPFLAGS) $(bsm-1_USER_CPPFLAGS)

# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_LDFLAGS = $(bsm-1_CPU_LDFLAGS) $(bsm-1_WIZARD_LDFLAGS) $(bsm-1_USER_LDFLAGS)

# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_CPPAFLAGS = $(bsm-1_CPU_CPPAFLAGS) $(bsm-1_WIZARD_CPPAFLAGS) $(bsm-1_USER_CPPAFLAGS)

# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_CSRC = $(bsm-1_CPU_CSRC) $(bsm-1_WIZARD_CSRC) $(bsm-1_USER_CSRC)

# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_PCSRC = $(bsm-1_CPU_PCSRC) $(bsm-1_WIZARD_PCSRC) $(bsm-1_USER_PCSRC)

# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_CPPASRC = $(bsm-1_CPU_CPPASRC) $(bsm-1_WIZARD_CPPASRC) $(bsm-1_USER_CPPASRC)

# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_CXXSRC = $(bsm-1_CPU_CXXSRC) $(bsm-1_WIZARD_CXXSRC) $(bsm-1_USER_CXXSRC)

# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_ASRC = $(bsm-1_CPU_ASRC) $(bsm-1_WIZARD_ASRC) $(bsm-1_USER_ASRC)

# CPU specific flags and options, defined in the CPU definition files.
# Automatically generated by the wizard. PLEASE DO NOT EDIT!
bsm-1_CPU_CPPASRC = bertos/cpu/arm/hw/crt_arm7tdmi.S bertos/cpu/arm/hw/vectors_at91.S
bsm-1_CPU_CPPAFLAGS = -g -gdwarf-2
bsm-1_CPU_CPPFLAGS = -O0 -g3 -gdwarf-2 -fverbose-asm -Ibertos/cpu/arm/ -D__ARM_AT91SAM7S256__
bsm-1_CPU_CSRC = bertos/cpu/arm/hw/init_at91.c
bsm-1_PROGRAMMER_CPU = at91sam7
bsm-1_CPU_LDFLAGS = -nostartfiles -Wl,--no-warn-mismatch -T bertos/cpu/arm/scripts/at91sam7_256_rom.ld
bsm-1_STOPFLASH_SCRIPT = bertos/prg_scripts/arm/stopopenocd.sh
bsm-1_CPU = arm7tdmi
bsm-1_STOPDEBUG_SCRIPT = bertos/prg_scripts/arm/stopopenocd.sh
bsm-1_DEBUG_SCRIPT = bertos/prg_scripts/arm/debug.sh
bsm-1_FLASH_SCRIPT = bertos/prg_scripts/arm/flash.sh

include $(bsm-1_SRC_PATH)/bsm-1_user.mk
