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

/*
  TOC:

*/


/* = Standard |#include|s */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#define HUL_MAIN
#include "hul.h"
#include "vec.h"


/**************************************************/

#define state_t uint16_t

/**************************************************/

static const char *cur_rx;
static int         cur_pos;
static uint16_t    cur_nrx;

static uint8_t     esc;
static uint16_t    capt;

/**************************************************/

static const char *emptystr = "";

static void yrxerr(char *errmsg)
{
  err(-1,"ERROR: %s\n%5d: %s\n%*s\n",errmsg,cur_nrx,cur_rx,cur_pos+8,"*");
}

/**************************************************/

/* = Handling labels */

typedef uint16_t  lbl_t[17];
typedef uint16_t *lbl_ptr;

/**************************************************/

#define lbl_clr(b,c) (b[(c)>>4] &= ~(1<<((c) & 0xF)))

#define lbl_set(b,c) (b[(c)>>4] |=  (1<<((c) & 0xF)))

#define lbl_tst(b,c) (b[(c)>>4]  &  (1<<((c) & 0xF)))

#define lbl_zro(b)   memset(b,0,16 * sizeof(uint16_t))

#define lbl_cpy(a,b) memcpy(a,b,sizeof(lbl_t))

#define lbl_neg(b)     do {uint8_t i; for(i=0; i<16; i++) (b)[i] ^= 0xFFFF;  } while (0)
#define lbl_minus(a,b) do {uint8_t i; for(i=0; i<16; i++) (a)[i] &= ~(b)[i]; } while (0)
#define lbl_or(a,b)    do {uint8_t i; for(i=0; i<16; i++) (a)[i] |=  (b)[i]; } while (0)
#define lbl_and(a,b)   do {uint8_t i; for(i=0; i<16; i++) (a)[i] &=  (b)[i]; } while (0)

#define lbl_isempty(b) ((b == NULL) || \
                        (((b)[0] | (b)[1] | (b)[2] | (b)[3] |\
                          (b)[4] | (b)[5] | (b)[6] | (b)[7] |\
                          (b)[8] | (b)[9] | (b)[10]| (b)[11]|\
                          (b)[12]| (b)[13]| (b)[14]| (b)[15]) == 0))

#define lbl_eq(a,b)    ((a == b) ||((a != NULL) && (b != NULL) &&\
                        ((((a)[0] ^(b)[0] ) | ((a)[1] ^(b)[1] ) |\
                          ((a)[2] ^(b)[2] ) | ((a)[3] ^(b)[3] ) |\
                          ((a)[4] ^(b)[4] ) | ((a)[5] ^(b)[5] ) |\
                          ((a)[6] ^(b)[6] ) | ((a)[7] ^(b)[7] ) |\
                          ((a)[8] ^(b)[8] ) | ((a)[9] ^(b)[9] ) |\
                          ((a)[10]^(b)[10]) | ((a)[11]^(b)[11]) |\
                          ((a)[12]^(b)[12]) | ((a)[13]^(b)[13]) |\
                          ((a)[14]^(b)[14]) | ((a)[15]^(b)[15]) )) == 0))

#define lbl_type(a)  ((a)[16])

#define lbl(l)     mapAdd(fa.lbls, l);
#define lbl_init() mapNew(sizeof(lbl_t), NULL)


/**************************************************/

static int c__;

#define hex(c) ((c__=c),('0' <= c__ && c__ <= '9')? c__ - '0' :\
                        ('A' <= c__ && c__ <= 'F')? c__ - 'A' + 10:\
                        ('a' <= c__ && c__ <= 'f')? c__ - 'a' + 10: -1)\

#define oct(c) ((c__=c),('0' <= c__ && c__ <= '7')? c__ - '0' : -1)

