#   C) 2007 by Remo Dentato (rdentato@users.sourceforge.net)
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

# This makefile is for GNU tool chain it has been tested under
# Windows (Mingw) and Linux (Slackware)

# Modify this file according your system before building yrx

#### SYSTEM
## Set up your platform type if the autodetect doesn't work for your

SYS=LINUX
SYS=MINGW
SYS=$(shell sh -c 'if [ `uname | grep -i -c mingw` =  '1' ]; then echo MINGW; else echo LINUX; fi' )

#### LOG2_ASM
## Uncomment next line to use the inline assembler version of |llog2()|
LOG2_ASM=-DLOG2_ASM


#### DEBUG
## Uncomment next line to compile a debugging instrumented code
DEBUG=-DDEBUG

#### LUA libraries
## PATH to the lua source code distribution. It is assumed that Lua
## libraries and executable have already been built.

# LUA=/d/0_Works/lua-5.1.2

#### UPX
UPX=test
## If you want to compress the final executable with
## UPX, just uncomment next line.
#UPX=upx

#########
######### NO Configurable items below this line
#########

ifeq ($(SYS),MINGW)
_EXE=.exe
endif

_LIB=.a
_OBJ=o

AR=ar rcu

