/*
**  (C) 2007 by Remo Dentato (rdentato@users.sourceforge.net)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
**
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
*/

#define HUL_MAIN
#define YRX_MAIN

#include "yrx.h"

void usage(void)
{
	fprintf(stderr,"Usage: yrxcc rx1 [rx2 ... rx250]\n ");
  exit(1);
}

static void init()
{
  yrxCleanVec = vpvNew();
  yrxCleanVec = yrxLblInit(yrxCleanVec); 
  yrxCleanVec = yrxDFAInit(yrxCleanVec);
}

static void cleanup()
{
  uint16_t k;
  
  for (k = 0; k < vpvCnt(yrxCleanVec); k++) {
     ((void (*)())(yrxCleanVec[k])) () ;
  }
}

int main(int argc, char **argv)
{
  int argn = 1;
  char **rxs;

  init();
  atexit(cleanup);
  
  while (argn < argc) {
    if (argv[argn][0] != '-') break;
    argn++; 
  }
  rxs = argv + argn;
  argn = argc - argn;

  if (argn < 1 || 250 < argn) usage();

  yrxParse(rxs, argn);

  exit(0);
}

