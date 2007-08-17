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

srlua_exe:
	cd srlua; make LUA=$(LUA) "LIBS=-L$(LUA)/src -llua -lm" srlua; make glue
	strip srlua/srlua$(_EXE)
	$(UPX) srlua/srlua$(_EXE)
	
clean:
	cd srlua; make clean
	rm -f srlua/srlua$(_EXE)
	rm -f srlua/glue$(_EXE)
		