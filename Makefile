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
	cd utils; make
	cd src; make
	
clean:
	cd utils; make clean
	cd src; make clean
