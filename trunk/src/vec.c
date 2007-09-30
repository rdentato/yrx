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
On Resizable arrays:

@inproceedings{ brodnik99resizable,
    author = "Andrej Brodnik and Svante Carlsson and Erik D. Demaine and J. Ian Munro and Robert Sedgewick",
    title = "Resizable Arrays in Optimal Time and Space",
    booktitle = "Workshop on Algorithms and Data Structures",
    pages = "37-48",
    year = "1999",
    url = "citeseer.ist.psu.edu/brodnik99resizable.html"
}

@article{Demaine:2001:AAF,
  author =       "Erik Demaine",
  title =        "Algorithm Alley: Fast and Small Resizable Arrays",
  journal =      "Doctor Dobb's Journal",
  volume =       "26",
  number =       "7",
  pages =        "132--134",
  month =        "jul",
  year =         "2001",
  CODEN =        "DDJOEB",
  ISSN =         "1044-789X",
  bibdate =      "Thu Jun 7 06:07:17 MDT 2001",
  url =          "http://www.ddj.com/architect/184404698",
}

On Randomized binary search tree:

@misc{ martinez97randomized,
  author = "C. Martinez and S. Roura",
  title = "Randomized binary search trees",
  text = "Conrado Martinez and Salvador Roura, Randomized binary search trees, in
    Research report of Universitat Politcnica de Catalunya, LSI97 -8-R, 1997.",
  year = "1997",
  url = "citeseer.ist.psu.edu/martinez97randomized.html"
}

@article{ seidel96randomized,
    author = "Raimund Seidel and Cecilia R. Aragon",
    title = "Randomized Search Trees",
    journal = "Algorithmica",
    volume = "16",
    number = "4/5",
    pages = "464-497",
    year = "1996",
    url = "citeseer.ist.psu.edu/seidel96randomized.html"
}

On Random Number generator:

@misc{ l'ecuyer99table,
  author = "P. L'Ecuyer",
  title = "A table of linear congruential generators of different sizes and good lattice
    structure",
  text = "P. L'Ecuyer. A table of linear congruential generators of different sizes
    and good lattice structure. Mathematics of Computation, 68(225), 1999. To
    appear.",
  year = "1999",
  url = "citeseer.ist.psu.edu/132363.html"
}

@misc{ entacher97collection,
  author = "K. Entacher",
  title = "A collection of selected pseudorandom number generators with linear structures",
  text = "K. Entacher. A collection of selected pseudorandom number generators with
    linear structures. Technical Report 97-1, ACPC -- Austrian Center for Parallel
    Computation, University of Vienna, Austria, 1997. Available at: http://random.mat.sbg.ac.at/.",
  year = "1997",
  url = "citeseer.ist.psu.edu/entacher97collection.html" }

*/

#include "vec.h"

static const char *errNOMEM = "Out of Memory (vec)";
static const char *errUNEXP = "Unexpected error";

/*
** Integer log base 2 of uint32 integer values.
**   llog2(0) == llog2(1) == 0
** Makes a binary search.
*/
static uint16_t llog2(uint32_t x)
{
  uint16_t l=0;

  if (x & 0xFFFF0000) {l += 16; x >>= 16;}   /* 11111111111111110000000000000000 */
  if (x & 0xFF00)     {l += 8;  x >>= 8 ;}   /* 1111111100000000*/
  if (x & 0xF0)       {l += 4;  x >>= 4 ;}   /* 11110000*/
  if (x & 0xC)        {l += 2;  x >>= 2 ;}   /* 1100 */
  if (x & 2)          {l += 1;  }            /* 10 */
  return l;
}

#define two_raised(n) (1<<(n))
#define div2(n)       ((n)>>1)

static uint16_t pgsize(uint16_t p)
{
  uint16_t k;

  p += 2;
  k  = llog2(p) - 1;

  k  = two_raised(k);
  if (p >= 3*k) k <<= 1;
  return k;

/*
  if (p >= 3* two_raised(k)) k++;
  return two_raised(k);
  */
}

static void ndx2pg(uint32_t ndx, uint16_t *p, uint16_t *n)
{
  uint32_t k,m,s;

  m  = ndx +1;
  k  = llog2(m);              /* compute MSB */
  m ^= two_raised(k);         /* clear MSB */
  s  = div2(k+1);             /* ceil(k/2)  */
  k  = div2(k);               /* floor(k/2)  */
 *p  = (uint16_t)(
	     (two_raised(s) - 1) +  /* Sum for i=0 to ceil(k/2 - 1)  of 2^i */
         (two_raised(k) - 1) +  /* Sum for i=0 to floor(k/2 - 1) of 2^i */
         (m >> s)
	   );
 *n  = (uint16_t) (m & (two_raised(s) - 1));
  dbgprintf(DBG_OFF,"ndg2pg ->  %4u %4u %4u (%4u)\n",ndx,*p, *n, pgsize(*p));

}


