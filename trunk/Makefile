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

# $Id$

# This makefile is for GNU tool chain it has been tested under 
# Windows (Mingw) and Linux (Slackware 11 and Xubuntu 7.10)


TARG=YRX

include config.mk

ifeq (x$(SYS),x)
  TARG=config
endif
  
all: $(TARG)

config:
	@echo "Edit the config.mk file before building YRX"
	
YRX:
	cd src; make
	
clean:
	cd src; make clean

	
SVN_URL=http://yrx.googlecode.com/svn/trunk
SVN_PATH=yrx
SVN_TAR=tar cvzf $(SVN_TBALL) $(SVN_PATH)
SVN_TBALL=yrx_`date +%Y%m%d`.tgz 
ifeq ($(SYS),MINGW)
SVN_CMD = $(TSVN) /command:export /url:$(SVN_URL) /path:$(SVN_PATH) /notempfile /closeonend
else
SVN_CMD = svn export $(SVN_URL) $(SVN_PATH)
endif

tarball:
	rm -rf $(SVN_PATH)
	mkdir $(SVN_PATH)
	$(SVN_CMD)
	rm -f $(SVN_TBALL)
	$(SVN_TAR)
#	rm -rf yrx
