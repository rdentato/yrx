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
Resizable Arrays in Optimal Time and Space
Lecture Notes In Computer Science; Vol. 1663
Proceedings of the 6th International Workshop on Algorithms and Data Structures
1999
Andrej Brodnik 	
Svante Carlsson 	
Erik D. Demaine 	
J. Ian Munro 	
Robert Sedgewick 	
Springer-Verlag   London, UK 
*/

#include "vec.h"

static const char *errNOMEM = "Out of Memory (vec)";
static const char *errUNEXP = "Unexpected error";

#define PTRSTEP 16  
#define PGSTEP  64
#define MBSTEP 512

#define old_pgsize(n) ((uint32_t)((n+1)*PGSTEP))

/*
** Integer log base 2 of uint32 integer values.
**   llog2(0) == llog2(1) == 0
** Makes a binary search.
*/
static uint32_t llog2(uint32_t x)
{
  uint32_t l=0;
  if (x & 0xFFFF0000) {l += 16; x >>= 16;}   /* 11111111111111110000000000000000 */
  if (x & 0xFF00)     {l += 8;  x >>= 8 ;}   /* 1111111100000000*/
  if (x & 0xF0)       {l += 4;  x >>= 4 ;}   /* 11110000*/
  if (x & 0xC)        {l += 2;  x >>= 2 ;}   /* 1100 */
  if (x & 2)          {l += 1;  }            /* 10 */
  return l;
}

static uint32_t pgsize(uint32_t p)
{
  uint32_t k;

  p += 2;
  k  = llog2(p/2);
  if (p >= 3* (1<<k)) k++;
  return (1<<k) * PGSTEP;
}

static void ndx2pg(uint32_t ndx, uint32_t *p, uint32_t *n)
{
  uint32_t k,m,s,b;

 *p = 0;
  m =(ndx/PGSTEP) ;

  k = llog2(m+1);
 *p = (k & 1)? 1<<(k/2): 0;

  m = m - (1<<k) + 1 ;
  s = k - (k/2);
  k = k/2;
  b = m >> s;

 *p += (1<<(k+1)) - 2 + b;
 *n  = ((m - (b<<s))*PGSTEP)+ (ndx & (PGSTEP-1));
 
  dbgprintf(DBG_OFF,"  %u %u %u (%u)\n",ndx,*p, *n,pgsize(*p));
}


static void vecInit(vec *v,uint32_t elemsize)
{
  if (v) {
    /* Ensure an element can store a (void *), will be used for the freelist */
    if (elemsize < sizeof(void *)) elemsize =  sizeof(void *);
    v->esz   = elemsize;
    v->ksz   = v->esz;
    v->npg   = 0;
    v->cnt   = 0;
    v->arr   = NULL;
    v->aux   = NULL;
    v->mrk.p = 0;
    v->mrk.n = (uint32_t)-1;
    v->mrk.w = (uint32_t)-1;
    v->mrk.q = ((uint8_t *)0 - elemsize);
  }
}

vec *vecNew(uint16_t elemsize)
{
  vec *v;

  v = malloc(sizeof(vec));
  vecInit(v,elemsize);
  return v;
}


static void vcleanup (vec *v)
{
  uint16_t i;
  if (v) {
    if (v->arr) {
      for (i=0; i< v->npg; i++) {
        if (v->arr[i] != NULL) free(v->arr[i]);
        v->arr[i] = NULL;
      }
      free(v->arr);
      v->arr = NULL;
    }
  }
}

void *vecFree(vec *v)
{
  if (v) {
    vcleanup(v);
    free(v);
  }
  return NULL;
}

static void *vecslot(vec *v,uint32_t page, uint32_t n)
{
  void *p=NULL;
  uint32_t t;

  if (page >= v->npg) {
    /* Extend the index to pages*/
    dbgprintf(DBG_OFF,"\t *X* %d %d\n",page,v->npg);
    t = v->npg;
    v->npg = ((page / PTRSTEP)+1)*PTRSTEP;
    v->arr = realloc(v->arr, v->npg * sizeof(void *));
    if (v->arr == NULL) err(3001,errNOMEM);
    while (t < v->npg)  v->arr[t++] = NULL;
    dbgprintf(DBG_OFF,"\t *** %d \n",v->npg);
  }

  v->mrk.s = pgsize(page);
  
  if (v->arr[page] == NULL) {
    v->arr[page] = calloc(v->mrk.s, v->esz); /* page is guaranteed to be filled with 0's */
    if (v->arr[page] == NULL)  err(3002,errNOMEM);
  }

  p = v->arr[page] + (n * v->esz);

  v->mrk.p = page;
  v->mrk.n = n;
  v->mrk.q = p;

  return p;
}

#define vecMark(v,m,o)  do {if (o=='r') (v)->mrk = m; else  m=(v)->mrk;} while(0)

void *vecGet(vec *v,uint32_t ndx)
{
  uint32_t page, n;

  if (v == NULL)  err(3003,errNOMEM);
  
  if (ndx == v->mrk.w) return v->mrk.q;
  
  v->mrk.w = ndx;
  ndx2pg(ndx,&page,&n);
  return vecslot(v,page,n);
}

