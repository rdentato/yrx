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

#define HUL_MAIN

#include "yrx.h"

/**************************************************/
static const char *cur_rx;
static int         cur_pos;
static uint16_t    cur_nrx;

static uint8_t     esc;
static uint16_t    capt;

/**************************************************/

static ucv_t buf = NULL;

static ucv_t buf_chk(int n)
{
  if (buf == NULL)
    buf = ucvNew();
  if (ucvSlt(buf) < n)
    buf = ucvSet(buf,n,'\0');
  return buf;
}

static const char *emptystr = "";

static void yrxerr(char *errmsg)
{
  err(123,"ERROR: %s\n%5d: %s\n%*s\n",errmsg,cur_nrx,cur_rx,cur_pos+8,"*");
}

/**************************************************/

/* = Handling labels */

typedef uint16_t  lbl_t[17];

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

#define lbl_type(a)       ((a)[16] & 0xFF)
#define lbl_settype(a,v)  ((a)[16] = ((a)[16] & 0xFF00) | ((v) & 0xFF))

#define lbl_pos(a)        ((a)[16] >> 8)
#define lbl_setpos(a,p)   ((a)[16] = ((a)[16] & 0xFF) | (((p) & 0xFF) << 8))

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

static int lbl_rng(lbl_ptr bmp, uint16_t a, uint16_t *min, uint16_t *max)
{
  uint16_t b;

  b = a;
  if ((a & 0x0F) == 0)
    while ((a <= 255) && (bmp[a>>4] == 0)) a += 16;

  while (a <=255 && !lbl_tst(bmp,a)) a++;

  if (a > 255) return 0;
  b=a+1;
  while (b <= 255 && lbl_tst(bmp,b)) b++;
  b=b-1;

 *min = a;
 *max = b;
  return 1;
}

uint8_t *lbl_str(lbl_ptr lb)
{
  ucv_t s;
  uint16_t a,b;

  int i=0;

  s = buf_chk(512);

  if (lb != NULL) {
    s[i++] = lbl_type(lb) | '@';

    a = 0;
    while (lbl_rng(lb, a, &a, &b) != 0) {
      s[i++] = (uint8_t)a;
      s[i++] = (uint8_t)b;
      a = b +1 ;
    }
    s[i++] = 1;
  }

  s[i] = '\0';
  return s;
}

char *yrxLabelStr(uint8_t c)
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

/*************************************/

/* = Handling tags */





tagset *ts_new()
{
  return (tagset *)mapNew(sizeof(tag_t)); 
}

tagset *ts_add(tagset *ts, tag_t tg)
{

}

tagset *ts_union(tagset *tsa, tagset *tsb)
{

}

tagset *ts_intersection(tagset *tsa, tagset *tsb)
{

}

tagset *ts_minus(tagset *tsa, tagset *tsb)
{

}

tagset *ts_dup(tagset *ts)
{

}

tagset *ts_cpy(tagset *ts)
{

}












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

#define tag_code(t,n) ((t) | ((n) << 24))
#define tag_nrx(t)    ((t) & 0xFF000000)

static uint32_t *searchtags(uint32_t *tl, uint32_t tag, uint32_t *k)
{
  uint32_t rx;
  uint32_t *p;

  rx = tag_nrx(tag);
  *k = 0; p = tl;
  while (*k < ulvCnt(tl) && (tag_nrx(*p) > rx)) {
     *k++; p++; 
  }
  if ((*k < ulvCnt(tl)) && (tag_nrx(*p) == rx)) return p;
  return NULL;
}

static uint32_t *addtags(uint32_t *tl, uint32_t tag)
{
  uint32_t *p;
  uint32_t k,j;

  if (tl == NULL) {
    tl = ulvNew();
    tl = ulvSet(tl, 0, tag);
  }
  else {
    if ((p = searchtags(tl, tag, &k)) != NULL)
     *p |= tag;
    else {
      tl = ulvAdd(tl,tag);
      /* keep sorted on rx */
      j = ulvCnt(tl)-1;
      while (j > k) {
        tl[j] = tl[j-1];
        j--;
      }
      tl[k] = tag;
    }
  }

  return tl;
}

