/* Copyright (C) 2006 by Remo Dentato <rdentato@users.sourceforge.net> 
** Distributed under the terms of the BSD license.
*/

#define ERRORS_C
#include "errors.h"

char *errstrs[] = { "", "Out of Memory" };

#include <stdarg.h>

jmp_buf errjmp;

char     *errmsg = NULL;
uint16_t  errcode = ERR_NOINIT;
char      errbuf[128];

void errinit()
{
  if (setjmp(errjmp) != 0) {
    fprintf(stderr,"ERR: %03d %s\n",errcode,errmsg);
    /*errcleanup();*/
    exit(errcode);
  }
}

char *errfmt(char *fmt, ... )
{
  va_list ap;
  
  va_start(ap,fmt);
  vsprintf(errbuf,fmt,ap);
  
  return errbuf;
}
  
