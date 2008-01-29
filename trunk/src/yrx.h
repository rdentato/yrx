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

tagset_t yrxTagset           (tag_t tag);
tagset_t yrxTagsFree         (tagset_t a);
tagset_t yrxTagsUnion        (tagset_t a, tagset_t b);
tagset_t yrxTagsIntersection (tagset_t a, tagset_t b);
tagset_t yrxTagsDifference   (tagset_t a, tagset_t b);
int      yrxTagsEmpty        (tagset_t a);
uint8_t *yrxTagsStr          (tagset_t a);
tagset_t yrxTagsIncrement    (tagset_t a);

#define yrxTagsDup ulvDup

#define yrxTagMaxCapt  25

/*****************************/
typedef uint16_t   lbl_bits[16];
typedef uint16_t  *lbl_t;

lbl_t    yrxLblEpsilon;
lbl_t    yrxLblLambda;

lbl_t    yrxLabel(char *l);
int      yrxLblEqual(lbl_t a, lbl_t b);
lbl_t    yrxLblMinus(lbl_t a, lbl_t b);
lbl_t    yrxLblIntersection(lbl_t a, lbl_t b);
lbl_t    yrxLblUnion(lbl_t a, lbl_t b);

void     yrxLblInit(void);
void     yrxLblClean(void);

char    *yrxLblStr(lbl_t lb);
uint8_t *yrxLblPairs(lbl_t lb);
int      yrxLblEmpty(lbl_t b);

/*****************************/

typedef struct {
  lbl_t    lbl;
  tagset_t tags;
  state_t  to;
} arc_t;

void     yrxDFAClean(void);
void     yrxDFAInit(void);

void     yrxNFAAddarc(state_t from, state_t to, lbl_t l, tagset_t tags);
void     yrxDFA();

arc_t   *yrxDFAFirstArc(state_t st);
arc_t   *yrxDFANextArc(void);
arc_t   *yrxDFAGetArc(state_t st, uint16_t arcn);
vec_t    yrxDFAArcs(state_t from);
uint16_t yrxDFACntArcs(state_t st);

state_t  yrxDFANextState(state_t st);

#define  yrxDFAStartState() yrxDFANextState(0);

/*****************************/
void yrxParse(char **rxs, int rxn);
void yrxParseErr(int errn, char *errmsg);
state_t yrxNextState(void);


/*****************************/
void yrxDump(uint8_t fmt);

/*****************************/
void yrxASMClean(void);
void yrxASMInit(void);

void yrxASM(int optimize);
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

#endif /* YRX_H */
