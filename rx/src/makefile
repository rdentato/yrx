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

include ../config.mk

ifeq ($(BUILD),Profile)
PROFILE = -fprofile-arcs -ftest-coverage
CFL = -DUTEST  -I. -I../test
endif

.SUFFIXES:  .c .o

# = Build all
# 
all: rx$(_EXE) rxc$(_EXE)

# = The RX Utility
# 
rx$(_EXE): rx_main.o librx.a
	$(LN)rx$(_EXE) rx_main.o -lrx

# = The RX library
# ================
#   There are three source file for this library. There
# are no depedencies among them meaning that you can link
# only the ones you are interested in.
#
# ---------------  ----------------------------------------
# rx_comp.c        Compile a regular expression.
# rx_dump.c        Print a textual representation of nfa's.
# rx_exec.c        Execute a compiled nfa
# rx.h             To be included by those using librx
# rx_.h            *not* to be included! It's for internal
#                  use only!

RX_SRC  =rx_comp.c rx_exec.c rx_dump.c 
RX_OBJS =rx_comp.o rx_exec.o rx_dump.o

librx.a: $(RX_OBJS)
	$(AR)librx.a $(RX_OBJS)
	$(RANLIB) librx.a

# rxc preprocessor

rxc$(_EXE): rxc.o librx.a
	$(LN)rxc$(_EXE) rxc.o -lrx

# = Profiling

# = Unit testing
rx_test: FORCE 
	$(RM) *.o
	make UTEST="yes" rx_tst
	
rx_tst: $(RX_OBJS)
	../test/ut $(RX_SRC) > ut_tests.h
	gcc -c $(CFLAGS) ../test/utlib.c	
	$(LN)rx_test $(RX_OBJS) utlib.o -lgcov
	
gcov: FORCE
	rx_test
	gcov $(RX_SRC)

# = Clean up

clean: FORCE
	$(RM) *.o
	$(RM) librx.a
	$(RM) rx$(_EXE)
	$(RM) rx_test$(_EXE)
	$(RM) *.tmp
	$(RM) gmon.out
	$(RM) *.gc??

# = Rebuild
rebuild: clean all

FORCE:
	