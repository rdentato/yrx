#  (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
#
# Permission to use, copy, modify and distribute this code and
# its documentation for any purpose is hereby granted without
# fee, provided that the above copyright notice, or equivalent
# attribution acknowledgement, appears in all copies and
# supporting documentation.
# 
# Copyright holder makes no representations about the suitability
# of this software for any purpose. It is provided "as is" without
# express or implied warranty.

# = Usage
# =======
# 
#   YRX is being developed in parallel under MS Windows and Linux.
# This makefile is intended for YRX developers and maintainers who
# are supposed to use MINGW+MSYS under Windows and the GCC compiler
# under Linux. It should work as it is on any Unix box with GCC
# tools installed but this has not been tested.
#

## SYS ########################
# Define the type of system this makefile works for
SYS=LINUX

# Count on the existence of the OSTYPE variable
ifeq ($(OSTYPE),msys)
 SYS=MINGW
endif

# Or simply uncoment next line if you're using mingw
#SYS=MINGW

## BUILD VARIANTS ###############
# 
BUILD=Debug
#BUILD=Profile
#BUILD=Release

## TRACING ###############
# 
TRACE=-DTRACE

## TOOLS
UPX=upx --best
UPX=test

RM=rm -f
RMDIR=rm -rf
RANLIB=ranlib
LN=gcc $(LDFLAGS) -o
AR=ar rcu 
AWK=awk -f 


##################################
## END OF CONFIGURABLE ITEMS 
##################################

DEBUG=
PROFILE= 
CFL = -O2 -Wall

ifeq ($(BUILD),Debug)
 DEBUG=-DDEBUG
endif

# .** Cross platform macros
_EXE=

ifeq (${SYS},MINGW)
_EXE=.exe
endif

CFLAGS = -I. $(PROFILE) $(DEBUG) $(TRACE) $(CFL)
LDFLAGS = $(PROFILE) $(LFL) -L. 