#
# User makefile.
# Edit this file to change compiler options and related stuff.
#

# Programmer interface configuration, see http://dev.bertos.org/wiki/ProgrammerInterface for help
bsm-radio_PROGRAMMER_TYPE = jtag-tiny
bsm-radio_PROGRAMMER_PORT = none

# Files included by the user.
bsm-radio_USER_CSRC = \
	$(bsm-radio_SRC_PATH)/main.c \
	$(bsm-radio_HW_PATH)/hw/hw_cc1101.c \
	#

# Files included by the user.
bsm-radio_USER_PCSRC = \
	#

# Files included by the user.
bsm-radio_USER_CPPASRC = \
	#

# Files included by the user.
bsm-radio_USER_CXXSRC = \
	#

# Files included by the user.
bsm-radio_USER_ASRC = \
	#

# Flags included by the user.
bsm-radio_USER_LDFLAGS = \
	#

# Flags included by the user.
bsm-radio_USER_CPPAFLAGS = \
	#

# Flags included by the user.
bsm-radio_USER_CPPFLAGS = \
	-fno-strict-aliasing \
	-fwrapv \
	#
