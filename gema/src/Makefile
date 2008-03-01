# .T Makefile for gema and gel (Unix)
# .A Remo Dentato
# .I gmake
# $Id: Makefile,v 1.3 2003/10/22 21:36:11 rdentato Exp $
#
# .% Usage
# This Makefile is for GNU Make (tested with version 3.79) under some flavour
# of Unix.
# .l
#   [] Configure the make file properly for you system (see '?config?)
#   [] Invoke '|make| with the following variations:
#      .l desc
#         ['|make|]          equivalent to make all
#         ['|make all|]      to compile gema, gel and gua
#         ['|make test|]     to start gema and gel test suites
#         ['|make clean|]    to cleanup temporary files
#         ['|make install|]  to copy gel executable in your path.
#      ..
# ..

#----------------- CONFIG ------------------------
#  If the make you are using does not support '|include|, 
# you can create the '|conf| file with '|make config| and
# then copy it here (comment the following line if make 
# complains).

include ./conf

#-------------- END OF CONFIG --------------------

# .% Finalize macros

CCF = $(CCFLAGS) $(TRCFLAG) $(DBGFLAG)
LCCF= $(CCF) -DLUA -I$(LUAINCPATH) -L$(LUALIBPATH)
ALL = gema$(EXE) $(LUAEXE)
TEST= t_gema $(LUATEST)

# .% Object files

OBJS= reg-expr.o \
      var.o \
      util.o \
      read.o \
      match.o \
      cstream.o \
      action.o \
      gema.o \
      gema_glob.o \
      version.o

LOBJS=l_gel_bind.o \
      l_reg-expr.o \
      l_var.o \
      l_util.o \
      l_read.o \
      l_match.o \
      l_gema_glob.o \
      l_cstream.o \
      l_action.o \
      l_version.o
      
# .% Rules

start:
	make config.x
	make all

all: $(ALL)

gema$(EXE): conf.x $(OBJS)
	$(CC) $(CCF) -o gema $(OBJS) $(LIBS)

gel$(EXE): conf.x libgel.a l_gema.o
	$(CC) $(LCCF) -o gel l_gema.o -L. -lgel $(LIBS) -llua -llualib

libgel.a: conf.x $(LOBJS)
	$(AR) $@ $(LOBJS)
	$(RANLIB)

gua$(EXE): conf.x libgel.a l_gua.o
	$(CC) $(LCCF) -o gua l_gua.o -L. -lgel $(LIBS) -llua -llualib

clean:
	@rm -f ../test/tf???.* ../test/test*.out *.x
	@rm -f conf conf.bat
	@rm -f *.o libgel.a libgel.lib
	@rm -f gel$(EXE) gua$(EXE) gema$(EXE)
	@echo >conf
	@echo "Cleaned up"

# .% Install

install: $(INSTALLDIR)/bin/gema$(EXE) $(INSTALLDIR)/bin/gel$(EXE) $(INSTALLDIR)/bin/gua$(EXE)
	@echo "Gema installed"

$(INSTALLDIR)/bin/gel$(EXE): gel$(EXE)
	cp gel$(EXE) $@
	$(SHRINK)

$(INSTALLDIR)/bin/gua$(EXE): gua$(EXE)
	cp gua$(EXE) $@
	$(SHRINK)

$(INSTALLDIR)/bin/gema$(EXE): gema$(EXE)
	cp gema$(EXE) $@
	$(SHRINK)

# .% Config

conf.x: conf
	@echo "Configuration file not found. Use 'make config' first"
	@exit 1

config.x: conf
	@chmod u+x gbuild
	@./gbuild config
	@echo >conf.x
	@echo >config.x

config:
	@chmod u+x gbuild
	@./gbuild config
	@echo >conf.x
	@echo >config.x

# .% Test

test: $(TEST)

t_gema:
	@cd ../test;\
	./gematest ../src/gema$(EXE) >tfgma.res; \
	if [ -s tfgma.res ]; then \
	  echo "Test 000 [FAIL]	(Gema test suite)" ;\
	  cat tfgma.res;\
    else \
      echo "Test 000 [OK]	(Gema test suite)" ;\
    fi

t_gel:
	@cd ../test; \
	../examples/gxml tsuite gel_testsuite.xml; \
	chmod +x ./tfgel.sh; \
	./tfgel.sh

# .% Create a tarball from CVS

tarball:
	@cd .. ;\
	rm -rf tarball ;\
    mkdir tarball ;\
	cd tarball ;\
	cvs export -Dtomorrow gema ;\
	tar cvfz gema.tgz gema ;\
	mv gema.tgz ../gema_`date +%Y%m%d`.tgz ;\
	cd .. ;\
	rm -rf tarball
          
# .% File dependencies

# .%% Gema files

reg-expr.o: reg-expr.c reg-expr.h
	$(CC) -c $(CCF) -o $@ reg-expr.c

var.o: var.c main.h cstream.h util.h bool.h var.h
	$(CC) -c $(CCF) -o $@ var.c

util.o: util.c bool.h main.h util.h
	$(CC) -c $(CCF) -o $@ util.c

read.o: read.c reg-expr.h patimp.h util.h main.h bool.h cstream.h pattern.h
	$(CC) -c $(CCF) -o $@ read.c

match.o: match.c var.h patimp.h util.h main.h bool.h cstream.h pattern.h
	$(CC) -c $(CCF) -o $@ match.c

gema.o: gema.c util.h pattern.h main.h bool.h cstream.h
	$(CC) -c $(CCF) -o $@ gema.c

gema_glob.o: gema_glob.c util.h pattern.h main.h bool.h cstream.h
	$(CC) -c $(CCF) -o $@ gema_glob.c

cstream.o: cstream.c util.h main.h bool.h cstream.h
	$(CC) -c $(CCF) -o $@ cstream.c

action.o: action.c patimp.h var.h util.h main.h bool.h cstream.h pattern.h
	$(CC) -c $(CCF) -o $@ action.c

version.o: version.c
	$(CC) -c $(CCF) -o $@ version.c

# .%% Gel files

l_reg-expr.o: reg-expr.c reg-expr.h
	$(CC) -c $(LCCF) -o $@ reg-expr.c

l_var.o: var.c main.h cstream.h util.h bool.h var.h
	$(CC) -c $(LCCF) -o $@ var.c

l_util.o: util.c bool.h main.h util.h
	$(CC) -c $(LCCF) -o $@ util.c

l_read.o: read.c reg-expr.h patimp.h util.h main.h bool.h cstream.h pattern.h
	$(CC) -c $(LCCF) -o $@ read.c

l_match.o: match.c var.h patimp.h util.h main.h bool.h cstream.h pattern.h
	$(CC) -c $(LCCF) -o $@ match.c

l_gema.o: gema.c util.h pattern.h main.h bool.h cstream.h
	$(CC) -c $(LCCF) -o $@ gema.c

l_gema_glob.o: gema_glob.c util.h pattern.h main.h bool.h cstream.h
	$(CC) -c $(LCCF) -o $@ gema_glob.c

l_cstream.o: cstream.c util.h main.h bool.h cstream.h
	$(CC) -c $(LCCF) -o $@ cstream.c

l_action.o: action.c patimp.h var.h util.h main.h bool.h cstream.h pattern.h
	$(CC) -c $(LCCF) -o $@ action.c

l_version.o: version.c
	$(CC) -c $(LCCF) -o $@ version.c

l_gel_bind.o: gel_bind.c patimp.h cstream.h pattern.h
	$(CC) -c $(LCCF) -o $@ gel_bind.c

l_gua.o: gua.c
	$(CC) -c $(LCCF) -o $@ gua.c
