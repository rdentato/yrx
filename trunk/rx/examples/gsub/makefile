#  RX - makefile
#  (C) 2008 by Remo Dentato
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

PROFILE = 
CFL = -O2 -Wall

ifeq  ("${UTEST}","yes")
PROFILE = -fprofile-arcs -ftest-coverage
CFL = -DUTEST  -I. -I../test
endif

CFLAGS = -I. -I../../src $(PROFILE) $(CFL)

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

MAKE=make

RANLIB=ranlib

LN=$(CC) $(PROFILE) -L . -o 
AR=ar rcu 

AWK=awk -f 

RX_SRC= rx_comp.c rx_exec.c rx_dump.c 

RX_OBJS= rx_comp.$(_OBJ) rx_exec.$(_OBJ) rx_dump.$(_OBJ) rx_match.$(_OBJ)
YRX_OBJS= $(RX_OBJS) yrx_runtime.$(_OBJ)

RX_LIB=librx.$(_LIB)
YRX_LIB=libyrx.$(_LIB)

.SUFFIXES:  .c .$(_OBJ)

.yc.c:
	yrx $*.yc >$*.c

# = Build everything
# 

all: gsub_test$(_EXE) 

# 
gsub_test$(_EXE): libbstrlib.a bgsub.$(_OBJ) 
	$(LN)gsub_test$(_EXE) bgsub.$(_OBJ) -L ../../src -L. -lrx -lbstrlib

# = The bstrlib library

libbstrlib.a: bstrlib.o bstraux.o
	$(AR)libbstrlib.a bstrlib.o bstraux.o
	$(RANLIB) libbstrlib.a

# = Profiling

yrxp$(_EXE): 
	$(MAKE) clean
	$(MAKE) PROFILE="-pg" $(YRX_LIB)
	$(CC) $(CFLAGS) -pg -DPROFILE -c -o yrx_mainp.$(_OBJ) yrx_main.c
	$(LN)yrxp$(_EXE) -pg yrx_mainp.$(_OBJ) -lyrx

# = Unit testing
rx_test: FORCE 
	$(RM) *.$(_OBJ)
	make UTEST="yes" rx_tst
	
rx_tst: $(RX_OBJS)
	../test/ut $(RX_SRC) > ut_tests.h
	$(CC) -c $(CFLAGS) ../test/utlib.c	
	$(LN)rx_test $(RX_OBJS) utlib.$(_OBJ) -lgcov
	
gcov: FORCE
	rx_test
	gcov $(RX_SRC)

# = Clean up

clean: FORCE
	$(RM) *.$(_OBJ)
	$(RM) lib*.a
	$(RM) *.tmp
	$(RM) gmon.out
	$(RM) ut_tests.h
	$(RM) *.gc??


FORCE:
	