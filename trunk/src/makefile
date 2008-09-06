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


.SUFFIXES:
.SUFFIXES: .rxc .c .o

.rxc.o:
	rxc $*.rxc >$*.c
	gcc $(CFLAGS) -c $*.c 
	$(RM) $*.c

# = Build all
# 
all: rx$(_EXE) rxc$(_EXE) rx_test$(_EXE)

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

RX_SRC   =rx_comp.c rx_exec.c rx_dump.c 
RX_OBJS  =rx_comp.o rx_exec.o rx_dump.o
RX_POBJS =rx_comp.po rx_exec.po rx_dump.po

librx.a: $(RX_OBJS)
	$(AR)librx.a $(RX_OBJS)
	$(RANLIB) librx.a

# rxc preprocessor

rxc$(_EXE): rxc_boot$(_EXE) librx.a rxc_b.o rxc.o
	$(LN)rxc$(_EXE) rxc.o -lrx

rxc_b.o: rxc_boot$(_EXE)
	./rxc_boot <rxc.rxc >rxc_b.c
	gcc $(CFLAGS) -c rxc_b.c
	$(LN)rxc$(_EXE) rxc_b.o -lrx

rxc_boot$(_EXE): librx.a rxc_boot.o 
	$(LN)rxc_boot$(_EXE) rxc_boot.o -lrx

rx_test$(_EXE): librx.a rxc$(_EXE) rx_test.o 
	$(LN)rx_test$(_EXE) rx_test.o -lrx

# = Profiling

.SUFFIXES: .po

.c.po:
	gcc $(CFLAGS) -g -pg -c -o $*.po $*.c

.rxc.po:
	rxc $*.rxc >$*.c
	gcc $(CFLAGS) -g -pg -c -o $*.po $*.c 
	$(RM) $*.c
	

rx_prof$(_EXE): rx_test.rxc rx_test.po $(RX_POBJS)
	$(LN)rx_prof$(_EXE) $(RX_POBJS) rx_test.po -pg  
        
# = Test coverage

gcov: FORCE
	rx_test
	gcov $(RX_SRC)

# = Clean up

clean: FORCE
	$(RM) *.o
	$(RM) *.po
	$(RM) librx.a
	$(RM) rx$(_EXE)
	$(RM) rxc$(_EXE)
	$(RM) rxc_boot$(_EXE)
	$(RM) rx_test$(_EXE)
	$(RM) rx_prof$(_EXE)
	$(RM) rxc.c
	$(RM) rxc_b.c
	$(RM) rx_test.c
	$(RM) *.tmp
	$(RM) gmon.out
	$(RM) *.gc??

# = Rebuild
rebuild: clean all

FORCE:
	