static uint32_t *uniontags(uint32_t *tl, uint32_t *newtl)
{
  int k;

  if (newtl == NULL) return tl;

  for (k = 0; k < ulvCnt(newtl); k++) {
    tl = addtags(tl,newtl[k]);
  }
  return tl;
}

static int tagscmp(ulv_t a, ulv_t b)
{
  int ret = 0;
  int k;

  if (a == b) return 0;
  if (a == NULL) return -1;
  if (b == NULL) return 1;
  k = ulvCnt(a);
  ret = k - ulvCnt(b);
  while ((k-- > 0) && (ret == 0)) {
    ret = a[k] - b[k];
  }
  return ret;
}

uint8_t *yrxArcTags(Arc *a)
{
  uint8_t   rx;
  uint8_t   j;
  uint32_t  p;
  uint16_t  k;
  uint32_t *tags;
  uint32_t  n = 0;

  tags = a->tags;
  
  if (tags != NULL) {
    for (k=0; k < ulvCnt(tags); k++) {
      p = ulvGet(tags,k);
      if (p != 0) {
        buf = buf_chk(n + 24);
        rx = (p & 0xFF000000) >> 24;
        if ( p & 0x00800000 ) {
          buf[n++] = '$';
          buf[n++] = rx;
        }
        if ( p & 0x00400000 ) {
          buf[n++] = '%';
          buf[n++] = rx;
        }
        for (j=0; j < MAXCAPTURES; j++) {
          if (p & TAG_CB(j)) {
            buf[n++] = 'A' + j;
            buf[n++] = rx;
          }
          if (p & TAG_CE(j)) {
            buf[n++] = 'a' + j;
            buf[n++] = rx;
          }
        }        
      }
    }
  }
  buf = buf_chk(n + 2);
  buf[n++] = '\0';
  buf[n] = '\0';
  return buf;
}

char *yrxTagStr(uint8_t tag, uint8_t rx)
{
  static char tbuf[16];

  if (tag >= 'a') {
    sprintf(tbuf,"%c_%d", tag, rx);
  }
  else if (tag >= 'A') {
    sprintf(tbuf,"%c_%d", tag, rx);
  }
  else {
    sprintf(tbuf,"%c%d", tag, rx);
  }

  return tbuf;
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

static Automata fa;

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
    a->tags = uniontags(NULL, a->tags);
    a->tags = uniontags(a->tags, tl);
    #ifdef xDEBUG
    _dbgmsg(" -- arc to %d ",a->to);
    lbl_dump(stderr,a->lbl);
    _dbgmsg("\n");
    #endif
  }
}

static vec_t marked;
static vec_t merged;
static map_t invmrgd;

static void removeeps(state_t from, vec_t  arcs)
{
  uint32_t k;
  Arc *a;

  k = 0;
  while (k < vecCnt(arcs)) {
    a = vecGet(arcs, k++);
    if (a->to != 0) {
      if (a->lbl == NULL) {
        _dbgmsg("Copy from %d to %d\n",a->to,from);
        if (vecGetVal(marked, a->to, state_t) != from) {
          vecSet(marked, a->to, &from);
          copyarcs(arcs, a->to, a->tags);
        }
        else {
          _dbgmsg("Already visited: %d\n",a->to);
          /*yrxerr("Expression contains empty closures");*/
        }
        delarc(arcs, a);
        k--; /* undo the index increment */
      }
    }
  }
  _dbgmsg("Copy done\n");
}

typedef struct {
  usv_t   stlist;
  state_t st;
} mrgd;

static void mrgcleanup(usv_t *sig)
{
  usvFree(*sig);
}

