
include ../../config.mk

CFL = -I../../src
LFL = -L../../src
RXC =../../src/rxc

CFLAGS = -I. $(PROFILE) $(DEBUG) $(CFL)

mpad.c: mpad.rxc
	$(RXC) mpad.rxc > mpad.c
	
mpad$(_EXE): mpad.o
	$(LN)mpad$(_EXE) mpad.o -lrx     

clean:
	rm -f *.o
	rm -f mpad.c
	rm -f mpad$(_EXE)
	