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
	fprintf(stderr,"Usage: yrxcc rx1 [rx2 ... rx250]\n ");
  exit(1);
}

int main(int argc, char **argv)
{
  aut *dfa;
  int argn = 1;
  char **rxs;


  while (argn < argc) {
    if (argv[argn][0] != '-') break;
    argn++; 
  }
  rxs = argv + argn;
  argn = argc - argn;

  if (argn < 1 || 250 < argn) usage();

  dfa = yrxParse(rxs, argn);
  if (dfa != NULL) yrxDump(dfa, DMP_DOT);

  exit(0);
}

