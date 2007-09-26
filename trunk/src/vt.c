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

int main(int argc, char * argv[])
{
  vec *v;
 
  uint32_t j,k,n=0;
  Arc a,*p;
  uint32_t t;
  uint8_t **p8;
  Arc *q;
  
  
  TSTHDR("Basic vec");
  
  TSTGROUP("Creating vectors");
  
  /**************************/
  TSTDESC("Just create and destroy a vector;");
  v=vecNew(sizeof(Arc));
  TST("New vector",(v != NULL));
  
  v=vecFree(v); /* Always use this idiom when freeing a vector */
  TST("Free vector",(v == NULL));
  
  /**************************/
  TSTDESC("Elements in a vector are supposed to be big enough to contain a\n"
          "generic pointer. If a smaller size is specified, the size is\n"
          "enlarged enough to ensure a pointer will fit.");
          
  v = vecNew(0);
  TST("New vector (elemsz 0 -> sizeof(void*))",
         (v != NULL && v->esz == sizeof(void *)));
         
  v = vecFree(v);
  
  TSTGROUP("Adding elements");
  
  v = vecNew(sizeof(Arc));

  a.from = 10; a.to = 32;
  
  TSTDESC("Let's start with an empty vector.\n");
  TST("vecCnt == 0",(vecCnt(v) == 0));
  
  TSTDESC("Now a single element stored in position 0.\n");
  p = vecSet(v,0,&a);
  TST("vecCnt == 1",(vecCnt(v) == 1));
  TST("cur_p  == 0",(v->cur_p == 0));
  TST("cur_n  == 0",(v->cur_n == 0));
  TST("cur_w  == 0",(v->cur_w == 0));
    
  TSTDESC("There is only one page allocated");
  TST("v->npg == 1",(v->npg == 1 && v->arr != NULL && v->arr[0] != NULL));
  
  v = vecFree(v);

  /**********************************************************************/
  TSTHDR("Stack discipline");

  TSTGROUP("Creating stacks");
  
  v = stkNew(sizeof(Arc));
  TST("The new stack is empty",(v != NULL && stkIsEmpty(v)));

  TSTGROUP("Pushing");
  TSTDESC("Let's push three elements.\n");
  
  a.from = 10; a.to = 32;  stkPush(v,&a);
  a.from++; a.to--;        stkPush(v,&a);
  a.from++; a.to--;        stkPush(v,&a);
  TST("Three elements",(stkDepth(v) == 3));

 
  TSTGROUP("Access the top");
  TSTDESC("Check top of the stack");
  p = stkTop(v);
  TST("stkTop() ",(p!=&a && p->from == a.from && p->to == a.to));
  TSTDESC("Let's check that it is really at position [2]");
  TST("direct access",(p == vecGet(v,2)));
  
  TSTGROUP("Popping elements");
  stkPop(v);
  TST("Two elements",(stkDepth(v) == 2));
  
  p = stkTop(v);
  TST("Top? ",(p!=&a && p->from == a.from-1 && p->to == a.to+1));
  stkPop(v);
  TSTW("One element",(stkDepth(v) == 1),"\tDepth = %d\n",stkDepth(v));
  stkPop(v);
  TSTW("No element",(stkIsEmpty(v) && stkDepth(v) == 0),"\tDepth = %d\n",stkDepth(v));
  stkPop(v);
  TSTW("Still No element",(stkIsEmpty(v) && stkDepth(v) == 2),"\tDepth = %d",stkDepth(v));
  p = stkTop(v);
  TST("Top is NULL",(p == NULL));
  
  v = stkFree(v);
  TST("Free stack",(v == NULL));
  
  TSTSTAT();
  #if 0
  v=vecNew(sizeof(Arc));
  
  printf("n;minsize;vsize;diff\n");
  for (j=1; j <100 ; j++) {
    for (k=j; k < 100000; k++) {
      if ((n & 0x3FF) == 0) {
        t  = vecSize(v);
        printf("%u;%u;%u;%u\n",n,sizeof(Arc)*n,t, t-sizeof(Arc)*n);
      }
      a.from = j; a.to = k;
      vecSet(v,n++,&a); 
    }
  } 
 
  p = (Arc *)vecGet(v,n/2);
  
  /*printf("%u;%u;%u;%u\n",n,sizeof(Arc)*n,t, t-sizeof(Arc)*n);*/
  
  printf("%d -> %d\n",p->from,p->to);
  p = (Arc *)vecNext(v);
  printf("%d -> %d\n",p->from,p->to);
  p = (Arc *)vecPrev(v);
  printf("%d -> %d\n",p->from,p->to);
  p = (Arc *)vecNext(v);
  printf("%d -> %d\n",p->from,p->to);
  printf("CNT: %u\n",vecCnt(v));
  fflush(stdout);
 
  v=vecFree(v);
  #endif
  
  /**** SET ****/
  #if 0
  v = setNew(sizeof(Arc),offsetof(Arc,info));

  a.from = 100; a.to = 100; a.info = 2323;
    
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  /*
  p = setGet(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  p8 = (uint8_t **)p - 1;
  printf("%p == %p\n",p,*p8);
  
 fflush(stdout);
 */
  a.from++;
  /*
  p = setGet(v,&a);
  printf("%p\n",p);
 */
  
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  /*
  p8 = (uint8_t **)p - 1;
  printf("%p == %p\n",p,*p8);
*/
 a.from--;
 fflush(stdout);
  /*
  p = setGet(v,&a);
  printf("%p\n",p);
  / *
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);*/
  
  a.from = 10; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 210; a.to = 210;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  printf("\n");
  
  a.from = 40; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 100; a.to = 100;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 140; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.info = 92;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  a.from = 40; a.to = 10;
  p = setGet(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  setDel(v,&a);
  p = setGet(v,&a);
  printf("%p\n",p);
  a.from = 140; a.to = 10;
  p = setGet(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  setDel(v,&a);
  p = setGet(v,&a);
  printf("%p\n",p);
  
  fflush(stdout);
  setForeach(v,k,p) 
  printf("%d] %p %d -> %d\n",k,p,p->from,p->to);
  
  
  a.from = 40; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  a.from = 140; a.to = 10;
  p = setAdd(v,&a);
  printf("%p %d -> %d (%p %d -> %d)\n",p,p->from,p->to,&a,a.from,a.to);
  
  fflush(stdout);
  setForeach(v,k,p) 
  printf("%d] %p %d -> %d\n",k,p,p->from,p->to);
    
  /*
  for (k=0;k < vecCnt(v);k++) {
    p = vecGet(v,k);
    p = *(Arc **)p;
    printf("%d] %p %d -> %d\n",k,p,p->from,p->to);
    if (k>0) printf(" == %d ",memcmp(q,p,v->esz-sizeof(uint8_t *)));
    printf("\n");
    q = p;
  }
*/
  v = setFree(v);
 #endif
 
 #if 0
  v = vecNew(sizeof(Arc));
  
  a.from = 3; a.to = 6;
  for (k = 0; k < 200; k++) {
    printf("%u;%u;%u\n",k,vecSize(v),k*sizeof(Arc));
    blkAdd(v,&a);
  }
  v = vecFree(v);
 #endif
 
 #if 0
  v = blkNew(sizeof(Arc));
  a.from = 3; a.to = 6;
  p=blkAdd(v,&a);
  a.from = 3; a.to = 7;
  blkAdd(v,&a);
  a.from = 3; a.to = 8;
  blkAdd(v,&a);
  printf("CNT: %u\n", v->mrk_w);
  blkDel(v,p);
  a.from = 2; a.to = 8;
  blkAdd(v,&a);
  printf("CNT: %u\n", v->mrk_w);
  a.from = 1; a.to = 8;
  blkAdd(v,&a);
  printf("CNT: %u\n", v->mrk_w);
  v = blkFree(v);
 #endif
 
 #if 0
   v = stkNew(sizeof(Arc));
  printf("CNT: %u\n", vecCnt(v));
  a.from = 1; a.to = 8;
   stkPush(v,&a);
  printf("CNT: %u\n", vecCnt(v));
   p = stkTop(v);
  printf("%p %u->%u (%u->%u)\n",p, p->from,p->to,a.from,a.to);
  a.from = 43; a.to = 8;
   stkPush(v,&a);
  printf("CNT: %u\n", vecCnt(v));
   p = stkTop(v);
  printf("%p %u->%u (%u->%u)\n",p, p->from,p->to,a.from,a.to);
   stkPop(v);
  printf("CNT: %u\n", vecCnt(v));
   p = stkTop(v);
  printf("%p %u->%u (%u->%u)\n",p, p->from,p->to,a.from,a.to);
   stkPop(v);
  printf("CNT: %u\n", vecCnt(v));
   p = stkTop(v);
  printf("%p \n",p);
   
   v = stkFree(v);
 #endif
 
 #if 0
   v = mapNew(sizeof(Arc),offsetof(Arc,info));

   for (j = 0; j< 10 ; j++) {
     a.from = rand() & 0xFF;
     a.to = rand()  & 0xFF;
     printf("%d: %u -> %u\n",j,a.from,a.to);
     p = mapAdd(v,&a); 
   }

   a.from = 3; a.to = 8;
   p = mapAdd(v,&a); 
   p = mapAdd(v,&a); 
   
   /*
   a.from = 4; a.to = 8;
   p = mapAdd(v,&a); 

   */
      

   a.from = 2; a.to = 8;
   p = mapAdd(v,&a); 
   a.from = 2; a.to = 7;
   p = mapAdd(v,&a); 

   p = mapFirst(v);
   t=0;
   while (p) {
    printf("NODE: %p (%u -> %u)\n",p,p->from, p->to);
    p = mapNext(v);
    if (t++ > 15) break;
   }
   v = mapFree(v);
 #endif
 
  exit(0); 
}


