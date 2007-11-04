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
#define HUL_UT
#include "hul.h"
#include "vec.h"

typedef struct Arc {
  uint32_t from;
  uint32_t to;
  uint32_t info[30];
} Arc;

int arccmp(Arc *a, Arc *b)
{
  int t;
  t = a->from - b->from;
  if (t == 0) t = a->to - b->to;
  _dbgmsg("arccmp : %p %p %d\n",a,b,t);
  return t;
}

#if 1

uint32_t mapdepth(mapNode *p)
{
  uint32_t mL=0,mR=0;

  if (p == NULL) return 0;
  if (mapLeft(p) == NULL && mapRight(p) == NULL) return 0;

  mL = mapdepth(mapLeft(p));
  mR = mapdepth(mapRight(p));

  if (mL > mR) return mL +1;
  return mR + 1;
}


typedef struct {
  mapNode *ptr;
  uint32_t dpt;
} mapStack_t;

uint32_t mapDepth(map_t m)
{
  mapNode *p;
  mapStack_t ms;
  mapStack_t *pms;
  stk_t depths;
  uint32_t d=0;
  uint32_t max_depth=0;

  depths = stkNew(sizeof(mapStack_t));
  if (depths == NULL) return 0;

  ms.ptr = mapRoot(m);
  ms.dpt = 1;

  stkPush(depths,&ms);

  for(EVER) {
    pms = stkTop(depths);
    if (pms == NULL) break;

    p = pms->ptr;
    d = pms->dpt;

    stkPop(depths);

    for(EVER) {
      if ((mapLeft(p) == NULL) && (mapRight(p) == NULL)) {
        if (d > max_depth) max_depth = d;
        break;
      }
      d++;
      if (mapLeft(p) != NULL) {
        if (mapRight(p) != NULL) { /* save it for later */
          ms.ptr = mapRight(p);
          ms.dpt = d;
          stkPush(depths,&ms);
        }
        p = mapLeft(p);
      }
      else
        p = mapRight(p);
    }
  }
  stkFree(depths);
  return max_depth;
}

void mapTree__(mapNode *p)
{

  if (p != NULL) {
    TSTWRITE("%d",((Arc *)(p->elem))->from);
    if (mapLeft(p) != NULL || mapRight(p) != NULL) {
      TSTWRITE(" (");
      mapTree__(mapLeft(p));
      TSTWRITE(") (");
      mapTree__(mapRight(p));
      TSTWRITE(")");
    }
  }
}

void mapTree(map_t m)
{
  TSTWRITE("\n# TREE: ");
  mapTree__(m->root);
  TSTWRITE("\n# COUNT: %u DEPTH: %u  (%u)\n",mapCnt(m),mapDepth(m),mapMaxDepth(m));
}

#endif


void arccleanup(Arc *a)
{
  dbgmsg("**** CLEANUP: %d->%d\n",a->from,a->to);
}
#include "time.h"

