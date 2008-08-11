
include ../../config.mk

CFL = -I../../src
LFL = -L../../src



CFLAGS = -I. $(PROFILE) $(DEBUG) $(CFL)

mpad$(_EXE): mpad.o
	$(LN)mpad$(_EXE) mpad.o -lrx     
	

clean:
	rm -f *.o
	rm -f mpad$(_EXE)