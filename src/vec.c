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

= Content
=========

*/

#include "vec.h"

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

#include <assert.h>

static const char *errNOMEM = "Out of Memory (vec)";
/*static const char *errUNEXP = "Unexpected error";*/


/****************/



/*
** Integer log base 2 of uint32 integer values.
**   llog2(0) == llog2(1) == 0
*/
static uint16_t llog2(uint32_t x)
{
  int l;

  l=0;

  #ifdef LOG2_ASM   /* Use inline assembly instructions! */
    #if defined(__POCC__) || defined(_MSC_VER)
      /* Pelles C & MS Visual C++*/
      __asm { mov eax, [x]
              cmp eax, 0
              je  z
              bsr ecx, eax
              mov  l, ecx
         z:
      }
    #elif defined(__GNUC__)
      /* GCC */
      __asm__( "bsrl %1,%0" :"=r" (l) :"r" (x));
    #else
      /* fallback to the C version */
      #undef LOG2_ASM
    #endif
  #endif

  #ifndef LOG2_ASM  /* Make a binary search.*/
  _dbgmsg("Standard llog\n");
  if (x & 0xFFFF0000) {l += 16; x >>= 16;}   /* 11111111111111110000000000000000 */
  if (x & 0xFF00)     {l += 8;  x >>= 8 ;}   /* 1111111100000000*/
  if (x & 0xF0)       {l += 4;  x >>= 4 ;}   /* 11110000*/
  if (x & 0xC)        {l += 2;  x >>= 2 ;}   /* 1100 */
  if (x & 2)          {l += 1;  }            /* 10 */

  #endif

  return (uint16_t)l;
}

#define two_raised(n) (1<<(n))
#define div2(n)       ((n)>>1)


/**************************/

static const uint8_t sz[] = {0, blkCHRsz, blkU16sz, blkU32sz, blsPTRsz };

uint8_t *blkNew(uint8_t ty)
{
  blk_t b;
  int newsz;

  newsz = offsetof(blk,elem) + sz[ty];
  b = calloc(1,newsz);
  if (b != NULL) {
    b->slt = 1;
    b->cnt = 0;
    return b->elem;
  }
  return NULL;
}

static blk_t blksetsize(blk_t bl, uint16_t ndx, uint8_t sz)
{
  uint16_t t;

  if (bl == NULL)   {t = 411; goto err;}
  if (ndx > 0xFFF0) {t = 413; goto err;}

  t = bl->slt;
  bl->slt = ndx + two_raised((llog2(ndx+1) >> 1) +1 );
                  /* approx 2*sqrt(ndx) */

  dbgmsg("blk realloc: %d (%d) [%d]\n",bl->slt,ndx,sz);
  bl = realloc(bl, offsetof(blk,elem) + (bl->slt * sz));
  if (bl == NULL) {t = 415; goto err;}

  _dbgmsg("blk realloc: %d (%d)\n",bl->slt,ndx);
  if (t < bl->slt) {
    memset(bl->elem + t * sz, 0, (bl->slt - t) * sz);
  }
  if (bl->cnt > ndx)  bl->cnt = ndx + 1;
  return bl;

err :
  vecErr(t,errNOMEM);
  return NULL;
}

uint8_t *blkSetSize(uint8_t *b, uint16_t ndx, uint8_t ty)
{
  blk_t bl;

  if (b != NULL) {
    bl = blkNodePtr(b);
    bl = blksetsize(bl, ndx, sz[ty]);
    return bl->elem;
  }
  return b;
}

static uint8_t *blkset(uint8_t *b, uint16_t ndx, void *val, uint8_t sz)
{
  blk_t bl = blkNodePtr(b);

  if (ndx >= bl->slt)
    bl = blksetsize(bl,ndx,sz);

  b = bl->elem + ndx * sz;
  memcpy(b,val,sz);
  if (ndx >= bl->cnt) bl->cnt = ndx+1;
  return bl->elem;
}

