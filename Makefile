#
# Copyright 2009 Develer S.r.l. (http://www.develer.com/)
# All rights reserved.
#
# Author: Lorenzo Berni <duplo@develer.com>
#

# Set to 1 for verbose build output, 0 for terse output
V := 0

default: all

include bertos/config.mk

include aprs_decoder/aprs_decoder.mk
include boot/boot.mk
include bsm-2/bsm-2.mk

include bertos/rules.mk
