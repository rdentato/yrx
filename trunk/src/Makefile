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

include ../config.mk

YRXLUA=yrxdfa.lua yrxgraph.lua yrxasm.lua yrxc.lua yrxl.lua 

CFLAGS = -DDEBUG

yrxcc$(_EXE):	yrxcc.o vec.o
	gcc -o yrxcc$(_EXE) yrxcc.o vec.o

yrx_l$(_EXE):	yrxexe.lua
	../utils/srlua/glue ../utils/srlua/srlua$(_EXE) yrxexe.lua yrx_l$(_EXE)
	rm yrxexe.lua
	
yrxlua.lua:	$(YRXLUA)
	luac -s -o yrxlua.lua $(YRXLUA)

yrxexe.lua:	$(YRXLUA) yrx.lua
	luac -s -o yrxexe.lua $(YRXLUA) yrx.lua

yrxclib$(_LIB): yrxclib.o
	$(AR)yrxclib.$(_LIB) yrxclib.o

	
vt$(_EXE):	vt.o vec.o
	gcc -o vt$(_EXE) vt.o vec.o

		
x$(_EXE): ../test/test.rxs yrxclib$(_LIB)
	#lua yrx.lua -cfun -o x.c -i ../test/test.rxs
	gcc -L. -O2 -o x$(_EXE) ../test/test.c yrxclib$(_LIB)

clean:
	rm -f yrxlua.lua yrxexe.lua yrx$(_EXE)
	rm -f x.*
	