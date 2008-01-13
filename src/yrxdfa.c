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

/*****/


static vpv_t   FA = NULL;

/*****/


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
  static usv_t stack  = NULL;
  static bit_t pushed = NULL;

  if (val == 0) op = 'Z';

  switch (op) {
    case 'Z' : stack  = usvFree(stack);
               pushed = bitFree(pushed);
               val = 0;
               break;

    case 'O' : val = usvPop(stack);
               break;

    case 'C' : return (0 != bitTest(pushed,val));

    case 'H' : if (stack == NULL) {
                 stack  = usvNew();
                 pushed = bitNew();
               }
               if (!bitTest(pushed,val)) {
                 stack  = usvPush(stack,val);
                 pushed = bitSet(pushed,val);
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

/**************/

static arc_t *nextArc(state_t st)
{
  static uint32_t curarcn;
  static state_t  curst;

  arc_t *a = NULL;

  if (st != 0) {
    curst = st;
    curarcn = 0;
    return NULL;
  }
    
  if ( curst == 0 || FA == NULL ||
       vpvCnt(FA) < curst ||
       FA[curst] == NULL ||
       curarcn >= vecCnt((vec_t)(FA[curst])) )
    return NULL;
  
  a = vecGet(FA[curst],curarcn++);
  if (a->to != 0) pushonce(a->to);
  return a;
}


arc_t *yrxDFANextArc()
{
  return nextArc(0); 
}

state_t yrxDFANextState(state_t st)
{
  if (st == 0) {
    resetstack();
    pushonce(1);
  }
  st = pop();
  nextArc(st); 
  return st;
}


/****/
  /* mapping  lbl_t -> (arc_t *) */
  
typedef struct lbl2arcp {
    lbl_t  lbl;
    arc_t *arcs;
} lbl2arcp;
  
/****/


map_t add2merge(vec_t v, arc_t *a)
{
  lbl_t lintr;
  uint32_t k;
  lbl2arcp *p;

  lintr = yrxLblIntersection(a->lbl, b->lbl);
  if (!yrxLblEmpty(lintr)) {
    dbgmsg(" %s\n", yrxLblStr(a->lbl));
    dbgmsg(" %s\n", yrxLblStr(b->lbl));
    dbgmsg(" %s\n", yrxLblStr(lintr));
  }
  return m;
}

/****/

static void copyarcs(vec_t arcs, state_t st, tagset_t tags)
{
  vec_t arcsf ;
  uint32_t k;
  arc_t *a;
  arc_t *b;

  arcsf = vpvGet(FA, st);
  k = 0;
  while (k < vecCnt(arcsf)) {
    b = vecGet(arcsf,k++);
    a = vecAdd(arcs,b);
    a->tags = yrxTagsUnion(b->tags, tags);
  }
}

static void delarc(vec_t  arcs, arc_t *a)
{
  arc_t *b;
  _dbgmsg("Deleted  -> %d\n", a->to);
  b = vecGet(arcs, vecCnt(arcs)-1);

  yrxTagsFree(a->tags);
  if (a != b) {
   *a = *b;
  }
  b->tags = NULL;
  vecCnt(arcs)--;
}

static vec_t marked;
static void determinize(state_t st)
{
  uint32_t k,j;
  arc_t *arc;
  arc_t *arc_tmp;
  vec_t  arclist;
  tagset_t  finaltags = NULL;
  vec_t  tomerge = NULL;

  arclist = vpvGet(FA, st);
  
  tomerge = vecFreeClean(tomerge,);
  tomerge = vecNew(sizeof(lbl2arcp));
  if (tomerge == NULL) err(713,yrxStrNoMem);
  
  vecSetVal(marked, st, st, state_t);
  k = 0;
  while (k < vecCnt(arclist)) {
    arc = vecGet(arclist, k++);
    if (arc->lbl == yrxLblLambda) {
      /* It's a final state! */
      finaltags = yrxTagsUnion(finaltags, arc->tags);
      _dbgmsg("Got final %d (%p , %p)\n", st,arc->tags,finaltags);
      delarc(arclist, arc);
      k--;
    }
    else if (arc->lbl == yrxLblEpsilon) {
      _dbgmsg("Copy from %d to %d [k==%d]\n",arc->to, st,k);
      if (vecGetVal(marked, arc->to, state_t) != st) {
        vecSetVal(marked, arc->to, st, state_t);
        copyarcs(arclist, arc->to, arc->tags);
      }
      delarc(arclist, arc);
      k--; /* undo the index increment */
    }
    else {
      add2merge(tomerge, arc);
      pushonce(arc->to);
    } 
  }
  if (finaltags != NULL) { /* ensure arc to 0 is the first one! */
    arc_t  a;
    
    if (vecCnt(arclist) > 0) {
      arc = vecGet(arclist, 0);
      a = *arc;
    }
    arc = &a;

    arc->to   = 0;
    arc->lbl  = yrxLabel("");
    arc->tags = finaltags;
    
    _dbgmsg("Fixed final %d\n", st);
    vecAdd(arclist,arc);
  }
}

void yrxDFA()
{
 state_t st;
 _dbgmsg ("DFA\n");    
 
 if (marked == NULL) 
   marked = vecNew(sizeof(state_t));
 
 if (marked == NULL) err(712,yrxStrNoMem);
    
 resetstack();
 pushonce(1);
 
 while ((st = pop()) != 0) {
   determinize(st);
 } 
}



/*****/

void yrxNFAAddarc(state_t from, state_t to, lbl_t l, tag_t tag)
{
  arc_t arc;
  vec_t arclist;
  
  arclist = vpvGet(FA,from);
  
  if (arclist == NULL) {
    arclist = vecNew(sizeof(arc_t));
    if (arclist == NULL) err(702,yrxStrNoMem);
    FA = vpvSet(FA,from,arclist);
  }
  
  arc.to   = to;
  arc.lbl  = l;
  arc.tags = yrxTagset(tag);

  vecAdd(arclist,&arc);
  
  _dbgmsg("nfa_addarc(%d, %d, \"%s\", %08X)\n",from,to,yrxLblStr(l),tag);
}

static void yrxDFAClean()
{
  uint32_t k;
  
  _dbgmsg ("Clean FA\n");
  if (FA != NULL) {
    for (k=0; k < ulvCnt(FA); k++) {
      if (FA[k] != NULL) FA[k] = vecFree(FA[k]);
    }
    FA = vpvFree(FA);
  }
}


vpv_t yrxDFAInit(vpv_t v)
{
  _dbgmsg ("Init FA\n");
  v = vpvAdd(v,yrxDFAClean);
  
  if (FA == NULL) FA = vpvNew();
  if (FA == NULL) err(701,yrxStrNoMem);
  return v;
}