static state_t mergestates(state_t x, state_t y)
{
  state_t z;
  usv_t s ;
  usv_t *t;
  uint16_t k;
  mrgd *m, mm;

  dbgmsg("Merging states %d %d ",x,y);

  if (x == y) return x;

  s = usvNew();

  s = usvSet(s, 0, x);
  s = usvSet(s, 1, y);

  k = 0;
  while ( k < usvCnt(s)) {
    t = vecGet(merged, s[k++]);
    if (*t != NULL) {
      k--;
      usvCnt(s)--;
      s[k] = s[usvCnt(s)];
      s = usvAppend(s, *t);
    }
  }

  usvSort(s);
  usvUniq(s);

  #ifdef xDEBUG
  fprintf(stderr,"as: ");
  for (k = 0; k < usvCnt(s); k++) {
     fprintf(stderr,"%d ",s[k]);
  }
  #endif

  mm.st = 0;
  mm.stlist = s;
  m = mapGet(invmrgd, &mm);

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
  dbgmsg(" to: %d\n",z);
  return z;
}

int mrgcmp(mrgd *a, mrgd *b)
{
  int z;
  int k;

  if (a == b) return 0;
  if (a == NULL) return -1;
  if (b == NULL) return 1;
/*
  z = a->st - b->st;
  if (z != 0) return z;
*/
  k = usvCnt(a->stlist);
  z = k -  usvCnt(b->stlist);
  if (z != 0) return z;

  return memcmp(a->stlist, b->stlist, k * blkU16sz);
}

static void mergearcs(state_t from, vec_t  arcs)
{
  uint32_t k, j;
  Arc *a, *b;
  Arc  arc;
  lbl_t lb;

  dbgmsg("  ** Mergearcs state: %d \n",from);
  for (k = 0; k < vecCnt(arcs); k++) {
    a = vecGet(arcs,k);
    dbgmsg("      arc %d ",k);
    dbgmsg("%s \n",lbl_str(a->lbl));
    j = k+1;
    while (j < vecCnt(arcs)) {
      b = vecGet(arcs, j++);
      dbgmsg("          cmp %d %s\n",j-1,lbl_str(b->lbl));

      #if 0
      if ((a->to != 0) && (a->to == b->to) && (tagscmp(a->tags, b->tags) == 0)) {
        dbgmsg("     (same dest)\n");
        lbl_cpy(lb,a->lbl);
        lbl_or(lb,b->lbl);
        b->lbl = lbl(lb);
        delarc(arcs, a);
        k--;
        break;
      }

      if (lbl_eq(a->lbl, b->lbl)) {
        dbgmsg("     (same)\n");
        a->to = mergestates(a->to, b->to);
        a->tags = uniontags(a->tags, b->tags);

        delarc(arcs, b);
        j--;
        continue;
      }

      if (a->to == 0 || b->to == 0) {
        dbgmsg("     (one is final)\n");
        continue;
      }

      /*  c = intersection(a,b)  */
      lbl_cpy(lb,a->lbl);
      lbl_and(lb,b->lbl);

      if (lbl_isempty(lb)) {
        dbgmsg("     (no intersection)\n");
        continue;
      }

      if (lbl_eq(lb, a->lbl)) {
        dbgmsg("     (a is a subset of intersection)\n");
        a->to = mergestates(a->to, b->to);
        a->tags = uniontags(a->tags, b->tags);
        pushonce(a->to);
        lbl_cpy(lb,b->lbl);
        lbl_minus(lb,a->lbl);
        b->lbl = lbl(lb);
        continue;
      }

      if (lbl_eq(lb, b->lbl)) {
        dbgmsg("     (b is a subset of intersection)\n");
        b->to = mergestates(a->to, b->to);
        b->tags = uniontags(b->tags, a->tags);
        pushonce(b->to);
        lbl_cpy(lb,a->lbl);
        lbl_minus(lb,b->lbl);
        a->lbl = lbl(lb);
        continue;
      }

      { dbgmsg("     (intersection)\n");
        arc.lbl  = lbl(lb);
        arc.tags = uniontags(NULL, a->tags);
        arc.tags = uniontags(arc.tags, b->tags);
        arc.to   = mergestates(a->to, b->to);
        vecAdd(arcs, &arc);

        lbl_cpy(lb,a->lbl);
        lbl_minus(lb, arc.lbl);
        a->lbl = lbl(lb);

        lbl_cpy(lb,b->lbl);
        lbl_minus(lb, arc.lbl);
        b->lbl = lbl(lb);

        continue;
      }
      #endif
    }
    if (a->to != 0)
      pushonce(a->to);
    else if (k != 0) {
    /* ensure that arc to the final state is the first one in the arcs list */
      b   = vecGet(arcs, 0);
      arc = *b;
     *b   = *a;
     *a   = arc;
    }
  }
}

