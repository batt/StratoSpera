#
# User makefile.
# Edit this file to change compiler options and related stuff.
#

# Programmer interface configuration, see http://dev.bertos.org/wiki/ProgrammerInterface for help
boot_PROGRAMMER_TYPE = jtag-tiny
boot_PROGRAMMER_PORT = none

# Files included by the user.
boot_USER_CSRC = \
	$(boot_SRC_PATH)/main.c \
	bertos/cpu/arm/drv/spi_dma_at91.c \
	bertos/cpu/arm/drv/flash_at91.c \
	bertos/struct/kfile_fifo.c \
	bertos/verstag.c \
	bertos/io/kblock.c \
	bertos/fs/fatfs/diskio.c \
	#

# Files included by the user.
boot_USER_PCSRC = \
	#

# Files included by the user.
boot_USER_CPPASRC = \
	#

# Files included by the user.
boot_USER_CXXSRC = \
	#

# Files included by the user.
boot_USER_ASRC = \
	#

# Flags included by the user.
boot_USER_LDFLAGS = \
	#

# Flags included by the user.
boot_USER_CPPAFLAGS = \
	#

# Flags included by the user.
boot_USER_CPPFLAGS = \
	-Os \
	-fno-strict-aliasing \
	-fwrapv \
	#-DDEMO_BOARD \
	#