void *vecNext(vec *v)
{
  if (v == NULL)  err(3004,errNOMEM);
  
  v->mrk.w++;
  if (++v->mrk.n >= v->mrk.s) return vecslot(v,v->mrk.p+1,0);

  v->mrk.q += v->esz;
  return v->mrk.q;
}

void *vecPrev(vec *v)
{
  if (v == NULL) err(3005,errNOMEM);
  if (v->mrk.w == 0) return NULL;
  if (v->mrk.n == 0) return vecGet(v,v->mrk.w - 1);
  
  v->mrk.n--;
  v->mrk.w--;
  v->mrk.q -= v->esz;
  return v->mrk.q;
}

void *vecSet(vec *v, uint32_t ndx, void *elem)
{
  void *p;

  p = vecGet(v,ndx);
  
  if (p != NULL) {
    if (ndx >= v->cnt) v->cnt = ndx+1;
    if (elem) memcpy(p, elem, v->esz);
    else      memset(p, 0x00, v->esz);
  }
  return p;
}

void *vecAdd(vec *v, void *elem)
{
  return vecSet(v,v->mrk.w+1,elem);
}

vec *vecShrink(vec *v,uint32_t ndx)
{
  uint32_t page, n;
  
  ndx2pg(ndx,&page,&n);
  
  while (++page < v->npg) {
     if (v->arr[page] != NULL) free(v->arr[page]);
     v->arr[page] = NULL;
  }
  if (v->cnt > ndx) v->cnt = ndx;
  return v; 
}

uint32_t vecSize(vec *v)
{
  uint16_t k = 0;
  uint32_t size = 0;
    
  if (v) {
    for (k=0; k < v->npg; k++) {
      if (v->arr[k] != NULL) {
        size += pgsize(k) * v->esz;
      } 
    }
    size += v->npg * sizeof(uint8_t *);
    size += sizeof(vec);
    
  }
  return size;
}

/********** SET ***********/



vec *setNew(uint16_t elemsize,uint16_t keysize)
{
   vec *v;
   
   v = vecNew(elemsize);
   if (v != NULL) {
     v->aux = vecNew(sizeof(void *));
     if (vecNdx(v) == NULL) v = vecFree(v);
   }
   if (v != NULL && keysize > 0) v->ksz = keysize;

   return v; 
}

static void *setsearch(vec *v,void *elem,uint32_t *ndx)
{
   int i = 0, k = 0;
   int c = 0, j = 0;
   void **q;
   
   if (vecNdx(v) == NULL) err(3232,errUNEXP);
   
   j = vecCnt(vecNdx(v))-1;
   
   while (i <= j) {
     k = (i+j)/2;
    *ndx = k;
     q = vecGet(vecNdx(v),k);
     if (q == NULL) err(3239,errUNEXP);
     if ((c = memcmp(elem, *q, v->ksz)) == 0) return *q;
     dbgprintf(DBG_MSG,"** search: %d,%d [%d] == %d\n",i,j,k,c);
     if (c < 0) j = k-1; 
     else i = k+1;
   }
  *ndx = k;
   if (c > 0) *ndx = k+1;
   return NULL;  
}

void *setGet(vec *v,void *elem)
{
  uint32_t k;
  return setsearch(v,elem,&k);
}

void *setAdd(vec *v,void *elem)
{
  int i;
  uint32_t k;
  void **q,**r;
  void *p;
 
  p = setsearch(v,elem,&k);

  if (p == NULL) {
    /* Need to be inserted in pos. k */
    i = vecCnt(vecNdx(v));
    
    /* Add |elem| to the vector */    
    q = vecNdx(v)->aux;
    if (q == NULL) {       
      p = vecSet(v,vecCnt(v),elem);
    }
    else {
      vecNdx(v)->aux = *q;
      p = (void *)q;
      memcpy(p, elem, v->esz);    
    }
    
    dbgprintf(DBG_MSG,"To be placed at: %d (i:%d p:%p)\n",k,i,p);
    q = vecSet(vecNdx(v),i,&p);
    /* Shift down pointers from k to the end*/
    while (i-- > k) {
      dbgprintf(DBG_MSG,"\t(i:%d)\n",i);
      r = q;
      q = vecGet(vecNdx(v),i);
     *r = *q;
    }
    
    *q = p; /* q points to vec[k] */
  }
  else  {
    memcpy(p, elem, v->esz);
    dbgprintf(DBG_MSG,"Found at: %d\n",k);
  }
  
  return p;  
}


void setDel(vec *v,void *elem)
{
  int i;
  uint32_t k;
  void **q,**r;
 
  q = setsearch(v,elem,&k);
  if (q != NULL) {
    *q = vecNdx(v)->aux;
    vecNdx(v)->aux = q;  /* Free list */
    
    /* Remove k-th index */
    q = vecGet(vecNdx(v),k);
    
    for (i = k+1; i < vecCnt(vecNdx(v)); i++) {
      r = q;
      q = vecGet(vecNdx(v),i);
     *r = *q;
    }
    vecCnt(vecNdx(v))--;
  }
}