uint8_t *blkAppend(uint8_t *b, uint8_t *a, uint8_t sz)
{
  uint32_t k;
  blk_t bl = blkNodePtr(b);

  k = blkCnt(b) + blkCnt(a);

  if (k > bl->slt)
    bl = blksetsize(bl, (uint16_t) (k+1) ,sz);

  memcpy(b+(bl->cnt * sz), a, blkCnt(a) * sz);
  bl->cnt = (uint16_t)k;

  return bl->elem;
}

uint8_t *blkSetInt(uint8_t *b, uint16_t ndx, uint32_t val, uint8_t ty)
{
  _dbgmsg("Set int: x[%d]=%X (%d)\n",ndx,val,ty);
  return blkset(b,ndx,&val, sz[ty]);
}

uint8_t *blkSetPtr(uint8_t *b, uint16_t ndx, void *val)
{
  return blkset(b,ndx,&val, sz[blkPTR]);
}

static void *blkget(uint8_t *b, uint16_t ndx, uint8_t sz)
{
  blk_t bl;
  if (b != NULL) {
    bl = blkNodePtr(b);
    if (ndx > 0xFFF0)  vecErr(545,"ulv index out of range");
    if (ndx >= bl->slt) return NULL;
    b = bl->elem + ndx * sz;
  }
  return b;
}

uint32_t blkGetInt(uint8_t *b, uint16_t ndx, uint8_t ty)
{
  uint32_t k = 0;
  b = blkget(b, ndx, sz[ty]);
  if (b != NULL) {
    switch (ty) {
      case blkCHR : k = (uint32_t)(*((uint8_t *)b));
      case blkU16 : k = (uint32_t)(*((uint16_t *)b));
      case blkU32 : k = (uint32_t)(*((uint32_t *)b));
    }
  }
  _dbgmsg("Get int: x[%d]=%X (%d)\n",ndx,k,ty);
  return k;
}

void *blkGetPtr(uint8_t *b, uint16_t ndx)
{
  b = blkget(b, ndx, sz[blkPTR]);
  if (b == NULL) return NULL;
  return (*((void **)b));
}

uint8_t *blkFree(uint8_t *b)
{
  blk_t bl;
  if (b == NULL) return NULL;
  bl = blkNodePtr(b);
  free(bl);
  return NULL;
}

int blkU32cmp (const void *a, const void *b) { return *((uint32_t *)a) - *((uint32_t *)b) ; }
int blkU16cmp (const void *a, const void *b) { return *((uint16_t *)a) - *((uint16_t *)b) ; }
int blkCHRcmp (const void *a, const void *b) { return *((uint8_t *)a)  - *((uint8_t *)b) ; }

void blkUniq(uint8_t *b,uint8_t sz)
{
  uint32_t j;
  uint8_t *a, *p;

  if (b == NULL) return;

  j = blkCnt(b);
  if (j < 2) return;

  a = b+sz;
  p = b+sz;
  while (--j > 0) {
     if (memcmp(a, a-sz, sz) != 0) {
       if (p != a)
         memcpy(p,a,sz);
       p += sz;
     }
     a += sz;
  }
  blkCnt(b) = (uint16_t)((p-b)/sz);
}

/*****************/

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
  _dbgmsg("ndg2pg ->  %4u %4u %4u (%4u)\n",ndx,*p, *n, pgsize(*p));
}

/* Ensure an element can store a (void *). Will be used for the freelist */
#define MINSZ sizeof(void *)

