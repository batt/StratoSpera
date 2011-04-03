#
# User makefile.
# Edit this file to change compiler options and related stuff.
#

# Programmer interface configuration, see http://dev.bertos.org/wiki/ProgrammerInterface for help
bsm-1_PROGRAMMER_TYPE = jtag-tiny
bsm-1_PROGRAMMER_PORT = none

# Files included by the user.
bsm-1_USER_CSRC = \
	$(bsm-1_SRC_PATH)/main.c \
	$(bsm-1_SRC_PATH)/gps.c \
	$(bsm-1_SRC_PATH)/adc_mgr.c \
	$(bsm-1_SRC_PATH)/sensors.c \
	$(bsm-1_SRC_PATH)/cutoff.c \
	$(bsm-1_SRC_PATH)/landing.c \
	$(bsm-1_SRC_PATH)/logging.c \
	bertos/mware/ini_reader.c \
	bertos/drv/kbd.c \
	bertos/kern/signal.c \
	bertos/kern/sem.c \
	bertos/cpu/arm/drv/spi_dma_at91.c \
	bertos/io/kblock.c \
	bertos/fs/fatfs/diskio.c \
	#

# Files included by the user.
bsm-1_USER_PCSRC = \
	#

# Files included by the user.
bsm-1_USER_CPPASRC = \
	#

# Files included by the user.
bsm-1_USER_CXXSRC = \
	#

# Files included by the user.
bsm-1_USER_ASRC = \
	#

# Flags included by the user.
bsm-1_USER_LDFLAGS = \
	#

# Flags included by the user.
bsm-1_USER_CPPAFLAGS = \
	#

# Flags included by the user.
bsm-1_USER_CPPFLAGS = \
	-O1 \
	-fno-strict-aliasing \
	-fwrapv \
	#
