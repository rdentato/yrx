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
  uint16_t from;
  uint16_t to;
  uint32_t info[31];
} Arc;

int arccmp(Arc *a, Arc *b)
{
  int t;
  t = a->from - b->from;
  if (t == 0) t = a->to - b->to;
  return t;
}

#if 0
void setTree(setNode *p)
{

  if (p != NULL) {
    TSTWRITE("%d [%d]",((Arc *)(p->elem))->from,p->size);
    if (p->left != NULL || p->right != NULL) {
      TSTWRITE(" (");
      setTree(p->left);
      TSTWRITE(") (");
      setTree(p->right);
      TSTWRITE(")");
    }
  }

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

  v = vecNew(0);
  TSTT("New vector (elemsz 0 -> sizeof(void*))",
         (v != NULL && v->esz == sizeof(void *)),"Re-define free list management");

  v = vecFree(v);

  TSTGROUP("Adding elements");

  v = vecNew(sizeof(Arc));


  a.from = 10; a.to = 32;

  TSTNOTE("Let's start with an empty vector.");
  TST("vecCnt == 0",(vecCnt(v) == 0));

  TSTNOTE("Now a single element stored in position 0.");
  p = vecSet(v,0,&a);
  TST("vecCnt == 1",(vecCnt(v) == 1));
  TST("cur_p  == 0",(v->cur_p == 0));
  TST("cur_n  == 0",(v->cur_n == 0));
  TST("cur_w  == 0",(v->cur_w == 0));

  TSTNOTE("There is only one page allocated");
  TST("v->npg == 1",(v->npg == 1 && v->pgs != NULL && v->pgs[0] != NULL));
  TSTNOTE("Check that elements are stored in order");
  for (k=0; k < 32; k++) {
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

 #if 0
  {
  blkNode *pn,*qn;

  TSTSECTION("Maps (RBST)");

  TSTGROUP("Creating maps");

  v = mapNew(sizeof(Arc),offsetof(Arc,info));
  TST("The new map is empty",(v != NULL && mapCnt(v) == 0 && mapRoot(v) == NULL));
  a.from = 12; a.to = 10;
  p = mapAdd(v,&a);  pn = blkNodePtr(p);
  TST("Added to root",(vecGet(v,0) == mapRoot(v) && pn == mapRoot(v)));

  a.from = 6; a.to = 10;
  p = mapAdd(v,&a);  pn = blkNodePtr(p);
  TST("Added as root left child",(pn == blkLeft(mapRoot(v))));

  a.from = 18; a.to = 10;
  p = mapAdd(v,&a);  pn = blkNodePtr(p);
  TST("Added as root right child",(pn == blkRight(mapRoot(v))));

  a.from = 8; a.to = 10;
  p = mapAdd(v,&a);
  pn = blkNodePtr(p);
  qn = blkRightPtr(blkLeftPtr(mapRoot(v)));
  TST("Added as root LR child",(pn == qn && ((Arc *)(qn->elm))->from == 8));

  a.from = 16; a.to = 10;
  p = mapAdd(v,&a);
  pn =  blkNodePtr(p);
  qn = blkLeftPtr(blkRightPtr(mapRoot(v)));
  TST("Added as root RL child",(pn == qn && ((Arc *)(qn->elm))->from == 16));

  TSTWRITE("#\n# TREE: ");
  p = mapFirstSorted(v);
  while (p != NULL) {
    TSTWRITE("%d ",p->from);
    p = mapNextSorted(v);
  }
  TSTWRITE("\n#\n");

  a.from = 8; a.to = 10;
  mapDel(v,&a);
  TST("Deleted 8",(mapGet(v,&a) == NULL));

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

  TSTWRITE("#\n# TREE: ");
  p = mapFirst(v);
  while (p != NULL) {
    TSTWRITE("%d ",p->from);
    p = mapNext(v);
  }
  TSTWRITE("\n#\n");

  v = mapFree(v);

  TSTGROUP("Creating maps");

  v = mapNew(sizeof(Arc),offsetof(Arc,info));
  TST("The new map is empty",(v != NULL && mapCnt(v) == 0 && mapRoot(v) == NULL));

  for (k=1;k<=200;k++) {
    a.from = k; a.to = 10;
    p = mapAdd(v,&a);  pn = blkNodePtr(p);
    if ((k+1) % 31 == 0)  {
      TSTWRITE("#\n# TREE: (");
      mapTree(mapRoot(v));
      TSTWRITE("\n#\n");
    }
  }
  TST("The new map has 200 elements",(v != NULL && mapCnt(v) == 200));

  v = mapFree(v);
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

  for (k=0; k < 100; k+=2) {
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

 #if 1
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

  st[1] = 0;
  for (k='a'; k<'z'; k++) {
    st[0] = k;
    p = stpAdd(pool,st);
  }
  stpFree(pool);
 }  
 #endif


  TSTDONE();
  exit(0);
}

/*-------------------------------------------------------------*/