static lbl_ptr lbl_bmp(char *s)
{
  int8_t negate = 0;
  int8_t range = 0;
  int    last = ' ';
  int    c;
  int    i = 0;
  int    h;

  static lbl_t ll;
  lbl_t l2;

  lbl_zro(ll);
  if (s && s[0]) {

    if (*s == '[') {
      s++;
      if (*s == '^') {s++; negate = 1;}
    }

    c = *s;
    while ( c != '\0' && c != ']') {
      if (c == '-')
        range = 1;
      else if (c == '.') {
        lbl_zro(ll); lbl_neg(ll); break;
      }
      else {
        if (c == '\\') {
          s++;
          if (*s == '\0') yrxerr("Unexpected \\");

          c = -1;

          lbl_zro(l2);

          switch (*s) {
            case 'h':
            case 'H': for (last='A'; last <= 'F'; last++) lbl_set(l2,last);
                      for (last='a'; last <= 'f'; last++) lbl_set(l2,last);
            case 'd':
            case 'D': for (last='0'; last <= '9'; last++) lbl_set(l2,last);
                      break;

            case 'o':
            case 'O': for (last='0'; last <= '7'; last++) lbl_set(l2,last);
                          break;

            case 'u':
            case 'U': for (last='A'; last <= 'Z'; last++) lbl_set(l2,last);
                          break;

            case 'w':
            case 'W': lbl_set(l2,'_');
                      for (last='0'; last <= '9'; last++) lbl_set(l2,last);
            case 'a':
            case 'A': for (last='A'; last <= 'Z'; last++) lbl_set(l2,last);
            case 'l':
            case 'L': for (last='a'; last <= 'z'; last++) lbl_set(l2,last);
                          break;

            case 's':
            case 'S': for (last=8; last <= 13; last++) lbl_set(l2,last);
                          lbl_set(l2,' ');
                          break;

            case 'y':
            case 'Y': lbl_set(l2,' ');lbl_set(l2,'\t');
                          break;

            case 'n': c='\n'; break;
            case 'r': c='\r'; break;
            case 'b': c='\b'; break;
            case 't': c='\t'; break;
            case 'v': c='\v'; break;
            case 'f': c='\f'; break;

            case 'x': c = 0;
                      if ((h = hex(s[1])) >= 0) {c = h; s++; }
                      if ((h = hex(s[1])) >= 0) {c = (c<<4) | h; s++;}

                      break;

            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                      c = oct(*s);
                      if ((h = oct(s[1]))>=0) {c = (c<<3) | h ; s++;}
                      if ((h = oct(s[1]))>=0) {c = (c<<3) | h ; s++;}
                      break;

            default : c = *s;

          }
        }
        if (c >= 0) {
          if (range)
            while (++last < c) lbl_set(ll,last);

          last = c;
          lbl_set(ll,last);
        }
        else {
         if (isupper(s[i])) lbl_neg(l2);
         lbl_or(ll,l2);
         lbl_zro(l2);
        }
        range = 0;
      }
      c = *++s;
    }
    if (range)  lbl_set(ll,'-');
    if (negate) lbl_neg(ll);
  }
  return ll;
}

static int lbl_rng(lbl_ptr bmp, uint16_t a)
{
  uint16_t b;

  b = a;
  if ((a & 0x0F) == 0)
    while ((a <= 255) && (bmp[a>>4] == 0)) a += 16;

  while (a <=255 && !lbl_tst(bmp,a)) a++;

  if (a > 255) return -1;
  b=a+1;
  while (b <= 255 && lbl_tst(bmp,b)) b++;
  b=b-1;
  return ((a<<8) | b);
}

static char *lbl_str(lbl_ptr lb)
{
  static char s[512];
  int a,b;
  int i=0;

  if (lb != NULL) {
    s[i++] = lbl_type(lb) | '@';

    a=0;
    while (a <= 255) {
      b = lbl_rng(lb,a);
      if (b<0) break;
      a = b >> 8;
      s[i++] = a;
      a = (b & 0xFF);
      s[i++] = a;
      a++;
    }
  }
  s[i] = '\0';
  return s;
}

