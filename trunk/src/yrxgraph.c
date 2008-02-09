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


static uint8_t *dmp_dotchr(uint8_t c)
{
  static char buf[8];
  
  if (c <= 32   || c > 126  || c == '\\' || c == '"' ||
      c == '\'' || c == '[' || c == ']'  || c == '-' ) {
    sprintf(buf,"\\\\x%02X",c);
  }
  else {
    buf[0] = c; buf[1] = '\0';
  }

  return buf;
}

static void dmp_dot(void)
{
  uint32_t from;
  arc_t *a;
  uint8_t *pairs;

  fprintf(yrxFileOut,"// Generated by YRX \n\n");
  fprintf(yrxFileOut,"digraph finite_state_machine {\n");
  fprintf(yrxFileOut,"\trankdir=LR;\n");

  fprintf(yrxFileOut,"\tnode [shape = circle];\n");
  fprintf(yrxFileOut,"\t0 [ style= filled, label = \"0\" ] ;\n") ;

  from = yrxDFAStartState();

  while (from != 0) {
    while ((a = yrxDFANextArc()) != NULL) {
      
      fprintf(yrxFileOut,"\t%d -> %d [ label = \"", from, a->to);
      pairs = yrxLblPairs(a->lbl);
      while (pairs[0] <= pairs[1]) {
        fprintf(yrxFileOut,"%s",dmp_dotchr(pairs[0]));
        if (pairs[0] != pairs[1]) {
          if (pairs[1] > pairs[0] +1) fprintf(yrxFileOut,"-");
          fprintf(yrxFileOut,"%s",dmp_dotchr(pairs[1]));          
        }
        pairs += 2;
      }
      
      if (a->tags) {
        fprintf(yrxFileOut," / %s",yrxTagsStr(a->tags));
      }
      fprintf(yrxFileOut,"\" ] ;\n");
    }
    from = yrxDFANextState(from);
  }
  fprintf(yrxFileOut,"}\n");
}


void yrxGraph(uint8_t fmt)
{
  dmp_dot();
}