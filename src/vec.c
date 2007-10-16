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

vec *bufNew()
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

uint32_t bufLen(vec *b)
{
  return b->cnt;
}

/*******/

#define bmpBlkSize 16
#define bmpBlkMask (bmpBlkSize-1)

vec *bmpNew() { return vecNew(bmpBlkSize); }

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

/*********/


lst_t lstNew(uint16_t elemsz)
{
  lstVec *l;

  l = malloc(sizeof(lstVec));
  if (l != NULL) {
    vecinit(l->vec,elemsz + offsetof(lstNode, data));
    l->tail = NULL;
    l->free = NULL;
    l->cnt = 0;
  }
  return l;
}

static lstNode *lstgetnode(lst_t l)
{
  lstNode *node = NULL;

  if (l->free != NULL) {
    node = l->free;
    l->free = node->data->nextfree;
  }
  else {
    node = vecGet(l->vec, vecCnt(l->vec));
    vecCnt(l->vec)++;
  }
  node->next = node;
  return node;
}

static lstNode *lstreleasenode(lst_t l,void *e)
{
  return NULL;
}

static lstNode *lstinsafter(lst_t l, lstNode *ref, void *e)
{
  lstNode *node;
  node = lstgetnode(l);
  if (node != NULL) {
    memcpy(node->data->elem, e, (l->vec->esz - offsetof(lstNode, data)));
    node->next = node;
    l->cnt++;
    if (ref != NULL) {
      node->next = ref->next;
      ref->next = node;
    }
  }
  return node;
}

void *lstInsAfter(lst_t l,void *ref,void *e)
{
  lstNode *node = NULL;

  if (ref != NULL) node = lstnode(ref);
  node = lstinsafter(l,node,e);
  return lstNodeElem(node);
}

void *lstInsHead(lst_t l, void *e)
{
  lstNode *node;
  node = lstinsafter(l,l->tail,e);
  if (l->tail == NULL) l->tail = node;
  return lstFirst(l);
}

void *lstFreeClean(lst_t l, vecCleaner cln)
{
  lstNode *node;
  uint32_t k = 0;

  if (l != NULL) {
    if (cln != NULL) {
      for (k = 0; k < vecCnt(l->vec); k++) {
        node = vecGet(l->vec,k);
        if (node->next != lstDELETED) cln(lstNodeElem(node));
      }
    }
    veccleanup(l->vec,NULL);
    free(l);
  }

  return NULL;
}

void *lstGet(lst_t l,uint32_t ndx)
{
  lstNode *node;
  node = lstHead(l);

  if (node == NULL) return NULL;
  while (ndx-- > 0) {
    if (node == l->tail) return NULL;
    node = node->next;
  }
  return node;
}

void *lstNext(lst_t l, void *ref)
{
  lstNode *node;

  node = lstnode(ref);
  if (node == NULL || node == l->tail)
    return NULL;

  return node->next->data->elem;
}

void *lstRemoveHead(lst_t l)
{
  lstNode *node;

}

/*******/

#define CNT_MSK         0x80000000
#define CNT_ISRIGHT(x)  (((x) & CNT_MSK) != 0)
#define CNT_ISLEFT(x)   (((x) & CNT_MSK) == 0)
#define CNT_SIZE(x)     ((x) & ~CNT_MSK)
#define CNT_FLIP(x,n)   ((((x) & CNT_MSK) ^ CNT_MSK) | (n - CNT_SIZE(x)))

#define mapLEFT(node)    (node->lnk[0])
#define mapRIGHT(node)   (node->lnk[1])

typedef struct mapNode {
  struct mapNode *lnk[2];
  uint32_t        cnt;
  int8_t          elem[sizeof(void *)];
} mapNode;

typedef struct mapVec {
  vec             nodes[1];
  vec             stack[1];
  struct mapNode *root;
  struct mapNode *free;
  uint32_t        cnt;
} mapVec;

#define map_t mapVec *


map_t mapNew(uint16_t elemsz, uint16_t keysz)
{
  map_t m;

  m = malloc(sizeof(lstVec));
  if (m != NULL) {
    vecinit(m->nodes,elemsz + offsetof(mapNode, elem));
    vecinit(m->stack,2);
    m->root = NULL;
    m->free = NULL;
    m->nodes->aux = keysz;
    m->cnt = 0;
  }

  return m;
}


void *mapFreeClean(map_t m,vecCleaner cln)
{
  if (m != NULL) {
    veccleanup(m->nodes,cln);
    veccleanup(m->stack,NULL);
    free(m);
  }
  return NULL;

}

#define mapCMP(m,n,e) memcmp(n->elem,e,m->nodes->aux)

static mapNode *ins_root(map_t m, mapNode **parent,void *e)
{
  mapNode *L,*R;
  mapNode *node;
  mapNode **S, **G;
  int cmp;

  node = *parent;

  S = &L; G = &R;

  while (node != NULL) {
    if ((cmp = mapCMP(m,node,e)) == 0) {
      /*...*/
       break;
    }
    if (cmp < 0) {   /* node < e */
     *S = node;
      S = &mapRIGHT(node);
      node = mapRIGHT(node);
    }
    else {
     *G = node;
      G = &mapLEFT(node);
      node = mapLEFT(node);
    }
  }
  *S=NULL; *G=NULL;

  node = newmapnode(m);

  memcpy(node->elem,e,m->nodes->esz);
  mapLEFT(node) = L;
  mapRIGHT(node) = R;
 *parent = node;

  return node;
}

void *mapAdd(map_t m, void *e)
{
  mapNode *node;
  mapNode **parent;
  uint32_t size;
  int cmp;

  parent = &(m->root);
  node = m->root;
  size = m->cnt;

  while (1) {
    if ((cmp = mapCMP(m,node,e)) == 0)
       return node->elem;

    if (rndnum(size) == size) {
      node = ins_root(m,parent,e);
      return node->elem;
    }
    if (cmp < 0)  {  /* node < e */
      if (CNT_ISRIGHT(node->cnt))
        node->cnt = CNT_FLIP(node->cnt,size);
      size -= CNT_SIZE(node->cnt);
      parent = &(mapRIGHT(node));
      node = mapRIGHT(node);

    }
    else {
      if (CNT_ISLEFT(node->cnt))
        node->cnt = CNT_FLIP(node->cnt,size);
      size -= CNT_SIZE(node->cnt);
      parent = &(mapLEFT(node));
      node = mapLEFT(node);
    }
  }

  return m;
}