static char *lbl_dumpchar(uint8_t c)
{
  static char s[16];
  if (c <= 32   || c > 126  || c == '\\' || c == '"' ||
      c == '\'' || c == '[' || c == ']'  || c == '-' ) {
    sprintf(s,"\\x%02X",c);
  }
  else {
    s[0] = c; s[1] = '\0';
  }
  return s;
}

static void lbl_dump(FILE *f, lbl_ptr lb)
{
  int a,b;

  if (lb != NULL) {
    fputc(lbl_type(lb) | '@' , f);

    a=0;
    while (a <= 255) {
      b = lbl_rng(lb,a);
      if (b<0) break;
      a = b >> 8;
      fputs(lbl_dumpchar(a),f);
      a = (b & 0xFF);
      fputs(lbl_dumpchar(a),f);
      a++;
    }
  }
}

/*************************************/

/* = Handling tags */

#define MAXCAPTURES  11

#define TAG_NONE   0
#define TAG_CE(n)  (1 << (n<<1))
#define TAG_CB(n)  (TAG_CE(n) << 1)
#define TAG_MRK    (1<<22)
#define TAG_FIN    (1<<23)

/* xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
**   nrx    ||()()() ()()()() ()()()()
**          |||                   | |
**          |||        ...         \ \__ capture 1
**          |||                     \___ capture 2
**          || \_____ capture 11
**          | \__ MRK
**           \___ FIN
*/

#define tag_code(t,n)((t) | ((n) << 24))
#define tag_nrx(t)   ((t) >> 8)

uint32_t *searchtags(uint32_t *tl, uint32_t tag)
{
  uint32_t rx;
  int k;
  uint32_t *p;

  rx = tag & 0xFF000000;
  for (k = 0, p = tl;  k < ulvCnt(tl);  k++, p++) {
    if ((*p & 0xFF000000) == rx)
      return p;
  }
  return NULL;
}

uint32_t *addtags(uint32_t *tl, uint32_t tag)
{
  uint32_t *p;

  if (tl == NULL) {
    tl = ulvNew();
    tl = ulvSet(tl, 0, tag);
  }
  else {
    if ((p = searchtags(tl,tag)) != NULL)
     *p |= tag;
    else
      tl = ulvAdd(tl,tag);
  }

  return tl;
}

uint32_t *copytags(uint32_t *tl, uint32_t *newtl)
{
  int k;

  if (newtl == NULL) return tl;

  for (k = 0; k < ulvCnt(newtl); k++) {
    tl = addtags(tl,newtl[k]);
  }
  return tl;
}

static void t_dump(FILE *f, uint32_t *tags)
{
  uint32_t  rx;
  uint32_t  p;
  uint32_t  k,j;

  if (tags != NULL) {

    fprintf(stdout," / ");

    for (k=0; k < ulvCnt(tags); k++) {
      p = ulvGet(tags,k);
      if (p != 0) {

        rx = (p & 0xFF000000) >> 24;
        if ( p & 0x00800000 ) {
          fprintf(f,"FIN_%d ",rx);
        }
        else {
          if ( p & 0x00400000 ) fprintf(f,"MRK_%d ",rx);
          for (j=0; j < MAXCAPTURES; j++) {
            if ( p & TAG_CE(j) ) fprintf(f,")%02d_%d ",j+1,rx);
            if ( p & TAG_CB(j) ) fprintf(f,"(%02d_%d ",j+1,rx);
          }
        }
      }
    }
  }
}


/**************************************************/

/* == A special flavour of stack
**  This function implements a stack of integers where values
** can be pushed only once.
**  For example in the code:
** {{
**    push(4);
**    ...
**    push(4);
** }}
** the value 4 will be pushed only the first time.
**  Values must be strictly positive as pushing 0 has the effect
** of resetting the stack.
*/

