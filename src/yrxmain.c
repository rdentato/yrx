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

static void init(void)
{
  yrxLblInit(); 
  yrxDFAInit();
  yrxASMInit();
}

static void cleanup(void)
{
  yrxLblClean(); 
  yrxDFAClean();
  yrxASMClean();  
}

#define DO_DOT  0x00000001
#define DO_ASM  0x00000002

int main(int argc, char **argv)
{
  int argn = 1;
  int rxn = 0;
  char **rxs = NULL;
  
  uint32_t to_do = DO_DOT;

  init();
  atexit(cleanup);
  
  while (argn < argc) {
    if (argv[argn][0] != '-') break;
    switch (argv[argn][1]) {
      case 'g': to_do = DO_DOT; break;
      case 'a': to_do = DO_ASM; break;
    }
    argn++; 
  }
  
  if (rxs == NULL) {
    rxs = argv + argn;
    rxn = argc - argn;
  }
  
  if (rxn < 1 || 250 < rxn) usage();

  yrxParse(rxs, rxn);
  switch (to_do) {
     case DO_DOT: yrxDump(0); break;
     case DO_ASM: yrxASM(1); break;
  }
  
  exit(0);
}

