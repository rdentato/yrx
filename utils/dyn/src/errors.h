/* Copyright (C) 2006 by Remo Dentato <rdentato@users.sourceforge.net>
** Distributed under the terms of the BSD license.
*/


#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <setjmp.h>

#define ERR_NONE     0x0000
#define ERR_NOINIT   0xFFFF
#define ERR_OUTOFMEM 0x0001

#ifndef ERRORS_C
extern jmp_buf  errjmp;
extern char    *errmsg;
extern uint16_t errcode;

extern char errbuf[128];
extern char *errstrs[];
#endif 

#define errstr(e) errstrs[e]

void errinit();
void errcleanup();

#define err(c,s) ((errcode == ERR_NOINIT?errinit():0), \
                  errmsg=(s), errcode=(uint16_t)(c), longjmp(errjmp,1))

char *errfmt(char *fmt, ... );

#endif