static void determinize(void)
{
  state_t from;
  vec_t *p;
  uint16_t k;
  vec_t  arcs;

  marked  = vecNew(sizeof(state_t));
  merged  = vecNew(sizeof(usv_t));
  invmrgd = mapNew(sizeof(mrgd), (int (*)())mrgcmp);

  resetstack();
  pushonce(1);

  while ((from = pop()) != 0) {
    vecSet(marked, from, &from);
    arcs = vecGetVal(fa.states, from, vec_t);

    if (arcs == NULL) yrxerr("Unexpected empty state");

    removeeps(from,arcs);
    mergearcs(from,arcs);
  }

  invmrgd = mapFree(invmrgd);
  merged  = vecFreeClean(merged, (vecCleaner)mrgcleanup);
  marked  = vecFree(marked);

  for (k = 1; k <= fa.nstates; k++) {
    _dbgmsg("State: %d unreachable: %d\n",k,!pushed(k));
    if (!pushed(k)) {
      p = vecGet(fa.states,k);
     *p = vecFreeClean(*p,cleantags);
    }
  }

  resetstack();
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
    lbl_settype(arc.lbl , l[0]);
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
**  Parsing is implemented as a Recursive Descent Parser
** for the following grammar:

       <expr>     ::= <term>+

       <term>     ::=   \( <expr> ( '|' <expr> )* \) [\+\-\*\?]?
                      | \\E <escaped>
                      | \\:
                      | \[eNQI] [\+\-\*\?]?
                      | <cclass> [\+\-\*\?]?

       <cclass>   ::=   \[ \e* \]
                      | <escaped>

       <escaped>  ::=   \\x\h?\h?
                      | \\\o\o?\o?
                      | \\.
                      | [^\|\*\+\-\?\(\)]

 */


static char *str_set(int i,int j)
{
  int n = 0;

  buf = buf_chk(j-i+2);
  if (buf != NULL) {
    buf[n++] = '@';
    while(i < j) {
      buf[n++] = cur_rx[i++];
    }
    buf[n] = '\0';
  }
  return (char *)buf;
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
  return 0;
}

static int nextch(void) {
  int c;
  c = cur_rx[cur_pos];
  if (c == '\0') return -1;
  cur_pos++;
  return c;
}

static char* escaped(void)
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

static char *cclass(void)
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
  uint16_t ncapt;
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
    if (c != ')') yrxerr("Unclosed capture");
    addarc(t1,alt,"",TAG_NONE);

    /* state -> (1)
    ** start -> (2)
    ** alt   -> (3)
    ** to    -> (4)
    ** t1    -> (5)
    */
    
    switch (peekch(0)) {
       case '?':  addarc(start, alt, "", TAG_NONE);
                  addarc(state, start, "", tag_code(TAG_CB(ncapt), cur_nrx));
                  c = nextch();
                  break;

       case '+':  addarc(alt, start, "", TAG_NONE);
                  addarc(state, start, "", tag_code(TAG_CB(ncapt),cur_nrx));
                  c = nextch();
                  break;

       case '*':  addarc(alt, start, "", TAG_NONE);
                  addarc(state, alt, "", tag_code(TAG_CB(ncapt), cur_nrx));
                  c = nextch();
                  break;

       default :  addarc(state, start, "", tag_code(TAG_CB(ncapt),cur_nrx));
                  break;
    }

    return to;
  }

  if ( c == '\\') {
    switch (peekch(1)) {
      case 'E' :  c = nextch(); c = nextch();
                  l = escaped();
                  if (l == NULL) yrxerr("Invalid escape sequence");
                  esc = l[1];  /*** TODO: Need to properly handle \x and \0 ***/
                  if (esc == '\\') esc = l[2];
                  return state;

      case ':' :  to = nextstate();
                  addarc(state, to, "", tag_code(TAG_MRK,cur_nrx));
                  c = nextch(); c = nextch();
                  return to;

      case 'e' :  c = nextch(); c = nextch();
                  c = peekch(0);

                  l = (char *)buf_chk(16);
                  sprintf(l,"@[^\\x%02X] @\\x%02X",esc,esc);
                  l[8]='\0';
                  if (c == '-') l[0] = '-';

                  t1 = nextstate();
                  to = nextstate();
                  addarc(state, to, l, TAG_NONE);
                  addarc(state, t1, l+9, TAG_NONE);
                  addarc(t1, to, "@.", TAG_NONE);
                  switch (c) {
                    case '-' :
                    case '*' :  addarc(state, to, "", TAG_NONE);
                    case '+' :  addarc(to, to, l, TAG_NONE);
                                addarc(to, t1, l+9, TAG_NONE);
                                c = nextch();
                                break;

                    case '?' :  addarc(state, to, "", TAG_NONE);
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
      case '*' :  addarc(state, to, "", TAG_NONE);
                  addarc(state, state, l, TAG_NONE);
                  c = nextch();
                  break;

      case '+' :  addarc(state, to, l, TAG_NONE);
                  addarc(to, to, l, TAG_NONE);
                  c = nextch();
                  break;

      case '?' :  addarc(state, to, "", TAG_NONE);
                  c = nextch();
      default  :  addarc(state, to, l, TAG_NONE);
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

  addarc(state, 0, "", tag_code(TAG_FIN, cur_nrx));

  return state;
}

/*************************************/

static void statescleanup(vec_t *arcs)
{
  vecFreeClean(*arcs,cleantags);
}

static void cleantemp(void)
{
  if (buf != NULL) buf = ucvFree(buf);
  resetstack();
  invmrgd = mapFree(invmrgd);
  merged  = vecFreeClean(merged,(vecCleaner)mrgcleanup);
  marked  = vecFree(marked);
}

static void closedown(void)
{
  fa.states = vecFreeClean(fa.states,(vecCleaner)statescleanup);
  fa.lbls   = mapFree(fa.lbls);

  cleantemp();
}

static void init(void)
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

Automata *yrxParse(char **rxs, int rxn)
{
  int i;

  init();

  if (rxn < 1 || 250 < rxn) return NULL;

  for (i = 0; i < rxn; i++) {
    parse(rxs[i],i+1);
  }
  #if 1
  determinize();
  cleantemp();
  #endif
  
  return &fa;
}

/***********************************/


state_t yrxNext(Automata *dfa, state_t st)
{
  if (st == 0) {
    resetstack();
    pushonce(1);
  }
  dfa->yarcn = 0;
  st = pop();
  dfa->yarcs = vecGetVal(dfa->states,st,vec_t);
  return st;
}

Arc *yrxGetArc(Automata *dfa)
{
  Arc *a;
  if (dfa->yarcn >= vecCnt(dfa->yarcs)) return NULL;
  a = vecGet(dfa->yarcs, dfa->yarcn++);
  if (a->to != 0) pushonce(a->to);
  return a;
}

Arc *yrxIsFinal(Automata *dfa, state_t st)
{
  Arc *a = NULL;
  vec_t arcs;

  arcs = vecGetVal(dfa->states,st,vec_t);
  if (arcs != NULL) {
    a = vecGet(arcs,0);
    if (a->to != 0) a = NULL;
  }

  return a;
}




