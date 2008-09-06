#  YRX - makefile
#  (C) 2006 by Remo Dentato
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
#


PROFILE=-pg -g
PROFILE=

CFLAGS = -I../src $(PROFILE) -Wall

# .** Cross platform macros

_EXE=

ifeq ("${OSTYPE}","msys")
_EXE=.exe
endif

_OBJ=o
_LIB=a

RM=rm -f
RMDIR=rm -rf
CP=cp
CAT=cat

CC=gcc 
LN=gcc $(PROFILE) -L ../src -o 
AR=ar rcu 

.SUFFIXES: .yc .c .$(_OBJ)

.yc.c:
	../src/yrx $*.yc >$*.c

ut$(_EXE): ut.$(_OBJ)
	$(LN)ut ut.$(_OBJ) -lyrx

rxiter$(_EXE): rxiter.o
	$(LN)rxiter rxiter.o -lrx
		
clean:
	$(RM) *.$(_OBJ)
	$(RM) ut$(_EXE)
	$(RM) gmon.out

		