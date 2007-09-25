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
@inproceedings{ brodnik99resizable,
    author = "Andrej Brodnik and Svante Carlsson and Erik D. Demaine and J. Ian Munro and Robert Sedgewick",
    title = "Resizable Arrays in Optimal Time and Space",
    booktitle = "Workshop on Algorithms and Data Structures",
    pages = "37-48",
    year = "1999",
    url = "citeseer.ist.psu.edu/brodnik99resizable.html"
}

@Article{Demaine:2001:AAF,
  author =       "Erik Demaine",
  title =        "Algorithm Alley: Fast and Small Resizable Arrays",
  journal =      j-DDJ,
  volume =       "26",
  number =       "7",
  pages =        "132--134",
  month =        jul,
  year =         "2001",
  CODEN =        "DDJOEB",
  ISSN =         "1044-789X",
  bibdate =      "Thu Jun 7 06:07:17 MDT 2001",
  URL =          "http://www.ddj.com/architect/184404698",
}
*/

#include "vec.h"

static const char *errNOMEM = "Out of Memory (vec)";
static const char *errUNEXP = "Unexpected error";

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

#define two_raised(n) (1<<(n))
#define div2(n)       ((n)>>1)

static uint32_t pgsize(uint32_t p)
{
  uint32_t k;
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

static void ndx2pg(uint32_t ndx, uint32_t *p, uint32_t *n)
{
  uint32_t k,m,s;
  
  m  = ndx +1;
  k  = llog2(m);              /* compute MSB */
  m ^= two_raised(k);         /* clear MSB */
  s  = div2(k+1);             /* ceil(k/2)  */
  k  = div2(k);               /* floor(k/2)  */
 *p  = (two_raised(s) - 1) +  /* Sum for i=0 to ceil(k/2 - 1)  of 2^i */
       (two_raised(k) - 1) +  /* Sum for i=0 to floor(k/2 - 1) of 2^i */
       (m >> s);  
 *n  = m & (two_raised(s) - 1);
  dbgprintf(DBG_MSG,"ndg2pg ->  %4u %4u %4u (%4u)\n",ndx,*p, *n,pgsize(*p));
  
}


static void vecInit(vec *v,uint16_t elemsize)
{
  if (v) {
    /* Ensure an element can store a 32 bit integer, will be used for the freelist */
    if (elemsize < sizeof(uint32_t)) elemsize =  sizeof(uint32_t);
    v->esz    = elemsize;
    v->ksz    = v->esz;
    v->npg    = 0;
    vecCnt(v) = 0;
    v->arr    = NULL;
    v->mrk_p  = (uint16_t)-1;
    v->mrk_n  = (uint16_t)-1;
    v->mrk_w  = VEC_ANYNDX;
    v->mrk_q  = ((uint8_t *)0 - elemsize);
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
    dbgprintf(DBG_OFF,"\t *X* %d %d %d\n",page,v->npg,llog2(page+1));

    t = v->npg;
   /* The smallest power of two greater then |page| */ 
    v->npg = (page == 0)? 1 : 1 << (llog2(page) +1) ; 
    
    dbgprintf(DBG_OFF,"\t === %d ",v->npg);
    
    v->arr = realloc(v->arr, v->npg * sizeof(void *));
    if (v->arr == NULL) err(3001,errNOMEM);
    while (t < v->npg)  v->arr[t++] = NULL;
    
    dbgprintf(DBG_OFF,"\t *** %d \n",v->npg);
  }

  if (page != v->mrk_p) v->mrk_s = pgsize(page);
  
  if (v->arr[page] == NULL) {
    v->arr[page] = calloc(v->mrk_s, v->esz); /* page is guaranteed to be filled with 0's */
    if (v->arr[page] == NULL)  err(3002,errNOMEM);
  }

  p = v->arr[page] + (n * v->esz);

  v->mrk_p = page;
  v->mrk_n = n;
  v->mrk_q = p;

  return p;
}

#define vecMark(v,m,o)  do {if (o=='r') (v)->mrk = m; else  m=(v)->mrk;} while(0)

void *vecGet(vec *v,uint32_t ndx)
{
  uint32_t page, n;

  if (v == NULL)  err(3003,errNOMEM);
  
  if (ndx == VEC_ANYNDX) ndx = 0;
  
  if (v->mrk_w != VEC_ANYNDX) {
    switch ((int)(ndx - v->mrk_w)) {
      case  0 : return v->mrk_q;
      
      case  1 : v->mrk_w++;
                if (++v->mrk_n >= v->mrk_s) return vecslot(v,v->mrk_p+1,0);
                v->mrk_q += v->esz;
                return v->mrk_q;
    }
  }
  
  v->mrk_w = ndx;
  ndx2pg(ndx,&page,&n);
  return vecslot(v,page,n);
}

void *vecPrev(vec *v)
{
  if (v == NULL) err(3005,errNOMEM);
  if (v->mrk_w == 0) return NULL;
  if (v->mrk_n == 0) return vecGet(v,v->mrk_w - 1);
  
  v->mrk_n--;
  v->mrk_w--;
  v->mrk_q -= v->esz;
  return v->mrk_q;
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
  uint32_t page, n;

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
#define FREELST(v) ((v)->aux0.p)

vec *blkNew(uint16_t elemsz)
{
  vec *v;
  
  v = vecNew(elemsz);
  if (v != NULL) FREELST(v) = NULL;
  
  return v;
}

void blkDel(vec *v, void *e)
{
  *((void **)e) = FREELST(v);
  FREELST(v) = e;
}

void *blkAdd(vec *v,void *e)
{
  void **p;
  
  p = FREELST(v);
  
  if (p != NULL) FREELST(v) = *p;
  else p = vecNext(v);
  if (e != NULL) memcpy(p,e,v->esz);
  return p;
}

/**** MAP ***/

typedef struct node {
  struct node *lnk[2];
  uint8_t      elm[sizeof(uint32_t)];
} node;


#define LNKSZ  offsetof(node,elm)
#define STACK(v) ((v)->aux1.p)

vec *mapNew(uint16_t elemsz, uint16_t keysz)
{
  vec *v;
  
  v = blkNew(elemsz+LNKSZ);
  
  if (v != NULL) {
    v->ksz = keysz;
    STACK(v) = NULL;
  }
  
  return v; 
}

#define CMP(v,n,k) (memcmp((n)->elm,k,(v)->ksz))
#define CPY(v,n,e) (memcpy((p)->elm,e,(v)->esz))
#define LEFT(n)  ((n)->lnk[0])
#define RIGHT(n) ((n)->lnk[1])
#define ROOT(v)  ((v)->aux2.p)

static node **parent;
static int cmp;

static node *mapsearch(vec *v, node *root, void *elem)
{
  if (root == NULL) return NULL;
  
  cmp = CMP(v,root,elem);
  if (cmp == 0) return root;
  if (cmp < 0) {
    parent = &RIGHT(root);
    return mapsearch(v,RIGHT(root),elem);
  }
  parent = &LEFT(root);
  return mapsearch(v,LEFT(root),elem);
}

void *mapAdd(vec *v, void *elem)
{
  node *p;
 
  parent = (node **)&ROOT(v);
  
  p = mapsearch(v,ROOT(v),elem); 
  if (p == NULL) {
    p = blkAdd(v,NULL);
    LEFT(p) = NULL; RIGHT(p) = NULL;
    *parent = p;
  }
  CPY(v,p,elem); /* overwrite if key already present */
  return p->elm;
}

void *mapGet(vec *v, void *elem)
{
  node *p = mapsearch(v,ROOT(v),elem); 
  if (p != NULL) return p->elm;
  return NULL;
}

void *mapFirst(vec *v)
{
  node *p;
  
  p = ROOT(v);
  if (p == NULL) {
    if (STACK(v) != NULL) stkReset((vec *)STACK(v));
    return NULL;
  }

  if (STACK(v) == NULL) STACK(v) = stkNew(sizeof(node *));
  stkReset(STACK(v));
    
  if (p != NULL) {
    if (RIGHT(p)) stkPush(STACK(v),RIGHT(p));
    if (LEFT(p)) stkPush(STACK(v),LEFT(p));
    return p->elm;
  }
  return NULL;
}

void *mapNext(vec *v)
{
  node *p;
  if (stkIsEmpty(STACK(v))) return NULL;
  
  p = stkTop(STACK(v));
  stkPop(STACK(v));
  if (RIGHT(p)) stkPush(STACK(v),RIGHT(p));
  if (LEFT(p)) stkPush(STACK(v),LEFT(p));
  return p->elm;
}