static void vecInit(vec *v,uint16_t elemsize)
{
  if (v != NULL) {
    /* Ensure an element can store a 32 bit integer, will be used for the freelist */
    if (elemsize < sizeof(uint32_t)) elemsize =  sizeof(uint32_t);
    v->esz    = elemsize;
    v->ksz    = v->esz;
    v->npg    = 0;
    vecCnt(v) = 0;
    v->arr    = NULL;
    v->cur_p  = (uint16_t)-1;
    v->cur_n  = (uint16_t)-1;
    v->cur_w  = VEC_ANYNDX;
    v->cur_q  = ((uint8_t *)0 - elemsize);
  }
}

vec *vecNew(uint16_t elemsize)
{
  vec *v;

  v = malloc(sizeof(vec));
  vecInit(v,elemsize);
  dbgprintf(DBG_OFF,"VEC: %p\n",v);
  return v;
}


static void vcleanup (vec *v)
{
  uint16_t i;
  if (v != NULL) {
    if (v->arr != NULL) {
      for (i = 0; i< v->npg; i++) {
        dbgprintf(DBG_OFF,"#  %d %p\n",i,v->arr[i]);
        if (v->arr[i] != NULL)
           free(v->arr[i]);
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

static void *vecslot(vec *v,uint16_t page, uint16_t n)
{
  void *p=NULL;
  uint32_t t;

  if (page >= v->npg) {
    /* Extend the index to pages*/
    dbgprintf(DBG_OFF,"\t *X* %d %d %d\n",page,v->npg,llog2(page+1));

    t = v->npg;
   /* The smallest power of two greater then |page| */
    v->npg = (page == 0)? 1 : 1 << (llog2(page) +1) ;

    dbgprintf(DBG_OFF,"\t === %d ",v->npg);

    v->arr = realloc(v->arr, v->npg * sizeof(void *));
    if (v->arr == NULL) err(3001,"Unable to allocate page index for (%u)\n",page);
    while (t < v->npg)  v->arr[t++] = NULL;

    dbgprintf(DBG_OFF,"\t *** %d \n",v->npg);
  }

  if (page != v->cur_p) v->cur_s = pgsize(page);

  if (v->arr[page] == NULL) {
    v->arr[page] = calloc(v->cur_s, v->esz); /* page is guaranteed to be filled with 0's */
    if (v->arr[page] == NULL)  err(3002,errNOMEM);
    dbgprintf(DBG_OFF,"#  page %d 0x%p\n",page,v->arr[page]);
  }

  p = v->arr[page] + (n * v->esz);

  v->cur_p = page;
  v->cur_n = n;
  v->cur_q = p;

  return p;
}

void *vecGet(vec *v,uint32_t ndx)
{
  uint16_t page, n;

  if (v == NULL)  err(3003,errNOMEM);

  if (ndx == VEC_ANYNDX) ndx = 0;

  if (v->cur_w != VEC_ANYNDX) {
    switch ((int)(ndx - v->cur_w)) {
      case  0 : dbgprintf(DBG_OFF,"get same %u (%u)\n",ndx,v->cur_w);
                return v->cur_q;

      case  1 : dbgprintf(DBG_OFF,"get next %u (%u)\n",ndx,v->cur_w);
                v->cur_w++;
                v->cur_n++;
                if (v->cur_n >= v->cur_s) return vecslot(v,v->cur_p+1,0);
                v->cur_q += v->esz;
                return v->cur_q;
      #if 0
      case -1 : dbgprintf(DBG_OFF,"get prev %u (w:%u p:%u)\n",ndx,v->cur_w,v->cur_p);
                if (v->cur_w == 0) return NULL;
                if (v->cur_n == 0) {
                  v->cur_p--;
                  return vecslot(v,v->cur_p,pgsize(v->cur_p) - 1);
                }
                v->cur_n--;
                v->cur_w--;
                v->cur_q -= v->esz;
                return v->cur_q;
      #endif
    }
  }
  dbgprintf(DBG_OFF,"get index %d (%d)\n",ndx,v->cur_w);
  v->cur_w = ndx;
  ndx2pg(ndx,&page,&n);
  return vecslot(v,page,n);
}

void *vecSet(vec *v, uint32_t ndx, void *elem)
{
  uint32_t *p = NULL;

  p = vecGet(v,ndx);
  if (ndx >= vecCnt(v)) vecCnt(v) = ndx+1;
  if (elem) memcpy(p, elem, v->esz);
  /*else      memset(p, 0x00, v->esz);*/

  return p;
}

vec *vecShrink(vec *v,uint32_t ndx)
{
  uint16_t page, n;

  ndx2pg(ndx,&page,&n);

  while (++page < v->npg) {
     if (v->arr[page] != NULL) free(v->arr[page]);
     v->arr[page] = NULL;
  }

  if (vecCnt(v) > ndx) vecCnt(v) = ndx;
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



/**** BLK ***/

#define LNKSZ  offsetof(blkNode,elm)

vec *blkNew(uint16_t elemsz)
{
  vec *v;

  v = vecNew(elemsz + LNKSZ);
  if (v != NULL) blkFreelst(v) = NULL;

  return v;
}

void blkDel(vec *v, void *p)
{
  blkLeft(p) = blkDeleted;
  blkRight(p) = blkFreelst(v);
  blkFreelst(v) = p;
}

void *blkAdd(vec *v,void *e)
{
  blkNode *p;

  p = blkFreelst(v);

  if (p != NULL) blkFreelst(v) = blkRight(p);
  else p = vecSet(v,vecCnt(v),NULL);

  blkLeft(p) = NULL;
  blkRight(p) = NULL;
  blkSet(v,p,e);
  return p;
}

#define CURCNT(v) ((v)->aux1.n)

void *blkNext(vec *v)
{
  blkNode *p;

  while (CURCNT(v) < vecCnt(v)) {
    p = vecGet(v,CURCNT(v)++);
    if (blkRight(p) != blkDeleted) return p;
  }
  return NULL;
}

void *blkFirst(vec *v)
{
  CURCNT(v) = 0; return blkNext(v);
}

void *blkPrev(vec *v)
{
  blkNode *p;

  while (CURCNT(v) > 0) {
    p = vecGet(v,--CURCNT(v));
    if (blkRight(p) != blkDeleted) return p;
  }
  return NULL;
}



/**** MAP ***/


#define STACK(v) ((v)->aux1.p)

#include <time.h>
uint16_t rndbit()
{
  static uint32_t x = 0;
  static uint16_t n = 0;

  if (x == 0) x = (uint32_t) time(NULL);

  if (n == 0) {
    x = ((12957 * x) % 16381) >> 2;
    n = 10;
  }
  else {
    n--;
    x >>= 1;
  }

  return x & 1;
}


vec *mapNew(uint16_t elemsz, uint16_t keysz)
{
  vec *v;
  v = blkNew(elemsz);

  if (v != NULL) {
    v->ksz = keysz;
    STACK(v) = stkNew(sizeof(blkNode *));
    if (STACK(v) != NULL) {
      mapRoot(v)  = NULL;
      mapCnt(v) = 0;
    }
    else {
      free(v); v = NULL;
    }
  }

  return v;
}


/*http://en.wikipedia.org/wiki/Tree_rotation*/

static blkNode *rotRight(blkNode *pivot)
{
  blkNode *q;

  if (pivot == NULL) return NULL;
  q = blkLeft(pivot);
  if (q == NULL) return pivot;

  blkLeft(pivot) =  blkRight(q);
  blkRight(q) = pivot;
  return q;
}

static blkNode *rotLeft(blkNode *pivot)
{
  blkNode *q;

  if (pivot == NULL) return NULL;
  q = blkRight(pivot);
  if (q == NULL) return pivot;

  blkRight(pivot) =  blkLeft(q);
  blkLeft(q) = pivot;
  return q;
}

static blkNode **parent;

static blkNode *mapsearch(vec *v, blkNode *node, void *elem)
{
  int cmp;
  dbgprintf(DBG_MSG,"# mapsearch() ROOT: %p\n",node);

  stkReset(STACK(v));

  while (node != NULL) {
    stkPush(STACK(v),&parent);
    dbgprintf(DBG_MSG,"# Pushed: %p\n",parent);
    cmp = memcmp(node->elm,elem,v->ksz);
    if (cmp == 0) return node;
    parent = (cmp < 0) ? &blkRight(node)
                       : &blkLeft(node);
    node = *parent;
  }
  stkPush(STACK(v),&parent);
  return NULL;
}

void *mapAdd(vec *v, void *elem)
{
  blkNode *p,**q;

  parent = (blkNode **)&mapRoot(v);

  p = mapsearch(v,mapRoot(v),elem);
  dbgprintf(DBG_OFF,"# mapAdd() PTR: %p\n",p);
  if (p == NULL) {
    p = blkAdd(v,NULL);
    *parent = p;
    mapCnt(v)++;
    dbgprintf(DBG_MSG,"# mapAdd() ADDED: %p TO: %p\n",p,parent);

    /* ROTATE */

    while ((stkDepth(STACK(v)) > 1) && rndbit()) {
       dbgprintf(DBG_MSG,"#   Rotate ");
       q = parent;
       stkPop(STACK(v));
       parent  = stkTop(STACK(v));
       if (&blkLeft(*parent) == q)  {
        /* p is the left son of parent - rotate right*/
        *parent = rotRight(*parent);
         dbgprintf(DBG_MSG,"RIGHT\n");
       }
       else {
        *parent = rotLeft(*parent);
         dbgprintf(DBG_MSG,"LEFT\n");
       }
    }
  }
  blkSet(v,p,elem);
  return p->elm;
}

void mapDel(vec *v, void *elem) /* Only the key part will be used */
{
  blkNode *p;

  parent = (blkNode **)&mapRoot(v);

  p = mapsearch(v,mapRoot(v),elem);
  dbgprintf(DBG_MSG,"# mapDel() PTR: %p\n",p);
  if (p != NULL) {

    while (1) {
      if (blkRight(p) != NULL) {
       *parent =  rotLeft(p);
        parent = &blkLeft(*parent);
      }
      else if (blkLeft(p) != NULL) {
       *parent =  rotRight(p);
        parent = &blkRight(*parent);
      }
      else {
       *parent = NULL;
        blkDel(v,p);
        break;
      }
    }

    mapCnt(v)--;
    dbgprintf(DBG_MSG,"# mapDel() Deleted: %p FROM: %p\n",p,parent);
  }
  return;
}

void *mapGet(vec *v, void *elem)
{
  return blkElm(mapsearch(v,mapRoot(v),elem));
}

void *mapFirst(vec *v)
{
  return mapFirstSorted(v);

 #if 0
  blkNode *p;
  blkNode *q;

  p = mapRoot(v);
  stkReset(STACK(v));

  if (p == NULL)  return NULL;

  if ((q = blkRight(p)) != NULL) stkPush(STACK(v),&q);
  if ((q = blkLeft(p))  != NULL) stkPush(STACK(v),&q);
  dbgprintf(DBG_OFF,"(STACK: %p) PARENT: %p LEFT:%p RIGHT: %p\n",STACK(v),p,blkLeft(p),blkRight(p));
  return p->elm;
 #endif
}

void *mapNext(vec *v)
{
  return mapNextSorted(v);
 #if 0
  blkNode *p,*q;

  if (stkIsEmpty(STACK(v))) return NULL;
  dbgprintf(DBG_OFF,"mapNext() STACK: %p\n",STACK(v));

  p = *((blkNode **)stkTop(STACK(v)));
  stkPop(STACK(v));

  if ((q = blkRight(p)) != NULL) stkPush(STACK(v),&q);
  if ((q = blkLeft(p))  != NULL) stkPush(STACK(v),&q);
  dbgprintf(DBG_OFF,"(STACK: %p) PARENT: %p LEFT:%p RIGHT: %p\n",STACK(v),p,blkLeft(p),blkRight(p));
  return p->elm;
 #endif
}

static blkNode *mapgoleft(vec *v,blkNode *p)
{
  while (blkLeft(p) != NULL) {
    stkPush(STACK(v),&p);
    p = blkLeft(p);
  }
  return p;
}

void *mapFirstSorted(vec *v)
{
  blkNode *p,*q;

  p = mapRoot(v);
  stkReset(STACK(v));

  if (p == NULL)  return NULL;

  p = mapgoleft(v, p);

  if ((q = blkRight(p)) != NULL) {
    q = mapgoleft(v,q);
    stkPush(STACK(v),&q);
  }

  return p->elm;

}

void *mapNextSorted(vec *v)
{
  blkNode *p,*q;

  if (stkIsEmpty(STACK(v))) return NULL;

  p = *((blkNode **)stkTop(STACK(v)));
  stkPop(STACK(v));

  if ((q = blkRight(p)) != NULL) {
    q = mapgoleft(v,q);
    stkPush(STACK(v),&q);
  }

  return p->elm;
}

void *mapFree(vec *v)
{
  if (STACK(v) != NULL) stkFree(STACK(v));
  return vecFree(v);
}


