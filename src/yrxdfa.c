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

/*****/

static vpv_t FA      = NULL;
static vpv_t mrgd    = NULL;
static map_t invmrgd = NULL;
static vec_t tomerge = NULL;
static usv_t marked  = NULL;

static yrxStack *ys = NULL;
static yrxStack *yn = NULL;

#define pushonce(s,x)    (s = yrxStkPushOnce(s,x))
#define resetstack(s)    (s = yrxStkReset(s))
#define pop(s)            yrxStkPop(s)
#define pushed(s,x)       yrxStkPushed(s,x)

/*****/

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
  if (a->to != 0) pushonce(yn,a->to);
  return a;
}

uint16_t yrxDFACntArcs(state_t st)
{
  if (vpvGet(FA,st) == NULL) return 0;
  return vecCnt((vec_t)(FA[st]));
}

arc_t *yrxDFAGetArc(state_t st, uint16_t arcn)
{
  arc_t *arc;

  if (arcn >= yrxDFACntArcs(st)) return NULL;
  arc = vecGet(FA[st],arcn);
  if (arc->to != 0) pushonce(yn,arc->to);
  
  return  arc; 
}

arc_t *yrxDFANextArc(void)
{
  return nextArc(0); 
}

arc_t *yrxDFAFirstArc(state_t st)
{
  nextArc(st);
  return nextArc(0); 
}

state_t yrxDFANextState(state_t st)
{
  if (st == 0) {
    resetstack(yn);
    pushonce(yn,1);
  }
  st = pop(yn);
  nextArc(st); 
  return st;
}

/****/
  /* mapping  lbl_t -> (arc_t *) */
  
typedef struct lbl2arcs {
    lbl_t  lbl;
    vpv_t  arcs;
} lbl2arcs;
  
static void l2a_clean(lbl2arcs *a)
{
  if (a->arcs) a->arcs = vpvFree(a->arcs);
}

static vec_t l2a_reset(vec_t a)
{
  uint32_t k;
  lbl2arcs *p;
  
  for (k=0; k<vecCnt(a); k++) {
    p = vecGet(a,k);
    if (p->arcs)
      vpvCnt(p->arcs) = 0;
  }
  vecCnt(a) = 0;
  return a;
}
/****/


static void add2merge(vec_t v, arc_t *a)
{
  lbl_t lintr;
  uint32_t k;
  lbl2arcs *p;
  lbl2arcs  q;

  q.lbl = a->lbl;
  q.arcs = vpvAdd(NULL,a);
  k = vecCnt(v);
  while (k-- > 0 && !yrxLblEmpty(q.lbl)) {
    p = vecGet(v,k);
    
    if (yrxLblEqual(q.lbl, p->lbl)) {
      _dbgmsg("Equal labels: %s\t", yrxLblStr(q.lbl));
      _dbgmsg("%s\n", yrxLblStr(p->lbl));
      p->arcs = vpvAppend(p->arcs,q.arcs);
      q.lbl = yrxLblEpsilon;
    }
    else {
      lintr = yrxLblIntersection(q.lbl, p->lbl);
      if (yrxLblEqual(lintr, q.lbl)) {
        _dbgmsg("q * p = q %s\t", yrxLblStr(q.lbl));
        _dbgmsg("%s\t", yrxLblStr(p->lbl));
        _dbgmsg("%s\n", yrxLblStr(lintr));
        p->lbl  = yrxLblMinus(p->lbl, lintr);
        q.arcs = vpvAppend(q.arcs, p->arcs);
      }  
      else if (yrxLblEqual(lintr, p->lbl)) {
        _dbgmsg("q * p = p %s\t", yrxLblStr(q.lbl));
        _dbgmsg("%s\t", yrxLblStr(p->lbl));
        _dbgmsg("%s\n", yrxLblStr(lintr));
        q.lbl  = yrxLblMinus(q.lbl, lintr);
        p->arcs = vpvAppend(p->arcs, q.arcs);
      }
      else if (!yrxLblEmpty(lintr)) {
        lbl2arcs *t = vecAdd(v,&q);
        _dbgmsg("q * p = t %s\t", yrxLblStr(q.lbl));
        _dbgmsg("%s\t", yrxLblStr(p->lbl));
        _dbgmsg("%s\n", yrxLblStr(lintr));
        t->lbl  = lintr;
        t->arcs = vpvAppend(usvDup(q.arcs), p->arcs);
        p->lbl  = yrxLblMinus(p->lbl,lintr);
        q.lbl  = yrxLblMinus(q.lbl,lintr); 
      }
    }
  }
  if (!yrxLblEmpty(q.lbl)) {
    p = vecAdd(v,&q);
  }
  else {
    q.arcs = vpvFree(q.arcs);
  }
}

