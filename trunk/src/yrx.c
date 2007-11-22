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

void dump_lbl(uint8_t *l)
{
  if (*l == 0) return;

  putchar(*l++);

  while (l[0] <= l[1]) {
    printf("%s",yrxLabelStr(*l++));
    printf("%s",yrxLabelStr(*l++));
  }
}

void dump_tags(uint8_t *t)
{
  if (*t == 0) return;

  printf(" / ");

  while (t[0] != '\0') {
    printf("%s ",yrxTagStr(t[0],t[1]));
    t += 2;
  }
}



void dump(aut *dfa)
{
  uint32_t from;
  Arc *a;

  from = yrxStartState(dfa);

  while (from != 0) {
    while ((a = yrxGetArc(dfa)) != NULL) {
      printf("%5d -> %-5d %p / %p  ", from, a->to, a->lbl, a->tags);
      dump_lbl(yrxArcLabel(a));
      dump_tags(yrxArcTags(a));
      printf("\n");
    }
    from = yrxNextState(dfa);
  }
}

void usage(void)
{
	fprintf(stderr,"Usage: yrxcc rx1 [rx2 ... rx250]\n ");
  exit(1);
}

int main(int argc, char **argv)
{
  aut *dfa;

  if (argc < 2 || 251 < argc) usage();

  dfa = yrxParse(argv+1, argc-1);
  if (dfa != NULL) dump(dfa);

  exit(0);
}