static state_t stkonce(state_t val,char op)
{
  static stk_t stack = NULL;
  static bmp_t pushed = NULL;

  if (val == 0) op = 'Z';

  switch (op) {
    case 'Z' : stack  = stkFree(stack);
               pushed = bmpFree(pushed);
               val = 0;
               break;

    case 'O' : if (stkIsEmpty(stack)) {
                 val = 0;
               }
               else {
                 val = stkTopVal(stack, state_t);
                 stkPop(stack);
               }
               break;

    case 'C' : return (0 != bmpTest(pushed,val));

    case 'H' : if (stack == NULL) {
                 stack  = stkNew(sizeof(state_t));
                 pushed = bmpNew();
               }
               if (!bmpTest(pushed,val)) {
                 stkPush(stack,&val);
                 bmpSet(pushed,val);
               }
               break;
  }

  return val;
}

/* {{ Macros to manage the stack */

/*    Reset the stack */
#define resetstack() stkonce(0,'Z')

/*    Push a value (only once!) */
#define pushonce(v)  stkonce(v,'H')

/*    Pop the element on the stack (0 if it's empty) */
#define pop()        stkonce(1,'O')

/*    Check if a value has been ever pushed in the stack */
#define pushed(v)    stkonce(v,'C')

/* }} */


/**************************************************/

typedef struct {
  lbl_ptr   lbl;
  ulv_t     tags;
  state_t   to;
} Arc;

typedef struct {
   vec_t     states;
   map_t     lbls;
   state_t   nstates;
} aut;

static aut fa;

static void addarc(state_t from, state_t to, char *l, uint32_t tag);
static state_t nextstate(void);


/**************************************************/

static void cleantags(void *a)
{
  _dbgmsg("cleantags: %p (%p)\n",a, a==NULL?a:((Arc *)a)->tags );
  if (a != NULL)
    ulvFree(((Arc *)a)->tags);
}

static void delarc(vec_t  arcs, Arc *a)
{
  Arc *b;

  b = vecGet(arcs, vecCnt(arcs)-1);

  cleantags(a);
  if (a != b) {
   *a = *b;
  }
  b->tags = NULL;
  vecCnt(arcs)--;
}

static void copyarcs(vec_t arcst, state_t fromst, uint32_t *tl)
{
  vec_t arcsf ;
  uint32_t k = 0;
  Arc *a;

  arcsf = vecGetVal(fa.states, fromst, vec_t );

  while (k < vecCnt(arcsf)) {
    a = vecGet(arcsf,k++);
    a = vecAdd(arcst,a);
    a->tags = copytags(NULL, a->tags);
    a->tags = copytags(a->tags, tl);
  }
}

static vec_t  marked;

static void removeeps(state_t from, vec_t  arcs)
{
  uint32_t k;
  Arc *a;

  k = 0;
  while (k < vecCnt(arcs)) {
    a = vecGet(arcs, k++);
    if (a->to != 0) {
      if (a->lbl == NULL) {
        dbgmsg("Copy from %d to %d\n",a->to,from);
        if (vecGetVal(marked, a->to, state_t) != from) {
          vecSet(marked, a->to, &from);
          copyarcs(arcs, a->to, a->tags);
          delarc(arcs, a);
          k--; /* undo the index increment */
        }
        else {
          dbgmsg("Already visited: %d\n",a->to);
          yrxerr("Expression contains empty closures");/**/
        }
      }
    }
  }
}

typedef struct {
  usv_t   stlist;
  state_t st;
} mrgd;

static vec_t merged;
static map_t invmrgd;

static void mrgcleanup(usv_t *sig)
{
  usvFree(*sig);
}

static state_t mergestates(state_t x, state_t y)
{
  state_t z;
  usv_t s ;
  usv_t t;
  uint16_t k;
  mrgd *m, mm;

  if (x == y) return x;

  s = usvNew();

  s = usvSet(s, 0, x);
  s = usvSet(s, 1, y);

  k = 0;
  while ( k < usvCnt(s)) {
    t = vecGetVal(merged, s[k++], usv_t);
    if (t != NULL) {
      k--;
      usvCnt(s)--;
      s[k] = s[usvCnt(s)];
      s = usvAppend(s, t);
    }
  }
  usvSort(s);
  usvUniq(s);
  m = mapGet(invmrgd, s);

  if (m == NULL) {
    z = nextstate();
    vecSet(merged, z, &s);
    mm.stlist = s;
    mm.st = z;
    mapAdd(invmrgd, &mm);
    addarc(z, x, "", 0);
    addarc(z, y, "", 0);
  }
  else {
    z = m->st;
    s = usvFree(s);
  }

  return z;
}