#if 1
static tagset_t tagsunion(vpv_t arcs)
{
  tagset_t ts = NULL;
  uint32_t k;
  arc_t *arc;
  
  for (k=0; k<vpvCnt(arcs); k++) {
    arc = vpvGet(arcs,k);
    ts = yrxTagsUnion(ts,arc->tags);
  }
  return ts;
}
#endif


static tagset_t tagsintersection(vpv_t arcs)
{
  tagset_t ts = NULL;
  uint32_t k;
  arc_t *arc;
  
  for (k=0; k<vpvCnt(arcs); k++) {
    arc = vpvGet(arcs,k);
    ts = yrxTagsIntersection(ts,arc->tags);
  }
  return ts;
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
    a->tags = yrxTagsUnion(ulvDup(b->tags), tags);
  }
}

typedef struct {
  usv_t   list;
  state_t state;  
} invmrgd_t;

int invmrgd_cmp(invmrgd_t *a, invmrgd_t *b)
{
  return usvCmp(a->list , b->list);
}

static usv_t destlist(vpv_t arcs)
{
  uint32_t j;
  arc_t *arc;
  usv_t lst = NULL;
  usv_t tl;
  
  for (j=0; j < vpvCnt(arcs); j++) {
    arc = vpvGet(arcs,j);
    tl = vpvGet(mrgd,arc->to);
    if (tl == NULL)
      lst = usvAdd(lst,arc->to);
    else
      lst = usvAppend(lst,tl);
  }
  
  lst = usvUniq(lst);

  #if 0
  dbgmsg("destlist: \n");
  for (j=0; j<usvCnt(lst); j++) {
    dbgmsg("\t %d\n",lst[j]);
  }
  #endif
  
  return lst;
}

static void determinize(state_t st,uint32_t opts)
{
  uint32_t k,j;
  arc_t *arc;
  arc_t  a;
  vec_t  arclist;
  vec_t  newarcs;
  tagset_t finaltags = NULL;
  lbl2arcs *p;

  tomerge = l2a_reset(tomerge);
  
  arclist = vpvGet(FA, st);
  marked = usvSet(marked, st, st);
  k = 0;
  /* Compute which arcs need to be merged */
  while (k < vecCnt(arclist)) {
    arc = vecGet(arclist, k++);
    if (arc->lbl == yrxLblLambda) {
      /* It's a final state! */
      finaltags = yrxTagsUnion(finaltags, arc->tags);
      _dbgmsg("Got final %d (%p , %p)\n", st,arc->tags,finaltags);
    }
    else if (arc->lbl == yrxLblEpsilon) {
      _dbgmsg("Copy from %d to %d [k==%d]\n",arc->to, st,k);
      if (usvGet(marked, arc->to) != st) {
        marked = usvSet(marked, arc->to, st);
        copyarcs(arclist, arc->to, arc->tags);
      }
    }
    else if (opts > 1) {
      add2merge(tomerge, arc);
    } 
    else {
      lbl2arcs q;
      q.lbl = arc->lbl;
      q.arcs = vpvAdd(NULL,arc);
      p = vecAdd(tomerge,&q);
    }
  }
  
  /* Build the new list of arcs */
  
  newarcs = vecNew(sizeof(arc_t));
  
  if (finaltags != NULL) { /* ensure arc to 0 is the first one */
    a.to   = 0;
    a.lbl  = yrxLblLambda;
    a.tags = finaltags;
    vecAdd(newarcs,&a);
  }
  
  _dbgmsg("state: %d\n",st);
  for (k = 0; k < vecCnt(tomerge); k++) {
    p = vecGet(tomerge,k);
    a.lbl  = p->lbl;
    if (vpvCnt(p->arcs) == 1) {
      arc = p->arcs[0];
      a.to = arc->to;
      a.tags = yrxTagsDup(arc->tags);
    }
    else {
      usv_t st_mrgd;
      invmrgd_t *inv;
      invmrgd_t  invnew;
      
      st_mrgd = destlist(p->arcs);
      invnew.list = st_mrgd;
      invnew.state   = 0;
      inv = mapGetOrAdd(invmrgd,&invnew);
      
      a.tags =  NULL;
          
      if (inv->state == 0) { /* A state to be added! */
        a.tags = tagsintersection(p->arcs);
        inv->state = yrxNextState();
        mrgd = vpvSet(mrgd, inv->state, st_mrgd);
        st_mrgd = NULL;
        dbgmsg("NEW STATE: %3u -> %u = ",st,inv->state);
        for (j = 0; j < vpvCnt(p->arcs); j++) {
          arc = p->arcs[j];
          dbgmsg("%u, ",arc->to);
          yrxNFAAddarc(inv->state, arc->to,
                       yrxLblEpsilon,
                       yrxTagsIncrement(
                          yrxTagsDifference(
                            yrxTagsDup(arc->tags), a.tags)));
        }
        dbgmsg("\n");
      }
      else if (inv->state == st) {
        a.tags = tagsintersection(p->arcs);
        inv->state = yrxNextState();
        mrgd = vpvSet(mrgd, inv->state, st_mrgd);
        st_mrgd = NULL;
        dbgmsg("LOOP STATE: %3u -> %u  \n",st,inv->state);
        
        yrxNFAAddarc(inv->state, st,
                     yrxLblEpsilon,
                         yrxTagsIncrement(
                             yrxTagsDifference(tagsunion(p->arcs),
                                               a.tags)));
      } 
      else {
        a.tags = tagsintersection(p->arcs);
        dbgmsg("OLD STATE: %3u -> %u \n",st, inv->state);
      }
      
      a.to = inv->state;
      st_mrgd = usvFree(st_mrgd);
    }
    vecAdd(newarcs,&a); 
    pushonce(ys,a.to);
  }
  arclist = vecFree(arclist);
  FA[st] = newarcs;

}

