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

#include "yrxlib.h"

void usage(void)
{
	fprintf(stderr,"Usage: yrx [opt] rx1 [rx2 ... rx250]\n ");
	fprintf(stderr,"  opt: -a         generate ASM code\n ");
	fprintf(stderr,"       -C[:fn]    generate the C function fn()\n ");
	fprintf(stderr,"       [-g]       generate DOT graph (default)\n ");
	fprintf(stderr,"       -o fname   set output file\n ");
	fprintf(stderr,"       -Ox        set optimization level\n ");
  exit(1);
}

#define DO_DOT  0x00000001
#define DO_ASM  0x00000002
#define DO_C    0x00000003

#define setdo(x,y)  x = ((x) & ~0xFF) | ((y) & 0xFF)
#define setopt(x,y) x = ((x) & 0xFF)  | ((y) << 8)

int main(int argc, char **argv)
{
  int argn = 1;
  int rxn = 0;
  char *tmp = NULL;
  char **rxs = NULL;
  char *fn = NULL;
  
  uint32_t to_do = DO_DOT;

  setopt(to_do,2);

  yrxInit();
  atexit(yrxCleanup);
  
  while (argn < argc) {
    if (argv[argn][0] != '-') break;
    if (argv[argn][1] == '-') break;
    
    switch (argv[argn][1]) {

      case 'g': to_do = DO_DOT; 
                setopt(to_do,2);
                break;
                     
      case 'a': to_do = DO_ASM;
                setopt(to_do,1);
                break;
      
      case 'C': to_do = DO_C  ; 
                setopt(to_do,1);
                if (argv[argn][2] == ':' && argv[argn][3] != '\0') {
                  fn = argv[argn]+3;
                }
                break;
      
      case 'O': if (isdigit(argv[argn][2]))
                   setopt(to_do, argv[argn][2] - '0');
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
  
  switch (to_do & 0xFF) {
     case DO_DOT: yrxDFA(to_do >> 8);
                  yrxGraph(0);
                  break;
                  
     case DO_ASM: yrxDFA(2);
                  yrxASM(to_do >> 8);
                  break;
                  
     case DO_C:   yrxDFA(2);
                  yrxC(to_do >> 8,fn);
                  break;
  }
  
  exit(0);
}