int mrgcmp(mrgd *a, mrgd *b)
{
  int z;
  int k;

  if (a == b) return 0;
  if (a == NULL) return -1;
  if (b == NULL) return 1;

  z = a->st - b->st;
  if (z != 0) return z;

  k = usvCnt(a->stlist);
  z = k -  usvCnt(b->stlist);
  if (z != 0) return z;

  return memcmp(a->stlist, b->stlist, k * blkU16sz);
}

static void mergearcs(state_t from, vec_t  arcs)
{
  uint32_t k, j;
  Arc *a, *b;
  lbl_t lb;

  merged = vecNew(sizeof(usv_t));
  invmrgd = mapNew(sizeof(mrgd), mrgcmp);

  dbgmsg("  ** Mergearcs state: %d \n",from);
  for (k = 0; k < vecCnt(arcs); k++) {
    a = vecGet(arcs,k);
    j = k+1;
    while (j < vecCnt(arcs)) {
      b = vecGet(arcs, j++);
    #if 10
      if (lbl_eq(a->lbl, b->lbl)) {
        a->to = mergestates(a->to, b->to);
        a->tags = copytags(a->tags, b->tags);

        delarc(arcs, b);
        #ifdef DEBUG
        dbgmsg("Arcs: %d %d (same) ",k,j-1);
        lbl_dump(stderr,a->lbl);
        dbgmsg("\n");
        #endif
        j--;
        continue;
      }

      if (a->to == 0 || b->to == 0) {
        #ifdef DEBUG
        dbgmsg("Arcs: %d %d (one is final) ",k,j-1);
        dbgmsg("\n");
        #endif

        continue;
      }

      /*  c = intersection(a,b)  */
      lbl_cpy(lb,a->lbl);
      lbl_and(lb,b->lbl);

      if (lbl_isempty(lb)) {
        #ifdef DEBUG
        dbgmsg("Arcs: %d %d (no intersection) ",k,j-1);
        lbl_dump(stderr,a->lbl);
        dbgmsg("  ");
        lbl_dump(stderr,b->lbl);
        dbgmsg("\n");
        #endif
        continue;
      }

      if (lbl_eq(lb, a->lbl)) {
        a->to = mergestates(a->to, b->to);
        a->tags = copytags(a->tags, b->tags);
        pushonce(a->to);
        lbl_cpy(lb,b->lbl);
        lbl_minus(lb,a->lbl);
        b->lbl = lbl(lb);
        #ifdef DEBUG
        dbgmsg("Arcs: %d %d (a subset of intersection) ",k,j-1);
        lbl_dump(stderr,a->lbl);
        dbgmsg("  ");
        lbl_dump(stderr,b->lbl);
        dbgmsg("\n");
        #endif
        continue;
      }

      if (lbl_eq(lb, b->lbl)) {
        b->to = mergestates(a->to, b->to);
        b->tags = copytags(b->tags, a->tags);
        pushonce(b->to);
        lbl_cpy(lb,a->lbl);
        lbl_minus(lb,b->lbl);
        a->lbl = lbl(lb);
        #ifdef DEBUG
        dbgmsg("Arcs: %d %d (b subset of intersection) ",k,j-1);
        lbl_dump(stderr,a->lbl);
        dbgmsg("  ");
        lbl_dump(stderr,b->lbl);
        dbgmsg("\n");
        #endif
        continue;
      }

      {
        Arc  arc;
        state_t tmp = 0;

        arc.lbl  = lbl(lb);
        arc.tags = copytags(NULL, a->tags);
        arc.tags = copytags(arc.tags, b->tags);
        arc.to   = mergestates(a->to, b->to);
        vecSet(marked, a->to, &tmp);
        vecSet(marked, b->to, &tmp);
        vecAdd(arcs, &arc);

        #ifdef DEBUG
        dbgmsg("Arcs: %d %d (intersection) ",k,j-1);
        lbl_dump(stderr,a->lbl);
        dbgmsg("  ");
        lbl_dump(stderr,b->lbl);
        dbgmsg("  ");
        lbl_dump(stderr,arc.lbl);
        dbgmsg("\n");
        #endif

        lbl_cpy(lb,a->lbl);
        lbl_minus(lb, b->lbl);
        a->lbl = lbl(lb);

        lbl_cpy(lb,b->lbl);
        lbl_minus(lb, a->lbl);
        b->lbl = lbl(lb);
      }
    #endif
    }
    if (a->to != 0)
      pushonce(a->to);
  }
  invmrgd = mapFree(invmrgd);
  merged = vecFreeClean(merged,(vecCleaner)mrgcleanup);
}

