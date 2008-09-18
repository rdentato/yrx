include ../../config.mk

CFL = -I../../src
LFL = -L../../src
RXC =../../src/rxc

all: gsub_test$(_EXE) 

# 
gsub_test$(_EXE): libbstrlib.a bgsub.$(_OBJ) 
	$(LN)gsub_test$(_EXE) bgsub.$(_OBJ) -L ../../src -L. -lrx -lbstrlib

# = The bstrlib library

libbstrlib.a: bstrlib.o bstraux.o
	$(AR)libbstrlib.a bstrlib.o bstraux.o
	$(RANLIB) libbstrlib.a


# = Clean up

clean: FORCE
	$(RM) *.$(_OBJ)
	$(RM) lib*.a
	$(RM) *.tmp
	$(RM) gmon.out
	$(RM) ut_tests.h
	$(RM) *.gc??


FORCE:
	