int main(int argc, char * argv[])
{
  vec_t v;

  uint32_t j,k,n=0;
  Arc a,*p;
  uint32_t t;
  Arc *q;

  srand(time(0));

 #if 1
  TSTSECTION("Basic vec");

  TSTGROUP("Creating vectors");

  /**************************/
  TSTNOTE("Just create and destroy a vector;");
  v=vecNew(sizeof(Arc));
  TST("New vector",(v != NULL));

  v=vecFree(v); /* Always use this idiom when freeing a vector */
  TST("Free vector",(v == NULL));

  /**************************/
  TSTNOTE("Elements in a vector are supposed to be big enough to contain a");
  TSTNOTE("generic pointer. If a smaller size is specified, the size is");
  TSTNOTE("enlarged enough to ensure a pointer will fit.");

  v = vecNew(1);
  TSTT("New vector (elemsz 0 -> sizeof(void*))",
         (v != NULL && v->esz == sizeof(void *)),"Re-define free list management");

  v = vecFree(v);

  TSTGROUP("Adding elements");

  v = vecNew(sizeof(Arc));

  a.from = 10; a.to = 32;

  TSTNOTE("Let's start with an empty vector.");
  TST("vecCnt == 0",(vecCnt(v) == 0));

  TSTNOTE("Now a single element stored in position 0.");
  p = vecSet(v, 0, &a);
  TST("vecCnt == 1",(vecCnt(v) == 1));
  TST("cur_p  == 0",(v->cur_p == 0));
  TST("cur_n  == 0",(v->cur_n == 0));
  TST("cur_w  == 0",(v->cur_w == 0));

  TSTNOTE("There is only one page allocated");
  TST("v->npg == 1",(v->npg == 1 && v->pgs != NULL && v->pgs[0] != NULL));
  TSTONFAIL("Pages allocated: %d",v->npg);

  TSTNOTE("Check that elements are stored in order");
  for (k = 0; k < 32; k++) {
    a.from = k; a.to = k;
    vecSet(v,k,&a);
  }

  j = 0;
  for (k=0; k < 30; k++) {
    p = vecGet(v,k);
    TSTNOTE("  k: %2d f: %d",k,p->from);
    if (j == 0) j = k-p->from;
  }
  TST("Proper ordering",(j == 0));

  TSTGROUP("Moving sequentially");
  TSTNOTE("Let's load a few elements and then move over them");

  a.from = 16; a.to = 16;
  vecSet(v,16,&a);

  vecCnt(v) = 0;
  for (k=0;k<16;k++) {
     a.from = k;
     a.to = rand() & 0x0F;
     vecSet(v,k,&a);
  }
  TST("16 elements",(vecCnt(v) == 16));
  p = vecGet(v,12);
  TST("Selected 12",(v->cur_w == 12 && p->from == 12));

  p = vecNext(v); /* 13 */
  p = vecNext(v); /* 14 */
  TST("Selected 14",(v->cur_w == 14));
  p = vecPrev(v); /* 13 */
  p = vecPrev(v); /* 12 */
  p = vecPrev(v); /* 11 */
  TST("Selected 11",(v->cur_w == 11));
  p = vecNext(v); /* 12 */
  p = vecNext(v); /* 13 */
  p = vecNext(v); /* 14 */
  p = vecNext(v); /* 15 */
  p = vecNext(v); /* 16 */
  TST("Selected 16",(v->cur_w == 16));

  TSTNOTE("Just moving on the elements won't change the elements counter");
  TST("Still 16 elements",(vecCnt(v) == 16));
  TSTONFAIL("Count = %d",vecCnt(v));

  v = vecFreeClean(v,(vecCleaner)arccleanup);
  TSTNOTE("Next/Prev boundary checks");
  v = vecNew(sizeof(Arc));
  q = vecPrev(v);
  TST("Prev of empty is NULL", (q == NULL));
  p = vecNext(v);
  TST("Next of empty is 0", (p == vecGet(v,0)));
  q = vecPrev(v);
  TST("Prev of 0 is NULL", (q == NULL));

  v = vecFree(v);
 #endif

 #if 0
  #define HUGECNT 10000000
  TSTGROUP("Huge vector!");

  v = vecNew(sizeof(uint32_t));
  for (k=0;k < HUGECNT; k++) {
    vecSet(v,k,&k);
  }
  dbgmsg("** %u elements. Size: %u (%u)\n",vecCnt(v), vecSize(v),v->esz * vecCnt(v));
  v = vecFree(v);
 #endif

  /**********************************************************************/
 #if 0
  {
  blkNode *pn,*qn;

  TSTSECTION("Block discipline");

  TSTGROUP("Creating blocks");

  v = blkNew(sizeof(Arc));

  TSTNOTE("Blocks are allocated one after the other");
  a.from = 1;
  blkAdd(v,&a);
  a.from++;
  blkAdd(v,&a);
  TST("Highest blk 1",(vecCnt(v) == 2));
  a.from++;
  blkAdd(v,&a);
  a.from++;
  blkAdd(v,&a);
  TST("Highest blk 3",(vecCnt(v) == 4));

  TST("Free list empty",(blkFreelst(v) == NULL));
  TSTNOTE("Let get the 2nd block using the vec discipline");
  p = blkElm(vecGet(v,1));
  TST("Check 2nd blk",(p->from == 2));
  a.from++;
  blkAdd(v,&a);
  TST("Highest blk 4",(vecCnt(v) == 5));

  TSTGROUP("Delete/Recycle blocks");
  TSTNOTE("Now let's delete the 2nd Block");
  pn = vecGet(v,1);
  blkDel(v,pn);
  TST("2nd deleted",(blkFreelst(v) == pn && blkRight(pn) == NULL
                                         && blkLeft(pn) == blkDeleted) );

  qn = vecGet(v,3);
  blkDel(v,qn);
  TST("4th deleted",(blkFreelst(v) == qn && blkRight(qn) == pn
                                         && blkLeft(qn) == blkDeleted) );

  TSTNOTE("We expect to have back the last block deleted");
  a.from = 12;
  pn = blkAdd(v,&a);
  TST("4th recycled",(qn == pn && ((Arc *)blkElm(pn))->from == 12));

  TSTNOTE("And the other one still in the Free list");
  pn = vecGet(v,1);
  TST("Free list adjusted",(blkFreelst(v) == pn && blkRight(pn) == NULL
                                                && blkLeft(pn) == blkDeleted));

  a.from = 13;
  qn = blkAdd(v,&a);
  q = blkElm(qn);
  TST("2nd recycled",(qn == pn && q->from == 13));

  TST("Free list adjusted",(blkFreelst(v) == NULL));
  a.from = 16;
  pn = blkAdd(v,&a);
  TST("Highest blk 5",(vecCnt(v) == 6));
  TSTONFAIL("\tCount = %d",vecCnt(v));

  TST("Free list is empty",(blkFreelst(v) == NULL));
  TSTNOTE("Check the boundary cases for vecCnt");
  blkDel(v,vecGet(v,vecCnt(v)-1));
  TST("Last deleted",(blkFreelst(v) == pn && blkRight(pn) == NULL
                                          && vecCnt(v) == 6) );
  a.from = 17;
  qn = blkAdd(v,&a);
  q = blkElm(qn);
  TST("Last recycled",(qn == pn && q->from == 17));

  v = blkFree(v);
  }
 #endif

  /**********************************************************************/
 #if 0
  {
  Arc *p;
  TSTSECTION("List discipline");

  TSTGROUP("Creating list");

  v = lstNew(sizeof(Arc));
  TST("New list is empty",(v != NULL && lstCnt(v) == 0));

  for (k=0;k<10;k++) {
    a.from = k+10;
    lstAppend(v,&a);
  }
  TST("list has 10 elements",(v != NULL  && lstCnt(v) == 10));
  TSTONFAIL("\t lstCnt == %u",lstCnt(v));

  TSTGROUP("Iterating");

  p = lstFirst(v);
  TSTWRITE("# ");
  j = p->from;
  while (p != NULL) {
    k = p->from;
    TSTWRITE("%d ",p->from);
    p = lstNext(v);
  }
  TSTWRITE("\n");
  TST("Starts at 10, ends at 19",(j == 10 && k == 19));

  p = lstLast(v);
  TSTWRITE("# ");
  j = p->from;
  while (p != NULL) {
    k = p->from;
    TSTWRITE("%d ",p->from);
    p = lstPrev(v);
  }
  TSTWRITE("\n");
  TST("Starts at 19, ends at 10",(j == 19 && k == 10));

  v = lstFree(v);
  }
 #endif

  /**********************************************************************/
 #if 1
  TSTSECTION("Stack discipline");

  TSTGROUP("Creating stacks");

  v = stkNew(sizeof(Arc));
  TST("The new stack is empty",(v != NULL && stkIsEmpty(v)));

  TSTGROUP("Pushing");
  TSTNOTE("Let's push three elements.");

  a.from = 10; a.to = 32;  stkPush(v,&a);
  a.from++; a.to--;        stkPush(v,&a);
  a.from++; a.to--;        stkPush(v,&a);
  TST("Three elements",(stkDepth(v) == 3));


  TSTGROUP("Access the top");
  TSTNOTE("Check top of the stack");
  p = stkTop(v);
  TST("stkTop() ",(p!=&a && p->from == a.from && p->to == a.to));
  TSTNOTE("Let's check that it is really at position [2]");
  TST("direct access",(p == vecGet(v,2)));

  TSTGROUP("Popping elements");
  stkPop(v);
  TST("Two elements",(stkDepth(v) == 2));

  p = stkTop(v);
  TST("Top? ",(p!=&a && p->from == a.from-1 && p->to == a.to+1));
  stkPop(v);
  TST("One element",(stkDepth(v) == 1));
  TSTONFAIL("Depth = %d",stkDepth(v));
  stkPop(v);
  TST("No element",(stkIsEmpty(v) && stkDepth(v) == 0));
  TSTONFAIL("Depth = %d",stkDepth(v));
  stkPop(v);
  TST("Still No element",(stkIsEmpty(v) && stkDepth(v) == 0));
  TSTONFAIL("Depth = %d",stkDepth(v));
  p = stkTop(v);
  TST("Top is NULL",(p == NULL));

  stkPush(v,&a);
  stkPush(v,&a);
  stkPush(v,&a);
  stkPush(v,&a);
  stkPush(v,&a);
  stkPush(v,&a);
  TST("Six elements",(stkDepth(v) == 6));

  stkReset(v);
  TST("Reset",(stkDepth(v) == 0));

  v = stkFree(v);
  TST("Free stack",(v == NULL));

 #endif

 #if 0
 uint16_t rndbit();

  TSTSECTION("Random number");
  TSTGROUP("Sequence of bits");
  TST("Just create some random bits",1);
  TSTWRITE("#\n#");
  for (k = 0; k<50; k++) {
    TSTWRITE("%d",rndbit());
  }
  TSTWRITE("\n");
 #endif

 #if 1
  {
  mapNode *pn,*qn;
  map_t v;

  TSTSECTION("Maps (RBST)");

  TSTGROUP("Creating maps");

  v = mapNew(sizeof(Arc),NULL);

  TST("The new map is empty",(v != NULL && mapCnt(v) == 0 && mapRoot(v) == NULL));
  a.from = 12; a.to = 10;
  p = mapAdd(v,&a);  pn = mapNodePtr(p);
  TST("Added to root",(vecGet(v->nodes,0) == mapRoot(v) && pn == mapRoot(v)));

  a.from = 6; a.to = 10;
  p = mapAdd(v,&a);  pn = mapNodePtr(p);
  TST("Added as root left child",(pn == mapLeft(mapRoot(v))));

  a.from = 18; a.to = 10;
  p = mapAdd(v,&a);  pn = mapNodePtr(p);
  TST("Added as root right child",(pn == mapRight(mapRoot(v))));

  a.from = 8; a.to = 10;
  p = mapAdd(v,&a);
  pn = mapNodePtr(p);
  qn = mapRight(mapLeft(mapRoot(v)));
  TST("Added as root LR child",(qn != NULL && qn->elem != NULL && pn == qn && ((Arc *)(qn->elem))->from == 8));

  a.from = 16; a.to = 10;
  p = mapAdd(v,&a);
  pn =  mapNodePtr(p);
  /*TSTNOTE("pn=%p",pn);*/
  qn = mapLeft(mapRight(mapRoot(v)));
  /*TSTNOTE("qn=%p",qn);*/
  TST("Added as root RL child",(qn != NULL && qn->elem != NULL && pn == qn && ((Arc *)(qn->elem))->from == 16));

  mapTree(v);

  TSTWRITE("#\n# TREE: ");
  p = mapFirst(v);
  while (p != NULL) {
    TSTWRITE("%d ",p->from);
    p = mapNext(v);
  }
  TSTWRITE("\n#\n");

  a.from = 8; a.to = 10;
  mapDel(v,&a);
  TST("Deleted 8",(mapGet(v,&a) == NULL));

  mapTree(v);
  TSTWRITE("#\n# TREE: ");
  p = mapFirst(v);
  while (p != NULL) {
    TSTWRITE("%d ",p->from);
    p = mapNext(v);
  }
  TSTWRITE("\n#\n");

  a.from = 6; a.to = 10;
  mapDel(v,&a);
  TST("Deleted 6",(mapGet(v,&a) == NULL));
  mapTree(v);
  TSTWRITE("#\n# TREE: ");
  p = mapFirst(v);
  while (p != NULL) {
    TSTWRITE("%d ",p->from);
    p = mapNext(v);
  }
  TSTWRITE("\n#\n");

  a.from = 18; a.to = 10;
  mapDel(v,&a);
  TST("Deleted 18",(mapGet(v,&a) == NULL));

  mapTree(v);
  TSTWRITE("#\n# TREE: ");
  p = mapFirst(v);
  while (p != NULL) {
    TSTWRITE("%d ",p->from);
    p = mapNext(v);
  }
  TSTWRITE("\n#\n");

  v = mapFree(v);
#if 0
  TSTGROUP("Medium size map");

  v = mapNew(sizeof(Arc),NULL);
  TST("The new map is empty",(v != NULL && mapCnt(v) == 0 && mapRoot(v) == NULL));

  for (k=1;k<=200;k++) {
    a.from = k; a.to = 10;
    p = mapAdd(v,&a);  pn = mapNodePtr(p);
    if ((k+1) % 31 == 0)  {
      mapTree(v);
    }
  }
  mapTree(v);
  TST("The new map has 200 elements",(v != NULL && mapCnt(v) == 200));

  v = mapFree(v);
 #endif
#if 0
  TSTGROUP("Huge size map");

  v = mapNew(sizeof(Arc),arccmp);
  TST("The new map is empty",(v != NULL && mapCnt(v) == 0 && mapRoot(v) == NULL));

  for (k=1; k <= 700000; k++) {
    a.from = k; a.to = k;
    p = mapAdd(v,&a);
    if ((k+1) % 0xFFFF == 0)  {
      TSTNOTE(" Count: %u Depth: %u (%u)",mapCnt(v),mapDepth(v),mapMaxDepth(v));
    }
  }
  TSTNOTE(" Count: %u Depth: %u (%u)",mapCnt(v),mapDepth(v),mapMaxDepth(v));

  v = mapFree(v);
 #endif

 #if 0
  TSTGROUP("Huge size map (rand)");

  v = mapNew(sizeof(Arc),arccmp);
  TST("The new map is empty",(v != NULL && mapCnt(v) == 0 && mapRoot(v) == NULL));
  srand((uint32_t)time(NULL));
  for (k=1;k <= 700000; k++) {
    a.from = rand(); a.to = k;
    p = mapAdd(v,&a);
    if ((k+1) % 0xFFFF == 0)  {
      TSTNOTE(" Count: %u Depth: %u (%u)",mapCnt(v),mapDepth(v),mapMaxDepth(v));
    }
  }
  TSTNOTE(" Count: %u Depth: %u (%u)",mapCnt(v),mapDepth(v),mapMaxDepth(v));

  v = mapFree(v);
 #endif
  }
 #endif

 #if 0
  {
  setNode *p;
  Arc     *b;
  set     *tb;

  TSTSECTION("Set (RBST)");

  TSTGROUP("Creating set ");

  tb = setNew(sizeof(Arc),arccmp);
  TST("New set", (tb != NULL && setCnt(tb) == 0));

  TSTNOTE(" Creating the following tree ");
  TSTNOTE("  (12 (10 (9) (11)) (13 (15) (14)))");

  a.from = 12; a.to = 19;
  b = setAdd(tb,&a);
  TST("Added an element", (b != &a && setCnt(tb) == 1 && a.from == b->from));
  TSTONFAIL("Count = %d",setCnt(tb));

  a.from = 10; a.to = 19;
  b = setAdd(tb,&a);
  TST("Added an element", (b != &a && setCnt(tb) == 2 && a.from == b->from));
  TSTONFAIL("Count = %d",setCnt(tb));

  a.from = 14; a.to = 19;
  b = setAdd(tb,&a);
  TST("Added an element", (b != &a && setCnt(tb) == 3 && a.from == b->from));

  TSTWRITE("#\n# ");
  setTree(tb->root);
  TSTWRITE("\n#\n");

  a.from =  9; b = setAdd(tb,&a);
  a.from = 11; b = setAdd(tb,&a);
  a.from = 13; b = setAdd(tb,&a);
  a.from = 15; b = setAdd(tb,&a);

  TSTWRITE("#\n# ");
  setTree(tb->root);
  TSTWRITE("\n#\n");

  TSTGROUP("Rotations ");

  tb->root = rotLeft(tb->root);

  TSTNOTE("After a left rotation:");
  TSTWRITE("#\n# "); setTree(tb->root); TSTWRITE("\n#\n");

  tb->root = rotRight(tb->root);

  TSTNOTE("After a right rotation:");
  TSTWRITE("#\n# "); setTree(tb->root); TSTWRITE("\n#\n");

  TSTGROUP("Deleting elements");

  a.from = 10;
  setDel(tb,&a);
  TSTWRITE("#\n# "); setTree(tb->root); TSTWRITE("\n#\n");

  a.from = 12;
  setDel(tb,&a);
  TSTWRITE("#\n# "); setTree(tb->root); TSTWRITE("\n#\n");

  tb = setFree(tb);

  }
 #endif

 #if 0
  {
    arr a;

    TSTSECTION("Multitype array");
    TSTGROUP("Creating array");
    a = arrNew();

    arrSet(a,1,INT,-432);

    TST("Setting Integers",(arrGet(a,1,INT) == -432));

    arrSet(a,4,STR,"pippo");

    TST("Setting strings",(strcmp(arrGet(a,4,STR),"pippo") == 0));
    a = arrFree(a);
  }
 #endif

 #if 1
  { int c;
    TSTSECTION("Memory buffer");
    TSTGROUP("Creating buffer");
    v = bufNew();

    bufPuts(v,"pippo");
    bufPuts(v,"pluto");

    bufSeek(v,0);
    TSTWRITE("# ");
    while ((c=bufGetc(v)) != EOF) {
      if (c == '\0') TSTWRITE(". ");
      TSTWRITE("%c",c);
    }

    TSTWRITE("\n#\n");
    v = bufFree(v);
  }
 #endif

 #if 1
  TSTSECTION("Bitmaps");
  TSTGROUP("Creating bmps");

  v = bmpNew();

  for (k=0; k < 1000; k+=2) {
    bmpSet(v,k);
  }
  TST("Bitmap set",(bmpTest(v,4) && !bmpTest(v,5)));
  bmpClr(v,4); bmpFlip(v,5);
  TST("Bitmap clr and flip",(!bmpTest(v,4) && bmpTest(v,5)));
  bmpFree(v);
 #endif

 #if 0
 {
   lst_t l;

   TSTSECTION("Lists");
   TSTGROUP("Creating a list");
   l = lstNew(sizeof(Arc));

   for (k = 0; k < 100; k++) {
     a.from = k; a.to = 2 *k;
     lstInsHead(l,&a);
   }

   TST("List with 100 elements in order",(lstLen(l) == 100));
   TSTONFAIL("Count = %d",lstLen(l));

   p = lstFirst(l);
   TST("Access first",(p->from == 99));
   TSTONFAIL("First from = %d",p->from);

   p = lstLast(l);
   TST("Access last",(p->from == 0));
   TSTONFAIL("First from = %d",p->from);

   p = lstFirst(l);
   k = 0;
   while (p!=NULL) {
      k += p->from;
      p = lstNext(l,p);
   }
   TST("lstNext()",(k == (99*100/2)));

   l = lstFree(l);

   l = lstNew(sizeof(Arc));

   for (k = 0; k < 100; k++) {
     a.from = k; a.to = 2 *k;
     p = lstInsTail(l,&a);
   }

   TST("List with 100 elements in reverse order",(lstLen(l) == 100));
   TSTONFAIL("Count = %d",lstLen(l));

   p = lstFirst(l);
   TST("Access first",(p->from == 0));
   TSTONFAIL("First from = %d",p->from);

   p = lstLast(l);
   TST("Access last",(p->from == 99));
   TSTONFAIL("First from = %d",p->from);

   p = lstFirst(l);
   k = 0;
   while (p!=NULL) {
      k += p->from;
      p = lstNext(l,p);
   }
   TST("lstNext()",(k == (99*100/2)));

   l = lstFree(l);
 }
 #endif

 #if 0
 {
  stp_t pool;
  char *p;
  char *q;
  char  st[10];

   TSTSECTION("Strings pool");
   TSTGROUP("Creating a pool");
  pool = stpNew();

  p = stpAdd(pool,"pippo");

  TST("Added a string",(stpCnt(pool) == 1));

  q = stpGet(pool,"pippo");
  TST("Get a string",(p == q && strcmp(p,"pippo") == 0));

  st[2] = 0;
  for (j='0'; j<255; j++) {
    st[1] = (char)j;
    for (k=' '; k<256; k++) {
      st[0] = (char)k;
      p = stpAdd(pool,st);
    }
  }
  TSTNOTE("Count: %u\n",stpCnt(pool));

  p = stpGet(pool,"pippo");
  if (p != NULL) TSTNOTE("pippo: %s\n",p);
  p = stpGet(pool,"aa");
  if (p != NULL) TSTNOTE("aa: %s\n",p);

  p = stpDel(pool,"aa");
  p = stpGet(pool,"aa");
  TST("Deleted aa",(p== NULL));

  p = stpDel(pool,"fU");
  p = stpDel(pool,"w}");
  TST("Deleted w}",(p == NULL));
  p = stpGet(pool,"w}");
  TST("Deleted w}",(p== NULL));
  stpFree(pool);
 }
 #endif

 #if 1
 {
  uint32_t *b;
  TSTSECTION("uint blocks");
  TSTGROUP("Creating a block");

  b = ulvNew();

  TST("Created ulv",(b != NULL && ulvCnt(b)==0 && ulvSlt(b)==1));

  b = ulvSet(b,0,444);
  TST("Set element 0",((ulvGet(b,0) == 444) && (b[0] == 444)));

  b = ulvSet(b,1,445);
  TST("Set element 1",((ulvGet(b,1) == 445) && (b[1] == 445)));
  TSTNOTE("Block slots: %d count: %d",ulvSlt(b),ulvCnt(b));

  for (k = 2; k < 1000; k++) {
    b = ulvSet(b,k,545+k);
    if (k % 10 == 0)
      TSTNOTE("Block (%d) slots: %d count: %d",k,ulvSlt(b),ulvCnt(b));
  }
  TSTNOTE("Block (%d) slots: %d count: %d",k,ulvSlt(b),ulvCnt(b));

  TST("Added 1000 elements!",(ulvCnt(b) == 1000 && b[999] == 999+545 && b[999] == ulvGet(b,999)));

  b = ulvFree(b);
  TST("ulv freed",(b == NULL));
 }
 #endif

  TSTDONE();
  exit(0);
}

/*-------------------------------------------------------------*/
