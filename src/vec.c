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
  k  = llog2(div2(p));
  if (p >= 3* two_raised(k)) k++;
  return two_raised(k);
}

static void ndx2pg(uint32_t ndx, uint32_t *p, uint32_t *n)
{
  uint32_t k,m,s;
  
  m  = ndx +1;
  k  = llog2(m);              /* compute MSB */
  m ^= two_raised(k);         /* clear MSB */
  s  = div2(k+1);             /* ceil(k/2)  */
  k  = div2(k);               /* floor(k/2)  */
 *p  = (two_raised(s) - 1) + (two_raised(k) - 1) + (m >> s);  /* Sum of two seqences of 2^j */
 *n  = m & (two_raised(s) - 1);
  dbgprintf(DBG_OFF,"ndg2pg ->  %4u %4u %4u (%4u)\n",ndx,*p, *n,pgsize(*p));
  
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

/**** MAP ***/

#define AUX(v) ((vec *)(((vec *)v)->aux))

#define KEY(v,e) *((uint32_t *)(((uint8_t *)e) + ((v)->esz) - sizeof(uint32_t)))

vec *mapNew(uint16_t elemsize,uint16_t keysize)
{
  vec *m = vecNew(elemsize);
  if (m != NULL){
    m->ksz = keysize;
    m->aux = vecNew(sizeof(void *));
    if (m->aux == NULL) {
      m = vecFree(m);
    }
    else {
      AUX(m)->cnt = 4;
    }
    
  }
  return m;
}

/*
** Fowler/Noll/Vo hash
** By:  chongo <Landon Curt Noll>  http://www.isthe.com/chongo/
** See: http://www.isthe.com/chongo/tech/comp/fnv/index.html
*/
 
#define FNV1_32_INIT ((uint32_t)0x811c9dc5)
#define FNV_32_PRIME ((uint32_t)0x01000193)

uint32_t fnv(void *buf, size_t len)
{
  uint8_t *bp = (uint8_t *)buf;
  uint32_t hval = FNV1_32_INIT;

  while (len-- > 0) {
    hval ^= (uint32_t)*bp++;
    hval *= FNV_32_PRIME;
  }
  return hval;
}

/*
** This is a linear probing hash.
** Max probes is N, if a chain is longer than N the table is doubled and reashed
** If after rehash it's still impossible to insert within N, N is doubled
** If still it's impossible, we'll exit with an error
*/

/* I just need a pointer that is different from NULL and can't be
** a pointer to an element. Pointing to a function will do the trick.
*/
#define DELETED ((void *)fnv)
static uint16_t maxchn = 4;

#define MSK(v) (AUX(v)->cnt - 1)



/*
                        AUX       MAP
           ,-----.    +-----+   +-----+
   slot -> | xx -|--> | zz -|-->|     |
           `-----'    +-----+   +-----+

*/

static void *mapSearch(vec *m, void *elem, void ***slot)
{
  void *p = NULL;
  uint32_t key;
  uint32_t k;
  uint16_t d;
  
  key = fnv(elem,m->ksz);
  k = key;
  d = 0;
  while (1) {
    k &= MSK(m);
    *slot = vecGet(AUX(m),k);
    if (*slot == NULL) return NULL;
    
    p  = *(*slot);
    if (p == NULL) return NULL;
    
    if ((KEY(m,p) == key) && (memcmp(p,elem,m->ksz) == 0)) return p;
    if (++d >= maxchn) {
      *slot = NULL;  /* Need to rehash if want to insert */
      return NULL;
    }
    k++;
  }
}

void *mapAdd(vec *m,void *elem)
{
  void *p = NULL;
  void **q;
  uint32_t key;
  
  key = fnv(elem,m->ksz);

  p = mapSearch(m,elem,&q);
  if (p == NULL) {
    KEY(m,elem) = key;
  }
  
  return p;
}