static void determinize()
{
  state_t from;
  vec_t *p;
  uint32_t k;
  vec_t  arcs;

  marked = vecNew(sizeof(state_t));

  resetstack();
  pushonce(1);

  while ((from = pop()) != 0) {
    vecSet(marked,from,&from);
    arcs = vecGetVal(fa.states, from, vec_t);

    if (arcs == NULL) yrxerr("Unexpected empty state");

    removeeps(from,arcs);
    mergearcs(from,arcs);
  }

  for (k = 1; k <= fa.nstates; k++) {
    _dbgmsg("State: %d unreachable: %d\n",k,!pushed(k));
    if (!pushed(k)) {
      p = vecGet(fa.states,k);
     *p = vecFreeClean(*p,cleantags);
    }
  }

  resetstack();
  marked = vecFree(marked);
}

/**************************************************/

static void addarc(state_t from, state_t to, char *l, uint32_t tag)
{
  Arc   arc;
  vec_t arcs;

  arc.to   = to;
  arc.lbl  = NULL;
  arc.tags = NULL;

  if ((tag & 0x00FFFFFF) != TAG_NONE)
    arc.tags = addtags(arc.tags,tag);

  if (l && l[0]) {
    arc.lbl = lbl_bmp(l+1);
    lbl_type(arc.lbl) = (uint16_t)(l[0]);
    arc.lbl = lbl(arc.lbl);
  }

  arcs = vecGetVal(fa.states, from, vec_t );

  if (arcs == NULL){
    arcs = vecNew(sizeof(Arc));
    vecSet(fa.states,from,&arcs);
  }

  vecAdd(arcs,&arc);
}

static state_t nextstate(void)
{
  if (fa.nstates == 65500)
    yrxerr("More than 65500 states!!");
  return ++fa.nstates ;
}

/**************************************************/
/* = Parsing expressions
*/

  /*
       <expr>     ::= <term>+

       <term>     ::= \( <expr> ( '|' <expr> )*\)[\+\-\*\?]? |
                      \\E<escaped> | \\: |
                      \[eNQI][\+\-\*\?]? |
                      <cclass>[\+\-\*\?]?

       <cclass>   ::= \[\e*\] | <escaped>

       <escaped>  ::= \\x\h?\h? | \\\o\o?\o? |
                      \\. | [^\|\*\+\-\?\(\)]

  */

static char *str =  NULL;
static int   str_len = 0;

static char *str_chk(int n)
{
  if (str == NULL || str_len < n) {
    str = realloc(str,n+4);
  }
  return str;
}

static char *str_set(int i,int j)
{
  int n;

  str = str_chk(j-i+2);

  if (str != NULL) {
    str[0] = '@';
    for (n=1; i<j;n++,i++) str[n] = cur_rx[i];
    str[n] = '\0';
  }
  return str;
}

static int peekch(int n) {
  int c;
  int i = 0;
  for(EVER) {
    c = cur_rx[cur_pos+i];
    if (c == '\0') return -1;
    if (i == n ) return c;
    i++;
  }
}

