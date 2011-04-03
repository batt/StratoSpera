#
# User makefile.
# Edit this file to change compiler options and related stuff.
#

# Programmer interface configuration, see http://dev.bertos.org/wiki/ProgrammerInterface for help
aprs_decoder_PROGRAMMER_TYPE = none
aprs_decoder_PROGRAMMER_PORT = none

aprs_decoder_HOSTED = 1

# Files included by the user.
aprs_decoder_USER_CSRC = \
	$(aprs_decoder_SRC_PATH)/main.c \
	#

# Files included by the user.
aprs_decoder_USER_PCSRC = \
	#

# Files included by the user.
aprs_decoder_USER_CPPASRC = \
	#

# Files included by the user.
aprs_decoder_USER_CXXSRC = \
	#

# Files included by the user.
aprs_decoder_USER_ASRC = \
	#

# Flags included by the user.
aprs_decoder_USER_LDFLAGS = \
	-lpulse-simple \
	#

# Flags included by the user.
aprs_decoder_USER_CPPAFLAGS = \
	#

# Flags included by the user.
aprs_decoder_USER_CPPFLAGS = \
	-fno-strict-aliasing \
	-fwrapv \
	#
