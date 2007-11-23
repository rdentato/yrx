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

#define  yrxArcLabel(a)  lbl_str((a)->lbl)
uint8_t *yrxArcTags  (Arc *a);
char    *yrxLabelStr (uint8_t c);
char    *yrxTagStr   (uint8_t tag, uint8_t rx);

#define TAG_CB1   0x41
#define TAG_CB2   0x42
#define TAG_CB3   0x43
#define TAG_CB4   0x44
#define TAG_CB5   0x45
#define TAG_CB6   0x46
#define TAG_CB7   0x47
#define TAG_CB8   0x48
#define TAG_CB9   0x49
#define TAG_CB10  0x4A
#define TAG_CB11  0x4B

#define TAG_CE1   0x61
#define TAG_CE2   0x62
#define TAG_CE3   0x63
#define TAG_CE4   0x64
#define TAG_CE5   0x65
#define TAG_CE6   0x66
#define TAG_CE7   0x67
#define TAG_CE8   0x68
#define TAG_CE9   0x69
#define TAG_CE10  0x6A
#define TAG_CE11  0x6B

#define TAG_MARK  '!'
#define TAG_FINAL '$'

#define DMP_PLAIN   1
#define DMP_COMPACT 2
#define DMP_CTAB    3
#define DMP_DOT     4
#define DMP_VCG     5
#define DMP_UCG     6

void yrxDump(aut *dfa, uint8_t fmt);

