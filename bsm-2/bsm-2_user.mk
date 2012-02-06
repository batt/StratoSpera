#
# User makefile.
# Edit this file to change compiler options and related stuff.
#

# Programmer interface configuration, see http://dev.bertos.org/wiki/ProgrammerInterface for help
bsm-2_PROGRAMMER_TYPE = jtag-tiny
bsm-2_PROGRAMMER_PORT = none

# Files included by the user.
bsm-2_USER_CSRC = \
	$(bsm-2_SRC_PATH)/main.c \
	$(bsm-2_SRC_PATH)/gps.c \
	$(bsm-2_SRC_PATH)/adc_mgr.c \
	$(bsm-2_SRC_PATH)/sensors.c \
	$(bsm-2_SRC_PATH)/cutoff.c \
	$(bsm-2_SRC_PATH)/logging.c \
	$(bsm-2_SRC_PATH)/measures.c \
	$(bsm-2_SRC_PATH)/hadarp.c \
	$(bsm-2_SRC_PATH)/status_mgr.c \
	$(bsm-2_SRC_PATH)/landing_buz.c \
	$(bsm-2_SRC_PATH)/radio.c \
	$(bsm-2_SRC_PATH)/testmode.c \
	$(bsm-2_SRC_PATH)/uplink.c \
	bertos/mware/ini_reader.c \
	bertos/mware/config.c \
	bertos/drv/kbd.c \
	bertos/kern/signal.c \
	bertos/kern/sem.c \
	bertos/cpu/arm/drv/spi_dma_at91.c \
	bertos/io/kblock.c \
	bertos/fs/fatfs/diskio.c \
	bertos/verstag.c \
	bertos/drv/i2c.c \
	bertos/drv/i2c_bitbang.c \
	bertos/drv/lm75.c \
	bertos/drv/pwm.c \
	bertos/drv/mma845x.c \
	bertos/cpu/arm/drv/pwm_at91.c \
	#

# Files included by the user.
bsm-2_USER_PCSRC = \
	#

# Files included by the user.
bsm-2_USER_CPPASRC = \
	#

# Files included by the user.
bsm-2_USER_CXXSRC = \
	#

# Files included by the user.
bsm-2_USER_ASRC = \
	#

# Flags included by the user.
bsm-2_USER_LDFLAGS = \
	-T bsm-2/at91sam7_256_rom.ld \
	#

# Flags included by the user.
bsm-2_USER_CPPAFLAGS = \
	#

# Flags included by the user.
bsm-2_USER_CPPFLAGS = \
	-O1 \
	-fno-strict-aliasing \
	-fwrapv \
	#-DDEMO_BOARD \
	#
