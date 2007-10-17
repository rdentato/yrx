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

For a discussion on MSB calculation, see section 8 of:

@misc{ bagwell-fast,
  author = "Phil Bagwell",
  title = "Fast Functional Lists, Hash-Lists, Deques and Variable Length Arrays",
  url = "citeseer.ist.psu.edu/bagwell02fast.html" }

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

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

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

/* Ensure an element can store a (void *). Will be used for the freelist */
#define MINSZ sizeof(void *)

static void vecinit(vec *v,uint16_t elemsize)
{
  if (v != NULL) {
    /*if (elemsize < MINSZ) elemsize = MINSZ;*/
    v->esz    = elemsize;
    v->aux    = 0;
    v->npg    = 0;
    v->pgs    = NULL;
    v->cnt    = 0;
    v->cur_p  = (uint16_t)-1;
    v->cur_n  = (uint16_t)-1;
    v->cur_w  = VEC_NULLNDX;
    v->cur_q  = ((uint8_t *)0 - elemsize);
  }
}

vec *vecNew(uint16_t elemsize)
{
  vec *v = NULL;

  if (elemsize > 0 && elemsize < UINT16_MAX) {
    v = malloc(sizeof(vec));
    vecinit(v,elemsize);
    dbgprintf(DBG_OFF,"VEC: %p\n", v);
  }
  return v;
}

static void veccleanup (vec *v,vecCleaner cln)
{
  uint16_t i,k;
  uint8_t *p;

  if (v != NULL) {
    if (v->pgs != NULL) {
      for (i = 0; i< v->npg; i++) {
        dbgprintf(DBG_OFF,"#  %d %p\n",i,v->pgs[i]);
        if (v->pgs[i] != NULL) {
          if (cln != NULL) {
            for (k=0, p=v->pgs[i] ; k<pgsize(i);k++, p += v->esz)
              if (p != NULL) cln(p);
          }
          free(v->pgs[i]);
        }
        v->pgs[i] = NULL;
      }
      free(v->pgs);
      v->pgs = NULL;
    }
  }
}

void *vecFreeClean(vec *v,vecCleaner cln)
{
  if (v != NULL) {
    veccleanup(v, cln);
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

    v->pgs = realloc(v->pgs, v->npg * sizeof(void *));
    if (v->pgs == NULL) err(3001,"Unable to allocate page index for (%u)\n",page);
    while (t < v->npg)  v->pgs[t++] = NULL;

    dbgprintf(DBG_OFF,"\t *** %d \n",v->npg);
  }

  if (page != v->cur_p) v->cur_s = pgsize(page);

  if (v->pgs[page] == NULL) {
    v->pgs[page] = calloc(v->cur_s, v->esz); /* page is guaranteed to be filled with 0's */
    if (v->pgs[page] == NULL)  err(3002,errNOMEM);
    dbgprintf(DBG_OFF,"#  page %d 0x%p\n",page,v->pgs[page]);
  }

  p = v->pgs[page] + (n * v->esz);

  v->cur_p = page;
  v->cur_n = n;
  v->cur_q = p;

  return p;
}

