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
	fprintf(stderr,"       [-g[x]]    generate DOT graph (default)\n ");
	fprintf(stderr,"       -o fname   set output file\n ");
	fprintf(stderr,"       -Ox        set optimization level\n ");
	fprintf(stderr,"                  0 stop after parsing\n ");
	fprintf(stderr,"                  1 only remove epsilon transitions\n ");
	fprintf(stderr,"                  2 full optimization\n ");
	fprintf(stderr,"                  5 create a dot file at each step\n ");
  exit(1);
}

#define DO_DOT  0x00000001
#define DO_ASM  0x00000002
#define DO_C    0x00000003

int main(int argc, char **argv)
{
  int argn = 1;
  int rxn = 0;
  char *tmp = NULL;
  char **rxs = NULL;
  char *fn = NULL;
  
  uint32_t to_do = DO_DOT;
  uint32_t opt   = 100;

  yrxInit();
  atexit(yrxCleanup);
  
  while (argn < argc) {
    if (argv[argn][0] != '-') break;
    if (argv[argn][1] == '-') {argn++ ; break; }
    
    switch (argv[argn][1]) {

      case 'g': to_do = DO_DOT; 
                if (opt == 100) opt = 2;
                break;
                     
      case 'a': to_do = DO_ASM;
                if (opt == 100) opt = 1;
                break;
      
      case 'C': to_do = DO_C  ; 
                if (opt == 100) opt = 1;
                if (argv[argn][2] == ':' && argv[argn][3] != '\0') {
                  fn = argv[argn]+3;
                }
                break;
      
      case 'O': if (isdigit(argv[argn][2]))
                   opt = argv[argn][2] - '0';
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
  if (opt == 100) opt = 2;
  
  if (rxs == NULL) {
    rxs = argv + argn;
    rxn = argc - argn;
  }
  
  if (rxn < 1 || 250 < rxn) usage();

  yrxParse(rxs, rxn);
  
  switch (to_do) {
     case DO_DOT: yrxDFA(opt);
                  if (opt <5)
                    yrxGraph(yrxFileOut, 0);
                  break;
                  
     case DO_ASM: yrxDFA(2);
                  yrxASM(opt);
                  break;
                  
     case DO_C:   yrxDFA(2);
                  yrxC(opt,fn);
                  break;
  }
  
  exit(0);
}