static void fixdfa()
{
  arc_t *arc;
  state_t st;

  for (st = 1; st < vpvCnt(FA); st++) {
    if (!pushed(ys,st)) { 
      FA[st] = vecFree(FA[st]);
    }
    else if (FA[st]!= NULL) {
      arc = vecGet(FA[st],0);
      if (arc != NULL && arc->lbl == yrxLblLambda) {
        arc->tags = yrxTagsDecrement(arc->tags);
      }
    }
  }
}

static void stepgraph(uint32_t n)
{
  char step_fname[16];
  FILE *f = NULL;
  
  sprintf(step_fname,"x_%05d.dot",n);
  f = fopen(step_fname,"w");
  if (f == NULL) err(771,"Can't open file");
  yrxGraph(f,0);
  fclose(f);  
}

void yrxDFA(uint32_t opts)
{
  state_t st;
  uint32_t k;
  int step = -1;
  
  if (opts == 0) return;
  
  if (mrgd == NULL) mrgd = vpvNew();
  if (mrgd == NULL) err(702,yrxStrNoMem);
  
  if (marked == NULL) marked = usvNew();
  if (marked == NULL) err(712,yrxStrNoMem);
  
  if (tomerge == NULL) tomerge = vecNew(sizeof(lbl2arcs));
  if (tomerge == NULL) err(713,yrxStrNoMem);
      
  if (invmrgd == NULL) invmrgd = mapNew(sizeof(invmrgd_t),
                                                 (mapCmp_t)invmrgd_cmp);
  if (invmrgd == NULL) err(715,yrxStrNoMem);
 
  if (opts == 5) {
    step = 0;
    stepgraph(step++);
    opts = 2;
  }
  
  dbgmsg ("DFA opt:%u\n",opts);    
   
  resetstack(ys);
  pushonce(ys,1);
 
  while ((st = pop(ys)) != 0) {
    determinize(st,opts);
    if (step > 0) stepgraph(step++);
  } 
  
  /* cleanup unreachable states      */
  /* fix tags on lambda transitions  */
  /* renumber states (TODO)          */
  /* compute equivalent states (TODO)*/
  fixdfa();
  if (step > 0) stepgraph(step++);
  
  if (mrgd != NULL) {
    for (k=0; k < vpvCnt(mrgd); k++) {
      if (mrgd[k] != NULL) mrgd[k] = usvFree(mrgd[k]);
    }
    mrgd = vpvFree(mrgd);
  }
  
  marked = usvFree(marked);
  tomerge = vecFreeClean(tomerge, (vecCleaner)l2a_clean);
  invmrgd = mapFree(invmrgd);
  
  ys = yrxStkFree(ys);
}



/*****/

void yrxNFAAddarc(state_t from, state_t to, lbl_t l, tagset_t tags)
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
  arc.tags = tags;

  vecAdd(arclist,&arc);
}

void yrxDFAClean(void)
{
  uint32_t k;
  
  _dbgmsg ("Clean FA\n");
  if (FA != NULL) {
    for (k=0; k < vpvCnt(FA); k++) {
      if (FA[k] != NULL) FA[k] = vecFree(FA[k]);
    }
    FA = vpvFree(FA);
  }
  yrxNCP = ucvFree(yrxNCP);
  ys = yrxStkFree(yn);
  yn = yrxStkFree(ys);
}

void yrxDFAInit(void)
{  
  if (FA == NULL) FA = vpvNew();
  if (FA == NULL) err(701,yrxStrNoMem);
}

