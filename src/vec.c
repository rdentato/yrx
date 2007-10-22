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
/*static const char *errUNEXP = "Unexpected error";*/

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
        dbgoff("#  %d %p\n",i,v->pgs[i]);
        if (v->pgs[i] != NULL) {
          if (cln != NULL) {
            for (k=0, p=v->pgs[i];  k<pgsize(i);  k++, p += v->esz)
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

  if (page != v->cur_p) 
    v->cur_s = pgsize(page);

  if (v->pgs[page] == NULL) {
    v->pgs[page] = calloc(v->cur_s, v->esz); /* page is guaranteed to be filled with 0's */
    if (v->pgs[page] == NULL)  err(3002,errNOMEM);
    dbgoff(" ]]]  page %d 0x%p\n", page, v->pgs[page]);
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
  uint32_t h = 0x811C9DC5;
  int i = 0;

  if (len == 0)
    for (i=0; p[i];  i++) { FNV_CALC }
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
  int i=0;

  dbgoff("** %p %u\n",e,len);

  if (len == 0)
    for (i=0; p[i];  i++) { OAT_CALC }
  else
    for (i=0; i<len; i++) { OAT_CALC }

  return h;
}

#define ROT_CALC h = ( h << 4 ) ^ ( h >> 28 ) ^ p[i];

static uint32_t rot_hash(void *e, uint16_t len)
{
  uint8_t *p = e;
  uint32_t h = 0;
  int i=0;

  if (len == 0)
    for (i=0; p[i];  i++) { ROT_CALC }
  else
    for (i=0; i<len; i++) { ROT_CALC }

  return h;
}
/********/

#define STP_REASHING ((void *)stpNew)

stp_t stpNew()
{
  stp_t pool;

  if ((pool = malloc(sizeof(stp))) != NULL) {
    vecinit(pool->str,sizeof(char *));
    pool->msk = 3;
  }

  return pool;
}


static void stpfree(void *e)
{
  char **s=e;
  if (s != NULL && *s != VEC_DELETED && *s != NULL) {
    dbgoff("-- %s\n",*s);
    free(*s);
  }
}

void *stpFree(stp_t pool)
{
  if (pool != NULL) {
    pool->str->cnt = pool->msk+1; /* pretend the table is full */
    veccleanup(pool->str,stpfree);
    free(pool);
  }
  return NULL;
}

#define stpN(p)  ((p)->msk+1)
#define stpM(p)  ((p)->str->aux)

#define TOODENSE(p)  ( (stpM(p) > 30) || \
                       ((uint32_t)((1 << (stpM(p) >> 1))) > stpCnt(p)) || \
                       ((stpN(p) - stpCnt(p)) * 16) < stpN(p))

/*#define TOOSPARSE(p) ( (stpCnt(p) * 2) < (stpN(p)))*/

static char **stpsearch(stp_t pool, char *str, char ***del);


#define ndx_hash oat_hash
#define inc_hash rot_hash

#define DOUBLE_SIZE  0
#define HALF_SIZE    1

static void stprehash(stp_t pool,int sz)
{
  uint32_t k,max;
  char **p,**s, **d;
  char  *t;
  uint32_t key;
  uint32_t inc;

  stpM(pool) = 0;
  max = pool->msk+1;
  pool->msk = 2 * pool->msk +1;
  dbgmsg(" ***** Rehash: (%u) %u,%u\n",stpCnt(pool),max,pool->msk);
  k = 0;
  d = NULL;
  while (k < max) {
    p = vecGet(pool->str, k);
    if (*p == VEC_DELETED) *p = NULL;
    if (*p != NULL) {
      t = *p;
      key = ndx_hash(t,0) & pool->msk;
      if (key != k) {
       *p = NULL;
        s = vecGet(pool->str, key);
        if (*s != NULL && *s != VEC_DELETED) {
          if ( key < max && s != d) {
           *p = *s; 
            k--;
          }
          else {
            inc = (inc_hash(t,0) ) | 1;
            do {
              key = (key+inc) & pool->msk;
              s = vecGet(pool->str, key);
              /*dbgif((*s != NULL && *s != VEC_DELETED),dbgmsg("  ++ %u %s %s\n",key,t,*s));*/
            } while (*s != VEC_DELETED && *s != NULL);
          }
        }
       *s = t;
        d = s; /* store last */
      }
    }
    k++;
  }
}

static char **stpsearch(stp_t pool, char *str, char ***del)
{
  uint32_t key;
  uint32_t inc;
  char **s;
  char *t;

  inc = (inc_hash(str,0) ) | 1;  /* ensure increment is odd */
  key = ndx_hash(str,0) & pool->msk;
  stpM(pool) = 0;
  while (1) {
    s = vecGet(pool->str,key);

    if (s == NULL || *s == NULL) return s;

    if (*s == VEC_DELETED) {
      if (del != NULL) {
        if (*del == STP_REASHING) {
          *del = s;
          return s;
        }
       *del = s;
      }
    }
	  else {
	    if (strcmp(str,*s) == 0)   return s;
    }
    t = *s;
    if (t == VEC_DELETED) t = "(del)";
    else if (t == NULL) t = "(null)";
    dbgmsg("XX %4d (%u) %s (%s)\n",key,inc, str, t);
    stpM(pool)++;
    key = (key + inc) & pool->msk;
  }
  return NULL;

}

char *stpAdd(stp_t pool, char *str)
{
  char **s,**d;

  if (TOODENSE(pool)) stprehash(pool,DOUBLE_SIZE);

  dbgmsg("ADD: %s\n", str);
  d = NULL;
  s = stpsearch(pool, str, &d);

  if (s == NULL) return NULL;

  if (*s == NULL) {
    if (d != NULL) s = d;
    *s = strdup(str);
    stpCnt(pool)++;
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

char *stpDel(stp_t pool, char *str)
{
  char **s,**d;
  d = NULL;
  s = stpsearch(pool, str, &d);

  if (s != NULL && (*s != NULL || *s != VEC_DELETED)) {
    free(*s);
    *s = VEC_DELETED;
    stpCnt(pool)--;
  }
  return NULL;
}

/**************************/

static uint8_t T[] = { 0x5B, 0x06, 0xED, 0x30, 0x34, 0xCB, 0x01, 0x99, 
                       0x2B, 0x68, 0x93, 0x16, 0x90, 0xC8, 0x82, 0xC6, 
                       0xB8, 0xDC, 0x3A, 0x66, 0xB7, 0x04, 0x8A, 0xC7, 
                       0x8E, 0xA8, 0xA4, 0x29, 0xE3, 0xD4, 0xFC, 0x6E, 
                       0xC9, 0x1C, 0x3B, 0xF3, 0x3C, 0xAB, 0x56, 0xAE, 
                       0x63, 0xB2, 0x8C, 0xBC, 0x70, 0x6F, 0x2F, 0xDD, 
                       0x1E, 0xCF, 0x98, 0x1D, 0xFF, 0x37, 0x38, 0x4F, 
                       0x3D, 0x6D, 0x03, 0x46, 0x67, 0x09, 0xD7, 0xCE, 
                       0xD1, 0xB1, 0x9C, 0xEE, 0x4A, 0x9B, 0x84, 0x0E, 
                       0x3E, 0xBE, 0xA6, 0x91, 0x35, 0xBF, 0xFB, 0x20, 
                       0xA0, 0xD8, 0x96, 0x12, 0x1A, 0xE1, 0xF1, 0x9F, 
                       0x14, 0xDF, 0x53, 0x25, 0xEB, 0x7D, 0x80, 0x73, 
                       0x9A, 0x7A, 0x58, 0x88, 0x5C, 0xE2, 0x43, 0xAD, 
                       0x05, 0x18, 0xD9, 0x2E, 0xD3, 0xCD, 0xCA, 0xA7, 
                       0x65, 0x1F, 0x79, 0x89, 0x83, 0xB5, 0x32, 0x41, 
                       0x28, 0x31, 0x92, 0x02, 0x9D, 0x2C, 0x0C, 0x10, 
                       0x76, 0x08, 0x74, 0xDB, 0x4D, 0xFA, 0x8D, 0xA3, 
                       0x07, 0x87, 0x78, 0xFE, 0x60, 0x8F, 0xF4, 0x4E, 
                       0x21, 0x7E, 0x9E, 0xBD, 0xA9, 0x86, 0x0A, 0x5D, 
                       0xEC, 0xA1, 0xB9, 0x00, 0x71, 0x57, 0x33, 0x62, 
                       0x4B, 0x55, 0xD0, 0x54, 0x50, 0x1B, 0xE8, 0x15, 
                       0x94, 0xDA, 0xE6, 0xFD, 0xBB, 0x7B, 0x13, 0x72, 
                       0xD2, 0x77, 0xAC, 0x42, 0xA5, 0x69, 0x48, 0x22, 
                       0xB6, 0x59, 0x5F, 0x24, 0x52, 0x40, 0x45, 0xC3, 
                       0xF5, 0xB3, 0x47, 0x6A, 0xF9, 0xAF, 0x75, 0xCC, 
                       0xE0, 0xEA, 0x4C, 0xBA, 0xDE, 0x49, 0xC1, 0xB4, 
                       0x85, 0xE5, 0x51, 0x23, 0xE7, 0x6C, 0x7C, 0xF0, 
                       0xF7, 0xF8, 0x0D, 0xA2, 0xC2, 0x5E, 0x19, 0x44, 
                       0x81, 0x7F, 0xAA, 0xC5, 0x6B, 0xD5, 0x11, 0xC0, 
                       0x95, 0xE4, 0x2A, 0x0F, 0x36, 0xEF, 0xF2, 0xD6, 
                       0xC4, 0x17, 0x97, 0x26, 0x0B, 0xE9, 0x2D, 0x8B, 
                       0x27, 0xB0, 0x3F, 0x39, 0x5A, 0x64, 0xF6, 0x61, 
};

static uint32_t mappriority(mapNode *node)
{
  uint8_t *p,*q;
  uint32_t h;

  p = (uint8_t *)(&node);
  q = (uint8_t *)(&h);

  q[0] = p[3] ^ T[p[1]];
  q[1] = p[2] ^ T[p[0]];
  q[2] = p[0] ^ T[p[3]];
  q[3] = p[1] ^ T[p[2]];

  return h;
}

map_t mapNew(uint16_t elemsz, uint16_t keysz)
{
  map_t m;

  m = malloc(sizeof(mapVec));
  if (m != NULL) {
    vecinit(m->nodes,elemsz + offsetof(mapNode, elem));
    vecinit(m->stack,sizeof(mapNode *));
    m->root = NULL;
    m->freelst = NULL;
    mapKeySz(m) = keysz;
    m->cnt = 0;
  }

  return m;
}

map_t mapFree(map_t m)
{
  if (m != NULL) {
    veccleanup(m->nodes, NULL);
    veccleanup(m->stack, NULL);
    free(m);
  }
  return NULL;
}

static mapNode *mapnewnode(map_t m)
{
  mapNode *p;

  if (m->freelst != NULL) {
    p = m->freelst;
    m->freelst = mapRight(p);
  }
  else {
    p = vecSet(m->nodes, vecCnt(m->nodes),NULL);
  }
  mapLeft(p) = NULL;
  mapRight(p) = NULL;
  mapCnt(m)++;
  return p;
}


/*http://en.wikipedia.org/wiki/Tree_rotation*/


static mapNode *maprotate(mapNode *pivot,uint8_t direction)
{
  mapNode *q;
  int8_t opposite;

  if (pivot == NULL) return NULL;

  direction &= 1;
  opposite   = direction ^ 1;

  q = pivot->lnk[opposite];
  if (q == NULL) return pivot;

  pivot->lnk[opposite] =  q->lnk[direction];
  q->lnk[direction] = pivot;
  return q;
}

#define maprotleft(p)   maprotate(p,0);
#define maprotright(p)  maprotate(p,1);

static mapNode *mapsearch(map_t m, mapNode ***par,  void *elem)
{
  int cmp;
  mapNode  *node;
  mapNode **parent = *par;

  dbgoff("mapsearch() ROOT: %p\n",*parent);

  stkReset(m->stack);

  do {
    stkPush(m->stack, &parent);  dbgoff("Pushed: %p (%p)\n",parent, *parent);

    node = *parent;
    if (node == NULL) break;   dbgoff("Not found\n");

    cmp = memcmp(elem, node->elem, mapKeySz(m));
    if (cmp == 0) break;   dbgoff("Found: %p\n",node);

    parent = (cmp > 0)? &mapRight(node) : &mapLeft(node);

  } while (1);

  *par = parent;

  return node;
}

static void mapbalance(map_t m)
{
  mapNode **p,**q;
  uint8_t direction;
  uint32_t nodepri,pri;

  dbgmsg("Count: %d Depth: %d Limit: %d\n",m->cnt, m->stack->cnt,1+llog2(m->cnt));
  p = stkTopVal(m->stack,mapNode **);
  nodepri = mappriority(*p);
  while (1) {
    stkPop(m->stack);
    dbgmsg("    TOP: Parent: %p  Node: %p direction: %d",p,*p,direction);
    q = stkTopVal(m->stack,mapNode **);
    if (q == NULL) break;

    direction = 0;
    if (p == &(mapRight(*q))) direction++;

    dbgmsg(" nodepri %x pri %x\n", nodepri, mappriority(*q));
    if (nodepri < mappriority(*q))
       *q = maprotate(*q,direction);
    else 
       break;

    p = q;
  } 
  printf("\n");
}

void *mapAdd(map_t m, void *e)
{
  mapNode *node;
  mapNode **parent;

  parent = &(m->root);


  node = mapsearch(m, &parent, e);
  if (node == NULL) {
    node = mapnewnode(m);
   *parent = node;
  }
  dbgmsg("ADD: %p to %p\n", node,parent);
  memcpy(node->elem, e, m->nodes->esz - offsetof(mapNode, elem));
  mapbalance(m);
  return node->elem;
}

void *mapGet(map_t m, void *e)
{
  mapNode *p,**parent;
  parent = &(m->root);
  p = mapsearch(m, &parent, e);
  return (p == NULL? NULL : p->elem);
}

void mapdelnode(map_t m, mapNode *node)
{
  mapLeft(node) = VEC_DELETED;
  mapRight(node) = m->freelst;
  m->freelst = node;
  mapCnt(m)--;
}

void mapDel(map_t m, void *e)
{
  mapNode  *node;
  mapNode **parent;

  parent = &(m->root);

  node = mapsearch(m, &parent, e);

  dbgoff("mapDel() PTR: %p\n",node);

  if (node != NULL) {
    while (1) {
      if (mapRight(node) != NULL) {
       *parent =  maprotleft(node);
        parent = &mapLeft(*parent);
      }
      else if (mapLeft(node) != NULL) {
       *parent =  maprotright(node);
        parent = &mapRight(*parent);
      }
      else {
       *parent = NULL;
        mapdelnode(m,node);
        break;
      }
    }
    dbgoff("mapDel() Deleted: %p FROM: %p\n", node, parent);
  }
  return;
}


static void mapgoleft(map_t m, mapNode *p)
{
  while (p != NULL) {
    stkPush(m->stack,&p);
    p = mapLeft(p);
  }
}

void *mapNext(map_t m)
{
  mapNode *p;

  if (stkIsEmpty(m->stack)) return NULL;

  p = stkTopVal(m->stack, mapNode *);
  stkPop(m->stack);
  mapgoleft(m, mapRight(p));
  return p->elem;
}

void *mapFirst(map_t m)
{
  stkReset(m->stack);
  mapgoleft(m, m->root);
  return mapNext(m);
}