void *vecGet(vec *v,uint32_t ndx)
{
  uint16_t page, n;

  if (v == NULL)  err(3003,errNOMEM);

  if (ndx == VEC_NULLNDX) ndx = 0;

  if (v->cur_w != VEC_NULLNDX) {
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
  if (elem) memcpy(p, elem, v->esz);
  /*else      memset(p, 0x00, v->esz);*/
  if (ndx >= v->cnt) v->cnt = ndx+1;

  return p;
}

vec *vecShrink(vec *v,uint32_t ndx)
{
  uint16_t page, n;

  ndx2pg(ndx,&page,&n);

  while (++page < v->npg) {
     if (v->pgs[page] != NULL) free(v->pgs[page]);
     v->pgs[page] = NULL;
  }
  /* TODO: Resize pgs to the closest power of 2 */

  return v;
}

uint32_t vecSize(vec *v)
{
  uint16_t k;
  uint32_t size = 0;

  if (v != NULL) {
    for (k=0; k < v->npg; k++) {
      if (v->pgs[k] != NULL) {
        size += pgsize(k) * v->esz;
      }
    }
    size += v->npg * sizeof(uint8_t *);
    size += sizeof(vec);
  }
  return size;
}

/*******/
static char *emptystr = "";

#define bufBlkSize 32
#define bufBlkMask (bufBlkSize-1)

buf_t bufNew()
{
  vec *b;

  if ((b = vecNew(bufBlkSize)) != NULL)
    b->aux = bufBlkSize;
  return b;
}

uint32_t bufPutc(vec *b,char c)
{
  uint8_t *s;
  uint32_t n;

  if (b->aux >= bufBlkSize) {
    b->aux = 0;
    s = vecNext(b);
  }
  else
    s = b->cur_q;

  s[b->aux] = c;

  n = b->cur_w * bufBlkSize + b->aux;
  b->aux++;
  if (b->cnt >= n) b->cnt = n+1;

  return n;
}

uint32_t bufPuts(vec *b, char *s)
{
  uint32_t n;
  while (*s != '\0') bufPutc(b,*s++);
  n = bufPutc(b,'\0');
  return n;
}

uint32_t bufPos(vec *b)
{
  if (b->cur_w == VEC_NULLNDX) return VEC_NULLNDX;
  return b->cur_w * bufBlkSize + b->aux;
}

uint32_t bufSeek(vec *b,uint32_t pos)
{
  if (pos != VEC_NULLNDX) {
    if (pos >= b->cnt) b->cnt = pos+1;
    vecGet(b, pos / bufBlkSize);
    b->aux = (uint16_t)(pos & bufBlkMask);
  }
  return pos;
}

int bufGetc(vec *b)
{
  char *s = emptystr;
  uint32_t w = b->cur_w;

  if ((b->cnt == 0) || (w * bufBlkSize + b->aux >= b->cnt))
    return EOF;

  if (b->aux >= bufBlkSize) {
    b->aux = 0;
    w++;
  }
  s = (char *)vecGet(b, w) + b->aux;
  b->aux++;

  return (int)(*s);
}

int bufGets(vec *b,char *s)
{
  int c,n=0;
  while ((c = bufGetc(b)) != EOF && c) {
   *s++ = c;
    n++;
  }
  *s = '\0';
  return n;
}


/*******/


static uint8_t *bmpbyte(vec *b, uint32_t ndx, uint8_t *i)
{
  uint32_t k;
  uint8_t *p;

  if (ndx >= b->cnt) b->cnt = ndx+1;
 *i   = 1 << (ndx % 8);
  ndx = ndx / 8;
  k   = ndx & bmpBlkMask;
  ndx = ndx / bmpBlkSize;
  p = vecGet(b, ndx);
  if (p != NULL) p += k;
  return p;
}

uint8_t bmpSet(vec *b,uint32_t ndx)
{
  uint8_t *p,i;

  if ((p = bmpbyte(b,ndx,&i)) != NULL)
    *p |= i;
  return 1;
}

uint8_t bmpClr(vec *b,uint32_t ndx)
{
  uint8_t *p,i;

  if ((p = bmpbyte(b,ndx,&i)) != NULL)
    *p &= ~i;
  return 0;
}

uint8_t bmpTest(vec *b,uint32_t ndx)
{
  uint8_t *p,i=0;

  if ((p = bmpbyte(b, ndx,&i)) != NULL)
    i &= *p;
  return i;
}

uint8_t bmpFlip(vec *b,uint32_t ndx)
{
  uint8_t *p,i=0;

  if ((p = bmpbyte(b, ndx, &i)) != NULL) {
   *p ^= i;
    i &= *p;
  }
  return i;
}


/*******/

/* http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx */

#define FNV_CALC        h = (h * 16777619) ^ p[i];

static uint32_t fnv_hash (void *e, uint16_t len)
{
  uint8_t *p = e;
  uint32_t h = 2166136261;
  int i = 0;
 
  if (len == 0) 
    while (p[i]) { FNV_CALC }
  else
    for (i=0; i<len; i++) { FNV_CALC }

  return h;
}

#define OAT_CALC { h += p[i];        \
                   h += ( h << 10 ); \
                   h ^= ( h >> 6 );  \
                 }                   \
                 h += ( h << 3 );    \
                 h ^= ( h >> 11 );   \
                 h += ( h << 15 );   
 
static uint32_t oat_hash(void *e, uint16_t len)
{ 
  uint8_t *p = e; 
  uint32_t h = 0; 
  int i; 

  if (len == 0) 
    while (p[i]) { OAT_CALC }
  else
    for (i=0; i<len; i++) { OAT_CALC }

  return h; 
} 

/********/

typedef struct stp {
  vec        str[1];
  uint32_t   msk;
} stp;

typedef stp *stp_t;

stp_t stpNew(void);
char *stpAdd(stp_t pool, char *str);


stp_t stpNew()
{
  stp_t pool;
 
  if ((pool = malloc(sizeof(stp))) != NULL) {
    vecinit(pool->str,sizeof(char *));
    pool->msk = 0x01;
  }

  return pool;
}

void *stpFree(stp_t pool)
{
  if (pool != NULL) {
    veccleanup(pool->str,free);
    free(pool);
  }
  return NULL;
}


#define TOODENSE(p)  ( (((p)->msk - (p)->str->cnt) * 4) < (p)->msk )
#define TOOSPARSE(p) ( ((p)->str->cnt * 2) < ((p)->msk -1))

static char **stpsearch(stp_t pool, char *str, char ***del);


static void stpreash(stp_t pool)
{
  uint32_t k,max;
  char **p,**s, **d;
  char  *t;

  max = pool->msk+1;

  pool->msk = 2 * pool->msk +1;
  for (k=0; k < max; k++) {
    p = vecGet(pool->str,k);
    t = *p;
    if (t != NULL && t != VEC_DELETED) {
      *p = VEC_DELETED;
      d = NULL;
      s = stpsearch(pool, t, &d);
      if (s != NULL) {
        if (d != NULL) s = d;
        *s = t;
      }
    }
  }

}

static char **stpsearch(stp_t pool, char *str, char ***del)
{
  uint32_t key;
  uint32_t inc;
  char **s;

  key = fnv_hash(str,0) & pool->msk;

  inc = oat_hash(str,0) | 1; /* ensure increment is odd */

  while (1) {
    s = vecGet(pool->str,key);

    if (s == NULL || *s == NULL) return s;

    if (*s == VEC_DELETED) {
      if (del == NULL) return s;
     *del = s;
    }
    else if (strcmp(str,*s) == 0)
      return s;

    key = (key + inc) & pool->msk;    
  }
  return NULL;

}

char *stpAdd(stp_t pool, char *str)
{
  char **s,**d;

  if (TOOFULL(pool)) stpreash(pool);

  d = NULL;
  s = stpsearch(pool, str, &d);

  if (s == NULL) return NULL;

  if (*s == NULL) {
    if (d != NULL) s = d;
    *s = strdup(str);
  }

  return *s;
}

char *stpGet(stp_t pool, char *str)
{
  char **s;
 
  s = stpsearch(pool, str, NULL);

  if (s == NULL || *s == NULL || *s == VEC_DELETED) return NULL;
 
  return *s;  
}

