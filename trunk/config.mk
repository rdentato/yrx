#  (C) 2007 by Remo Dentato (rdentato@users.sourceforge.net)
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
## Set up your platform type

#SYS=LINUX
SYS=MINGW

#### LUA libraries
## PATH to the lua source code distribution. It is assumed that Lua 
## libraries and executable have already been built.

LUA=/d/0_Works/lua-5.1.2

#### UPX 
UPX=test
## If you have UPX installed and want srlua to be compressed with 
## UPX then uncomment next line.

UPX=upx

#########
######### NO Configurable items below this line
#########

ifeq ($(SYS),MINGW)
_EXE=.exe
endif

_LIB=.a
_OBJ=o

AR=ar rcu 

