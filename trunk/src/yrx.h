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

#include "hul.h"
#include "vec.h"

#define state_t uint16_t
typedef uint16_t *lbl_ptr;

typedef struct {
   vec_t     states;
   map_t     lbls;
   state_t   nstates;

   vec_t     yarcs;
   uint32_t  yarcn;
} aut;

typedef struct {
  lbl_ptr   lbl;
  ulv_t     tags;
  state_t   to;
} Arc;

uint8_t *lbl_str(lbl_ptr lb);
state_t yrxNext(aut *dfa, state_t st);

#define yrxStartState(a) yrxNext(a,0);
#define yrxNextState(a)  yrxNext(a,1);

Arc *yrxGetArc(aut *dfa);
Arc *yrxIsFinal(aut *dfa, state_t st);
aut *yrxParse(char **rxs, int rxn);

#define yrxArcLabel(a)  lbl_str((a)->lbl)
char *yrxLabelChr(uint8_t c);