static int nextch() {
  int c;
  c = cur_rx[cur_pos];
  if (c == '\0') return -1;
  cur_pos++;
  return c;
}

static char* escaped()
{
  char * l;
  int    c;
  int    j;

  c = peekch(0);

  if ( c < 0 || c == '*' || c == '+' || c == '?' || c == '-' ||
       c == '(' || c == ')' || c == ']' || c == '[' || c == '|') {
    return NULL;
  }

  j = cur_pos;
  if (c == '\\') {
    c = nextch();
    c = nextch();
    if (c < 0) yrxerr ("Unexpected character (\\)");
    if (c == 'x') {
      c = peekch(0);
      if (isxdigit(c)) {
        c = nextch();
        c = peekch(0);
        if (isxdigit(c)) c = nextch();
      }
    }
    else if ('0' <= c && c <= '7') {
      c = peekch(0);
      if ('0' <= c && c <= '7') {
        c = nextch();
        c = peekch(0);
        if ('0' <= c && c <= '7') c = nextch();
      }
    }
  }
  else c = nextch();

  l = str_set(j,cur_pos);
  return l;
}

static char *cclass()
{
  char* l;
  int c;
  int j;

  c = peekch(0);
  if (c < 0) return NULL;
  j = cur_pos;
  if (c == '[') {
    c = nextch();
    while (c != ']') {
      c = nextch();
      if (c == '\\') c = (nextch(), 0);
      if (c < 0) yrxerr("Unterminated class");
    }
    l = str_set(j, cur_pos);
  }
  else l = escaped();

  return l;
}

static state_t expr(state_t state);

static state_t term(state_t state)
{
  state_t to,t1,alt,start;
  int c;
  uint8_t ncapt;
  char *l;

  c = peekch(0);

  if ( c < 0) return 0;

  if ( c == '(') {
    c = nextch();
    ncapt = capt++;
    if (ncapt >= MAXCAPTURES) yrxerr("Too many captures");

    start = nextstate();

    alt = nextstate();
    to  = nextstate();
    addarc(alt,to,"",tag_code(TAG_CE(ncapt),cur_nrx));

    t1 = expr(start);
    while (t1 > 0) {
      c = nextch();
      if (c != '|') break;
      addarc(t1,alt,"",TAG_NONE);
      t1 = expr(start);
    }
    addarc(t1,alt,"",TAG_NONE);
    if (c != ')') yrxerr("Unclosed capture");

    switch (peekch(0)) {
       case '?':  addarc(start,alt,"",TAG_NONE);
                  addarc(state,start,"",tag_code(TAG_CB(ncapt),cur_nrx));
                  c = nextch();
                  break;

       case '*':  addarc(alt,start,"",TAG_NONE);
                  addarc(state,alt,"",tag_code(TAG_CB(ncapt),cur_nrx));
                  c = nextch();
                  break;

       case '+':  addarc(alt,start,"",TAG_NONE);
                  addarc(state,start,"",tag_code(TAG_CB(ncapt),cur_nrx));
                  c = nextch();
                  break;

       default :  addarc(state,start,"",tag_code(TAG_CB(ncapt),cur_nrx));
                  break;
    }

    return to;
  }

  if ( c == '\\') {
    switch (peekch(1)) {
      case 'E' :  c = nextch(); c = nextch();
                  l = escaped();
                  if (l == NULL) yrxerr("Invalid escape sequence");
                  esc = l[1];  /*** BUG Need to properly handle \x and \0 ***/
                  if (esc == '\\') esc = l[2];
                  return state;

      case ':' :  to = nextstate();
                  addarc(state,to,"",tag_code(TAG_MRK,cur_nrx));
                  c = nextch(); c = nextch();
                  return to;

      case 'e' :  c = nextch(); c = nextch();
                  c = peekch(0);

                  l=str_chk(16);
                  sprintf(l,"@[^\\x%02X] @\\x%02X",esc,esc);
                  l[8]='\0';
                  if (c == '-') l[0] = '-';

                  t1 = nextstate();
                  to = nextstate();
                  addarc(state,to,l,TAG_NONE);
                  addarc(state,t1,l+9,TAG_NONE);
                  addarc(t1,to,"@.",TAG_NONE);
                  switch (c) {
                    case '-' :
                    case '*' :  addarc(state,to,"",TAG_NONE);
                    case '+' :  addarc(to,to,l,TAG_NONE);
                                addarc(to,t1,l+9,TAG_NONE);
                                c = nextch();
                                break;

                    case '?' :  addarc(state,to,"",TAG_NONE);
                                c = nextch();
                                break;
                    default  :  break;
                  }
                  return to;

      default  :  break;
    }
  }

  l = cclass();
  if (l != NULL) {
    to = nextstate();
    c = peekch(0);
    if ( c == '-') l[0] = '-';

    switch (c) {
      case '-' :
      case '*' :  addarc(state,to,"",TAG_NONE);
                  addarc(state,state,l,0);
                  c = nextch();
                  break;

      case '+' :  addarc(state,to,l,TAG_NONE);
                  addarc(to,to,l,0);
                  c = nextch();
                  break;

      case '?' :  addarc(state,to,"",TAG_NONE);
                  c = nextch();
      default  :  addarc(state,to,l,TAG_NONE);
                  break;
    }

    return  to;
  }

  return 0;
}

