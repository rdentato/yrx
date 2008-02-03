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

CFLAGS = $(DEBUG) -Wall -O2 -g 

LDFLAGS = -L ../utils/lua -L.
#LIBS   = -llua -lm

OFILES = yrxparse.$(_OBJ)\
         yrxlabel.$(_OBJ)\
         yrxtags.$(_OBJ)\
         yrxdfa.$(_OBJ) \
         yrxgraph.$(_OBJ)\
         yrxcode.$(_OBJ)\
         yrxlib.$(_OBJ)

yrx$(_EXE): libyrxlib.a yrxmain.$(_OBJ) 
	gcc -o yrx$(_EXE) yrxmain.$(_OBJ) $(LDFLAGS) -lyrxlib $(LIBS) 

libyrxlib.a: $(OFILES) vec.$(_OBJ) 
	$(AR) libyrxlib.a $(OFILES) vec.$(_OBJ) 
	

vec.$(_OBJ): vec.c vec.h
	gcc $(CFLAGS) $(LOG2_ASM) -c -o vec.$(_OBJ) vec.c

vt$(_EXE):	vt.$(_OBJ) vec.$(_OBJ)
	gcc $(CFLAGS) -o vt$(_EXE) vt.$(_OBJ) vec.$(_OBJ)

clean:
	rm -f yrx$(_EXE) vt$(_EXE)
	rm -f x.*
	rm -f *.$(_OBJ)
	rm -f libyrxlib.a
	
