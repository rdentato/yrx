/* Copyright (C) 2006 by Remo Dentato <rdentato@users.sourceforge.net>
** Distributed under the terms of the BSD license.
*/

#ifndef DBG_H
#ifdef DEBUG
static int dbg_i;  /* just to avoid warnings */
#define dbgprintf(n,fmt,...) \
               (n? fprintf(stderr,fmt,__VA_ARGS__),fflush(stderr): (dbg_i=0))
#else
#define dbgprintf(n,fmt,...)
#endif
#endif


#ifdef DBG_UTEST
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  dbgprintf(1,"%d %d",12,30);
  dbgprintf(0,"%d %d",21,22);
  dbgprintf(1,"%s %d %d %d %d"," list:",1,2,3,4);
}

#endif
