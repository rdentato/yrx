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

#ifndef YRX_H
#define YRX_H

#include "hul.h"
#include "vec.h"

#ifndef YRX_MAIN
#define EXTERN(t, v, i) extern t v
#else
#define EXTERN(t, v, i) t v i
#endif

#define state_t   uint16_t

/*****************************/

typedef uint32_t tag_t;
typedef ulv_t    tagset_t;

#define yrxTagNone   0
#define TAG_CB(n)  ('a'+n)
#define TAG_CE(n)  ('A'+n)
#define TAG_MRK    (':')
#define TAG_FIN    ('$')

#define yrxTag(t,n,d) (((n) << 24) | ((t) << 16) | (d))

#define yrxTagDelta(t) ((t) & 0x0000FFFF)
#define yrxTagType(t) (((t) >> 16) & 0x00FF)
#define yrxTagExpr(t)  ((t) >> 24)

tagset_t yrxTagset           ();
tagset_t yrxTagsFree         (tagset_t a);
tagset_t yrxTagsUnion        (tagset_t a, tagset_t b);
tagset_t yrxTagsIntersection (tagset_t a, tagset_t b);
tagset_t yrxTagsDifference   (tagset_t a, tagset_t b);
int      yrxTagsEmpty        (tagset_t a);
uint8_t *yrxTagsStr          (tagset_t a);
tagset_t yrxTagsIncrement    (tagset_t a);

/*****************************/
typedef uint16_t   lbl_bits[16];
typedef uint16_t  *lbl_t;

lbl_t    yrxLblEpsilon;
lbl_t    yrxLblLambda;

lbl_t    yrxLabel(char *l);
int      yrxLblEqual(lbl_t a, lbl_t b);
lbl_t    yrxLblDifference(lbl_t a, lbl_t b);
lbl_t    yrxLblIntersection(lbl_t a, lbl_t b);
lbl_t    yrxLblUnion(lbl_t a, lbl_t b);
vpv_t    yrxLblInit(vpv_t v);
char    *yrxLblStr(lbl_t lb);
uint8_t *yrxLblPairs(lbl_t lb);
int      yrxLblEmpty(lbl_t b);

/*****************************/

typedef struct {
  lbl_t    lbl;
  tagset_t tags;
  state_t  to;
} arc_t;

vpv_t    yrxDFAInit(vpv_t v);

void     yrxNFAAddarc(state_t from, state_t to, lbl_t l, tag_t tag);
void     yrxDFA();

arc_t   *yrxDFANextArc();
state_t  yrxDFANextState(state_t st);

#define  yrxDFAStartState() yrxDFANextState(0);

/*****************************/
void yrxParse(char **rxs, int rxn);
void yrxParseErr(int errn, char *errmsg);

/*****************************/
void yrxDump(uint8_t fmt);
/*****************************/

EXTERN(char *, yrxStrNoMem, = "Out of memory");

/**************************************************/
#define YRX_BUF_MAX 512

EXTERN(uint8_t, yrxBuf[YRX_BUF_MAX], );

#define yrxBufChk(n) ((n)<=YRX_BUF_MAX ? yrxBuf \
                                       : (err(617,yrxStrNoMem), NULL))

/*****************************/

vpv_t yrxCleanVec;

/*****************************/
#if 0
typedef struct {
   vec_t     states;
   map_t     lbls;
   state_t   nstates;

   vec_t     yarcs;
   uint32_t  yarcn;
} Automata;


typedef map_t *tagset;

typedef struct {
  lbl_ptr   lbl;
  tagset    tags;
  state_t   to;
} Arc;

uint8_t *lbl_str(lbl_ptr lb);
state_t yrxNext(Automata *dfa, state_t st);

#define yrxStartState(a) yrxNext(a,0);
#define yrxNextState(a)  yrxNext(a,1);

Arc *yrxGetArc(Automata *dfa);
Arc *yrxIsFinal(Automata *dfa, state_t st);

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

#if 0
typedef uint32_t tag_t;
typedef ulv_t    tagset_t;

TagSet addtag(TagSet ts, Tag t);
#endif 

void yrxDump(Automata *dfa, uint8_t fmt);
#endif

#endif /* YRX_H */