static void vecinit(vec *v,uint16_t elemsize)
{
  if (v != NULL) {
    if (elemsize < MINSZ) elemsize = MINSZ;
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
        _dbgmsg("#  %d %p\n",i,v->pgs[i]);
        if (v->pgs[i] != NULL) {
          if (cln != NULL) {
            for (k = 0, p = v->pgs[i];  k < pgsize(i);  k++, p += v->esz)
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

static void *vecslot(vec *v, uint16_t page, uint16_t n)
{
  void *p=NULL;
  uint16_t t;

  if (page >= v->npg) {

    /* Extend the index to pages*/
    _dbgmsg("\t *X* %d %d %d\n",page,v->npg,llog2(page+1));

    t  = v->npg;

   /* The smallest power of two greater then |page| */
    #if 0
    v->npg = (page == 0)? 1 : 1 << (llog2(page) +1) ;
    #else
    v->npg = page + 1;
    if (v->npg & (v->npg-1)) { /* if it's not a power of 2, get next power of 2 */
      uint16_t xp = v->npg - 1;
      xp |= (xp >> 1);
      xp |= (xp >> 2);
      xp |= (xp >> 4);
      xp |= (xp >> 8);
      v->npg = xp + 1;
    }
    #endif
    _dbgmsg("\t === %d (%d)\n",v->npg,page);
    v->pgs = realloc(v->pgs, v->npg * sizeof(void *));
    if (v->pgs == NULL) err(3001,"Unable to allocate page index for (%u)\n",page);
    /*while (t < v->npg)  v->pgs[t++] = NULL;*/
    memset(v->pgs+t, 0, (v->npg - t)*sizeof(void *));/**/

    _dbgmsg("\t *** %d \n",v->npg);
  }

  if (page != v->cur_p)
    v->cur_s = pgsize(page);

  if (v->pgs[page] == NULL) {
    v->pgs[page] = calloc(v->cur_s, v->esz); /* page is guaranteed to be filled with 0's */
    if (v->pgs[page] == NULL)  vecErr(3002,errNOMEM);
    _dbgmsg(" ]]]  page %d 0x%p\n", page, v->pgs[page]);
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

  if (v == NULL)  vecErr(3003,errNOMEM);

  if (ndx == VEC_NULLNDX) ndx = 0;

  if (v->cur_w != VEC_NULLNDX) {
    switch ((int)(ndx - v->cur_w)) {
      case  0 : _dbgmsg("get same %u (%u)\n",ndx,v->cur_w);
                return v->cur_q;

      case  1 : _dbgmsg("get next %u (%u)\n",ndx,v->cur_w);
                v->cur_w++;
                v->cur_n++;
                if (v->cur_n >= v->cur_s) return vecslot(v,v->cur_p+1,0);
                v->cur_q += v->esz;
                return v->cur_q;
      #if 1
      case -1 : _dbgmsg("get prev %u (w:%u p:%u)\n",ndx,v->cur_w,v->cur_p);
                if (v->cur_w == 0) return v->cur_q;
                v->cur_w--;
                if (v->cur_n == 0) {
                  page = v->cur_p - 1;
                  n = pgsize(page)-1;
                  return vecslot(v, page, n);
                }
                v->cur_n--;
                v->cur_q -= v->esz;
                return v->cur_q;
      #endif
    }
  }
  _dbgmsg("get index %d (%d)\n",ndx,v->cur_w);
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

/**************************/

/* Taken from Christopher Clark hash table implementation
** <http://www.cl.cam.ac.uk/~cwc22/hashtable>
*/
static uint32_t scrambleint(uint32_t i)
{
    /* - logic taken from java 1.4 hashtable source */
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); /* >>> */
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); /* >>> */
    return i ;
}

static union { mapNode *ptr; uint32_t n; } x__;
#define mappriority(x) (x__.n=0,x__.ptr = x,scrambleint(x__.n))

map_t mapNew(uint16_t elemsz, mapCmp_t cmp)
{
  map_t m;

  m = malloc(sizeof(mapVec));
  if (m != NULL) {
    vecinit(m->nodes,elemsz + offsetof(mapNode, elem));
    vecinit(m->stack,sizeof(mapNode *));
    m->root = NULL;
    m->freelst = NULL;
    m->cnt = 0;
    m->cmp = cmp;
  }
  return m;
}

map_t mapFreeClean(map_t m, vecCleaner cln)
{
  uint32_t k;
  mapNode *p;

  if (m != NULL) {
    if (cln != NULL) {
      for (k = 0; k < m->nodes->cnt; k++) {
        p = vecGet(m->nodes,k);
        if (p != NULL && mapLnkLeft(p) != VEC_DELETED)
          cln(p->elem);
      }
    }
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
    m->freelst = mapLnkRight(p);
  }
  else {
    p = vecSet(m->nodes, vecCnt(m->nodes),NULL);
  }
  mapLnkLeft(p) = NULL;
  mapLnkRight(p) = NULL;
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

  _dbgmsg("mapsearch() ROOT: %p\n",*parent);

  stkReset(m->stack);

  for(EVER) {
    stkPush(m->stack, &parent);  _dbgmsg("Pushed: %p (%p)\n",parent, *parent);

    node = *parent;
    if (node == NULL) break;   _dbgmsg("Not found\n");

    if (m->cmp != NULL)
      cmp = m->cmp(elem,node->elem);
    else
      cmp = memcmp(elem, node->elem, m->nodes->esz - offsetof(mapNode, elem));

    if (cmp == 0) break;   _dbgmsg("Found: %p\n",node);

    parent = (cmp > 0)? &mapLnkRight(node) : &mapLnkLeft(node);

  }

  *par = parent;

  return node;
}

uint32_t mapMaxDepth(map_t m)
{
  return (5*llog2(mapCnt(m)+1))/2;
}

static void mapbalance(map_t m)
{
  mapNode **p,**q;
  mapNode  *node;
  uint8_t direction;
  uint32_t nodepri;

  _dbgmsg("Count: %d Depth: %d Limit: %d\n",m->cnt, m->stack->cnt,1+llog2(m->cnt));
  p = stkTopVal(m->stack,mapNode **);
  node = *p;
  nodepri = mappriority(node);
  for(EVER) {
    stkPop(m->stack);
    q = stkTopVal(m->stack,mapNode **);
    if (q == NULL) break;

    direction = 0;
    if (p == &(mapLnkRight(*q))) direction++;
    _dbgmsg("  %p -> %p %c  (%x < %x)?\n",*q,*p,direction == 0? 'L':'R',nodepri,mappriority(*q));

    if (nodepri > mappriority(*q)) break;

    *q = maprotate(*q,direction ^ 1);
    _dbgmsg("Rotated: %p -> %p\n",*q,node);
    assert(*q == node);
    p = q;
  }
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

  _dbgmsg("ADD: %p to %p\n", node,parent);

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

void mapdelnode(map_t m, mapNode **parent)
{
  mapNode *node;
  uint32_t  priL = 0;
  uint32_t  priR = 0;

  node = *parent;

  _dbgmsg("mapDel() PTR: %p\n",node);

  if (node != NULL) {
    while ((mapLnkRight(node) != NULL) || (mapLnkLeft(node) != NULL)) {
      if (mapLnkLeft(node) != NULL) {
        if (mapLnkRight(node) != NULL) {
          priL = mappriority(mapLnkLeft(node));
          priR = mappriority(mapLnkRight(node));
        }
        else {
          priL = 1 ; priR = 0;
        }
      }
      else {
        priL = 0 ; priR = 1;
      }

      if (priL > priR) {
       *parent =  maprotright(node);
        parent = &mapLnkRight(*parent);
      }
      else {
       *parent =  maprotleft(node);
        parent = &mapLnkLeft(*parent);
      }
    }
   *parent = NULL;
    mapLnkLeft(node) = VEC_DELETED;
    mapLnkRight(node) = m->freelst;
    m->freelst = node;
    mapCnt(m)--;
  }
}

void mapDel(map_t m, void *e)
{
  mapNode  *node;
  mapNode **parent;

  parent = &(m->root);

  node = mapsearch(m, &parent, e);
  mapdelnode(m,parent);
   _dbgmsg("mapDel() Deleted: %p FROM: %p\n", node, parent);
  return;
}

static void mapgoleft(map_t m, mapNode *p)
{
  while (p != NULL) {
    stkPush(m->stack, &p);
    p = mapLnkLeft(p);
  }
}

void *mapNext(map_t m)
{
  mapNode *p;

  if (stkIsEmpty(m->stack)) return NULL;

  p = stkTopVal(m->stack, mapNode *);
  stkPop(m->stack);
  mapgoleft(m, mapLnkRight(p));
  return p->elem;
}

void *mapFirst(map_t m)
{
  stkReset(m->stack);
  mapgoleft(m, m->root);
  return mapNext(m);
}


      
/**************************/
/*
= References
============
On Resizable arrays:

@inproceedings{ brodnik99resizable,
  author    = "Andrej Brodnik and Svante Carlsson and Erik D. Demaine
               and J. Ian Munro and Robert Sedgewick",
  title     = "Resizable Arrays in Optimal Time and Space",
  booktitle = "Workshop on Algorithms and Data Structures",
  pages     = "37-48",
  year      = "1999",
  url       = "citeseer.ist.psu.edu/brodnik99resizable.html"
}

@article{Demaine:2001:AAF,
  author  =  "Erik Demaine",
  title   =  "Algorithm Alley: Fast and Small Resizable Arrays",
  journal =  "Doctor Dobb's Journal",
  volume  =  "26",
  number  =  "7",
  pages   =  "132--134",
  month   =  "jul",
  year    =  "2001",
  CODEN   =  "DDJOEB",
  ISSN    =  "1044-789X",
  bibdate =  "Thu Jun 7 06:07:17 MDT 2001",
  url     =  "http://www.ddj.com/architect/184404698",
}

For a discussion on MSB calculation, see section 8 of:

@misc{ bagwell-fast,
  author = "Phil Bagwell",
  title  = "Fast Functional Lists, Hash-Lists, Deques and
            Variable Length Arrays",
  url    = "citeseer.ist.psu.edu/bagwell02fast.html" }

On Randomized binary search tree:

@misc{ martinez97randomized,
  author = "C. Martinez and S. Roura",
  title  = "Randomized binary search trees",
  text   = "Conrado Martinez and Salvador Roura, Randomized binary
            search trees, in Research report of Universitat Politcnica
            de Catalunya, LSI97 -8-R, 1997.",
  year   = "1997",
  url    = "citeseer.ist.psu.edu/martinez97randomized.html"
}

@article{ seidel96randomized,
  author  = "Raimund Seidel and Cecilia R. Aragon",
  title   = "Randomized Search Trees",
  journal = "Algorithmica",
  volume  = "16",
  number  = "4/5",
  pages   = "464-497",
  year    = "1996",
  url     = "citeseer.ist.psu.edu/seidel96randomized.html"
}

On Random Number generator:

@misc{ l'ecuyer99table,
  author = "P. L'Ecuyer",
  title  = "A table of linear congruential generators of different 
            sizes and good lattice structure",
  text   = "P. L'Ecuyer. A table of linear congruential generators of
            different sizes and good lattice structure. Mathematics of
            Computation, 68(225), 1999. To appear.",
  year   = "1999",
  url    = "citeseer.ist.psu.edu/132363.html"
}

@misc{ entacher97collection,
  author = "K. Entacher",
  title  = "A collection of selected pseudorandom number generators
            with linear structures",
  text   = "K. Entacher. A collection of selected pseudorandom number
            generators with linear structures. Technical Report 97-1,
            ACPC -- Austrian Center for Parallel Computation,
            University of Vienna, Austria, 1997.
            Available at: http://random.mat.sbg.ac.at/.",
  year   = "1997",
  url    = "citeseer.ist.psu.edu/entacher97collection.html"
}

*/
