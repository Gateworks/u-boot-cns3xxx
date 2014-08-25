#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
#
# SPDX-License-Identifier: GPL-2.0+
#

PLATFORM_RELFLAGS += -fno-strict-aliasing  -fno-common -ffixed-r8

ifeq ($(BIG_ENDIAN), y)
PLATFORM_RELFLAGS += -mbig-endian
PLATFORM_CPPFLAGS += -mbig-endian
endif

PLATFORM_CPPFLAGS += -march=armv5te
