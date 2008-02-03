/*
**  (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
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


#include <stdio.h>

#include "yrx.h"

#define yrxPosType unsigned char *
#define yrxStream  unsigned char *
#define yrxPos(y)  (y)
#define yrxGet(y)  ((((y)==NULL) || *(y) == '\0')? -1 : *(y)++)

#include "x.c"

int main(int argc , char **argv)
{
  int k;
  yrxResult *y;
  if (argc > 1) {
    y = yrxExec(argv[1]);
    printf("MATCH: %d LEN: %d\n",yrxMatch(y),yrxLen(y,0));
    for (k=0; k < yrxNumCapt(y); k++) {
      printf("%6u: [%u] \"",k,yrxLen(y,k));
      yrxPuts(y,k);
      printf("\"\n");      
    }
  }
}

