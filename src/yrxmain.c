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

#include "yrx.h"

void usage(void)
{
	fprintf(stderr,"Usage: yrx [opt] rx1 [rx2 ... rx250]\n ");
	fprintf(stderr,"  opt: -a         generate ASM code\n ");
	fprintf(stderr,"       -C         generate C code\n ");
	fprintf(stderr,"       -g         generate DOT graph\n ");
	fprintf(stderr,"       -o fname   set output file\n ");
	fprintf(stderr,"       -Ox        set optimization level\n ");
  exit(1);
}

#define DO_DOT  0x00000001
#define DO_ASM  0x00000002
#define DO_C    0x00000003

static uint8_t optlvl = 1;

int main(int argc, char **argv)
{
  int argn = 1;
  int rxn = 0;
  char *tmp = NULL;
  char **rxs = NULL;
  
  uint32_t to_do = DO_DOT;

  yrxInit();
  atexit(yrxCleanup);
  
  while (argn < argc) {
    if (argv[argn][0] != '-') break;
    if (argv[argn][1] == '-') break;
    
    switch (argv[argn][1]) {
      case 'g': to_do = DO_DOT; break;
      case 'a': to_do = DO_ASM; break;
      case 'C': to_do = DO_C  ; break;
      
      case 'O': if (argv[argn][2]) optlvl = argv[argn][2] - '0';
                break;
                
      case 'o': if (argv[argn][2])
                  tmp = argv[argn]+2;
                else if (argn < (argc +1)) {
                  tmp = argv[++argn];
                }
                if ((tmp == NULL) || 
                    ((yrxFileOut = fopen(tmp,"w")) == NULL))
                  err(923,"Unable to open output file");
                break;
      
    }
    argn++; 
  }
  
  if (rxs == NULL) {
    rxs = argv + argn;
    rxn = argc - argn;
  }
  
  if (rxn < 1 || 250 < rxn) usage();

  yrxParse(rxs, rxn);
  yrxDFA();
  switch (to_do & 0x03) {
     case DO_DOT: yrxGraph(0); break;
     case DO_ASM: yrxASM(optlvl);   break;
     case DO_C:   yrxC(optlvl);     break;
  }
  
  exit(0);
}