static state_t expr(state_t state)
{
  state_t j = state;
  do {
    state = j;
    j = term(state);
  } while ( j > 0 )  ;
  return state;
}

static state_t parse(const char *rx,uint16_t nrx)
{
  state_t state;

  cur_rx  = rx;
  cur_pos = 0;
  cur_nrx = nrx;
  esc     = '\\';
  capt    = 0;

  state =  expr(1);

  if (peekch(0) != -1) yrxerr("Unexpected character ");

  addarc(state,0,"",tag_code(TAG_FIN,cur_nrx));

  return state;
}

/*************************************/

static void statescleanup(vec_t *arcs)
{
  vecFreeClean(*arcs,cleantags);
}

static void cleantemp()
{
  if (str != NULL) free(str);
  str = NULL;
  resetstack();
  invmrgd = mapFree(invmrgd);
  merged = vecFreeClean(merged,(vecCleaner)mrgcleanup);
  marked = vecFree(marked);
}

static void closedown()
{
  fa.states = vecFreeClean(fa.states,(vecCleaner)statescleanup);
  fa.lbls  = mapFree(fa.lbls);

  cleantemp();
}

static void init()
{
  cur_pos = 0;
  cur_rx  = emptystr;

  fa.states  = vecNew(sizeof(vec_t));
  fa.lbls    = lbl_init();
  fa.nstates = 1;
  merged     = NULL;
  invmrgd    = NULL;
  marked     = NULL;

  atexit(closedown);
}

aut *yrx_parse(char **rxs, int rxn)
{
  int i;

  init();

  if (rxn > 250) yrxerr("Too many expressions (max 250)");

  for ( i = 0; i < rxn; i++) {
    parse(rxs[i],i+1);
  }
  determinize();
  cleantemp();

  return &fa;
}

/***********************************/

void dump(aut *dfa)
{
  uint32_t i,from;
  Arc *a;
  vec_t arcs;

  resetstack();
  pushonce(1);
  while ((from = pop()) != 0) {
    arcs = vecGetVal(dfa->states, from, vec_t );
    if (arcs != NULL)  {
      for (i = vecCnt(arcs), a = vecGet(arcs,0) ;
           i>0; a = vecNext(arcs),i--) {
        printf("%5d -> %-5d %p / %p  ", from, a->to, a->lbl, a->tags);
        lbl_dump(stdout,a->lbl);
        t_dump(stdout,a->tags);
        printf("\n");
        if (a->to != 0) pushonce(a->to);
      }
    }
  }
  resetstack();
}

int main(int argc, char **argv)
{
  aut *dfa;

  dfa = yrx_parse(argv+1, argc-1);
  dump(dfa);

  exit(0);
}

