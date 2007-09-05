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

#define PTRSTEP 128  
#define PGSTEP 64

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
  k=llog2(p/2);
  if (p >= 3* (1<<k)) k++;
  return (1<<k) * PGSTEP;
}

static void ndx2pg(uint32_t ndx, uint32_t *p, uint32_t *n)
{
  uint32_t k,m,s,b;

 *p = 0;
  m=(ndx/PGSTEP) ;

  k  = llog2(m+1);
  *p = (k & 1)? 1<<(k/2): 0;

  m  = m - (1<<k) + 1 ;
  s  = k - (k/2);
  k  = k/2;
  b  = m >> s;

  *p += (1<<(k+1)) - 2 + b;
  *n = ((m - (b<<s))*PGSTEP)+ (ndx & (PGSTEP-1));
  dbgprintf(DBG_MSG,"  %u %u %u (%u)\n",ndx,*p, *n,pgsize(*p));
}


static void vecInit(vec *v,uint32_t elemsize)
{
  uint16_t i;
  if (v) {
    v->esz = elemsize;
    v->npg = PTRSTEP;
    v->mrk.p = 0;
    v->mrk.n = (uint32_t)-1;
    v->arr = malloc(PTRSTEP * sizeof(uint8_t *));
    v->mrk.q = *v->arr-v->esz;
    if (v->arr) {
      for (i=0; i<PTRSTEP; i++) v->arr[i] = NULL;
    }
    else v->npg = 0;
  }
}

vec *vecNew(uint32_t elemsize)
{
  vec *v;

  v = malloc(sizeof(vec));
  vecInit(v,elemsize);
  if (v->arr == NULL) {
    free(v);
    v=NULL;
  }
  return v;
}

vec *vecSetPSZ(vec *v,uint16_t pgsize)
{
  
}

static void vecCleanup (vec *v)
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
    vecCleanup(v);
    free(v);
  }
  return NULL;
}

static void *vecslot(vec *v,uint32_t page, uint32_t n)
{
  void *p=NULL;
  uint32_t t;

  if (page >= v->npg) {
    t=v->npg;
    v->npg = ((page / PTRSTEP)+1)*PTRSTEP;
    v->arr = realloc(v->arr, v->npg * sizeof(void *));
    if (v->arr == NULL) err(errNOMEM);
    while (t < v->npg)  v->arr[t++] = NULL;
    dbgprintf(0,"\t *** %d \n",v->npg);
  }

  v->mrk.s = pgsize(page);
  if (v->arr[page] == NULL) {
    v->arr[page] = calloc(v->mrk.s,v->esz);
  }
  if (v->arr[page] == NULL) {
     dbgprintf(1,"\t *** %d \n",v->npg);
     err(errNOMEM);
  }

  p = v->arr[page]+(n*v->esz);

  v->mrk.p=page;
  v->mrk.n=n;
  v->mrk.q=p;

  return p;
}

#define vecMark(v,m,o)  do {if (o=='r') (v)->mrk = m; else  m=(v)->mrk;} while(0)

void *vecGet(vec *v,uint32_t ndx)
{
  uint32_t page, n;

  if (v == NULL)  err(errNOMEM);
  ndx2pg(ndx,&page,&n);
  return vecslot(v,page,n);
}

void *vecNext(vec *v)
{
  if (v == NULL)  err(errNOMEM);
  if (++v->mrk.n >= v->mrk.s) {
    return vecslot(v,v->mrk.p+1,0);
  }

  v->mrk.q += v->esz;
  return v->mrk.q;
}

void *vecSet(vec *v, uint32_t ndx, void *elem)
{
  void *p=NULL;

  p = vecGet(v,ndx);

  if (p != NULL) {
    if (elem) memcpy(p, elem, v->esz);
    else      memset(p, 0x00, v->esz);
  }
  return p;
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
  }
  return size;
}

/***********/
/* TODO (remo#9#) document how hashtable are handled */

#define NOKEY ((uint32_t)0L)

static uint32_t hashrnd;

static uint32_t hash(ht *h,void *a)
{
  uint32_t k;
  k=h->hfn(a);
  if (k == NOKEY) k = 0x811c9dc5;
  hashrnd ^= k;
  return k;
}

/* The macro helem is not safe with respect to side effects on q*/
#define helem(h,q) (q == NULL? \
                              NULL: \
                              (h)->arr.arr == NULL? \
                                                   (void *)(&((q)->elem)): \
                                                   (q)->elem.p)

ht *htInit(ht *h, uint32_t elemsize,void *hfn, void *cmp, void *cpy, void *clr)
{
  if (h == NULL) h=malloc(sizeof(ht));
  if (h != NULL) {
    h->ecnt = 0;
    h->msk  = PGSTEP-1;  // Assumes PGSTEP is a power of two!!
    h->flst = NULL;
    h->hfn  = hfn;
    h->cmp  = cmp;
    h->cpy  = cpy;
    h->clr  = clr;
    h->arr.arr = NULL;
    h->arr.esz = elemsize;
    vecInit(&(h->ndx),sizeof(hx));
    /*if (elemsize < sizeof(he)) elemsize = sizeof(he);*/
    if (elemsize > sizeof(he))
      vecInit(&(h->arr),elemsize);
  }
  return h;
}


#define addto(l,i) ((i)->next=l,i)
static void reash(ht *h)
{
   uint32_t m,n;
   hx *q, *p, *s, *r, *t, *u;
   vMark vm;

   dbgprintf(1,"reash %d %d ...",h->ecnt, h->msk+1);
   m = (h->msk << 1) | 1;
   q = vecGet(&(h->ndx),0);
   for (n=0; n <= h->msk; n++) {
     if ((q->key != NOKEY) && (q->key & h->msk) == n) {
       p=NULL; s=NULL; r=q;
       while (1) {
         t=r->next;
         if ((r->key & m) > n) s=addto(s,r);
         else p=addto(p,r);
         r=t; if (r==q) break;
       }
       /* now p points to the list of 0k  and s to the list of 1k*/
       /* the list are "inverted!" */
       if (s != NULL) {
         /* fix 1k list */
         vecMark(&(h->ndx),vm,'s');
         r=vecGet(&(h->ndx), s->key & m);
         vecMark(&(h->ndx),vm,'r');
         t=s->next; s->next = r;
         while (t != NULL) {
           u = t; t = t->next;
           u->next = s; s = u;
         }
         *r=*s;
         s->key = NOKEY; s->elem.p = NULL; s->next = NULL;
       }
       if (p!=NULL) {
         /* fix 0k list */
         t = p->next; p->next = q;
         while (t != NULL) {
           u = t; t = t->next;
           u->next = p; p = u;
         }
         if (p != q)  {
           *q=*p;
            p->key = NOKEY; p->elem.p = NULL; p->next = NULL;
         }
       }
     }
     q=vecNext(&(h->ndx));
   }
   h->msk = m;
   /*fprintf(stderr,"done\n");fflush(stderr);*/
}


void *htSearch(ht *h,void *e)
{
  uint32_t k,n;
  hx *q, *s;

  k=hash(h,e);
  n = k & h->msk;
  q = vecGet(&(h->ndx),n);
  s = q;
  while (q) {
    if (q->key == k && h->cmp(e,helem(h,q)) == 0)
      break;
    q = q->next;
    if (q==s) q=NULL;
  }
  if (q != NULL && q->key == 0) q = NULL;

  return helem(h,q);
}

void *htDelete(ht *h,void *e)
{
  uint32_t k,n;
  hx *q, *s, *r;

  k=hash(h,e);
  n = k & h->msk;
  q = vecGet(&(h->ndx),n);
  s = q; r = s;
  while (q) {
    if (q->key == k && h->cmp(e,helem(h,q)) == 0)
      break;
    r = q;
    q = q->next;
    if (q==s) q=NULL;
  }
  if (q != NULL) {
    if (h->arr.arr) {
      *((void **)q->elem.p) = h->flst;
      h->flst = q->elem.p;
    }
    if (q == s) {
      q = q->next;
      *s = *q;
    }
    else {
      r->next = q->next;
    }
    q->key = 0;  q->elem.p = NULL; q->next = NULL;
    h->ecnt--;
  }
  return NULL;
}

/* This should be the "Brent variation" hash table as described in
** the Lua code. Unfortunately the paper where this is introduced is not
** freely available and I could'nt check.
*/
#define TOO_DENSE(h) ((h->msk - h->ecnt) < (h->msk/4))
void *htInsert(ht *h,void *e)
{
  void *p=NULL;
  uint32_t k,n,t;
  hx *q, *r, *s;

  if (TOO_DENSE(h)) reash(h);
/*fprintf(stderr,"*");fflush(stderr);*/
  k=hash(h,e);

  n = k & h->msk;
  q = vecGet(&(h->ndx),n);

  if (q->key != k || h->cmp(e, helem(h,q)) != 0) {
    if (q->key == NOKEY) {
      q->next = q;
    }
    else {
      t = (hashrnd & h->msk);
      r = vecGet(&(h->ndx),t);
      while (r->key != 0) {
        if (++t > h->msk) r=vecGet(&(h->ndx),(t=0));
        else r=vecNext(&(h->ndx));
      }
      t = q->key & h->msk;
      if (t == n) { /* position is right! */
        r->next = q->next;        /* add to the list */
        q->next = r;
        q=r;
      }
      else { /* move the intruder out of the way */
        s = q;
        while (s->next != q) s=s->next;
        s->next = r;
        *r=*q;
        q->next = q;
      }
    }
    q->key = k;
    if (h->arr.arr != NULL) {
      if ( h->flst != NULL) {
        q->elem.p = h->flst;
        h->flst = *((void **)h->flst);
      }
      else q->elem.p = vecGet(&(h->arr),h->ecnt);
    }
    h->ecnt++;
  }

  p=helem(h,q);
  if (p != NULL) {
    if (e != NULL) h->cpy(p, e);
    else h->clr(p);
  }
  return p;
}

ht *htFree(ht *h)
{
  if (h != NULL) {
    vecCleanup(&(h->ndx));
    vecCleanup(&(h->arr));
    h->ecnt = 0;
    free(h);
  }
  return NULL;
}

void *htNext(ht *h)
{
  uint32_t n;
  vec *x;
  hx *q = NULL;

  x=&(h->ndx);
  n = (x->mrk.p * (x->mrk.p+1) * PGSTEP) + x->mrk.n;
  while (++n <= h->msk ) {
    q=vecNext(x);
    /*fprintf(stderr,"%u,%X\n",n,q->key);*/
    if (q->key != NOKEY) break;
  }
  if (q != NULL && q->key != NOKEY) return q->elem.p;
  return NULL;
}

void *htFirst(ht *h)
{
  uint32_t n=0;
  hx *q = NULL;

  q = vecGet(&(h->ndx),0);
  while (n++ <= h->msk) {
    q=vecNext(&(h->ndx));
    /*fprintf(stderr,"%u,%X\n",n,q->key);*/
    if (q->key != NOKEY) break;
  }
  if (q != NULL && q->key != NOKEY) return q->elem.p;
  return NULL;
}

void *htIndex(ht *h)
{
  void **p=NULL;
  void **q;

  if (htCount(h)>0) {
    p=malloc(sizeof(void **)*(htCount(h)+1));
    if (p != NULL) {
      q=p;
      *q=htFirst(h);
      while (*q != NULL) {
        *++q=htNext(h);
      }
    }
  }
  return p;
}

/* Taken from Christopher Clark hash table implementation
**
*/
static uint32_t scrambleint(uint32_t i)
{
    /* - logic taken from java 1.4 hashtable source */
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); /* >>> */
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); /* >>> */
    return i;
}


uint32_t u16mapHfn(u16map *a)        {return (a->key); }
int u16mapCmp(u16map *a, u16map *b)  {return (a->key - b->key);}
void u16mapCpy(u16map *a, u16map *b) {*a = *b; }
void u16mapClr(u16map *a)            {a->key = 0; a->val = 0;}


uint32_t u32setHfn(u32set *a)        {return a->val; }
int u32setCmp(u32set *a, u32set *b)  {return a->val - b->val; }
void u32setCpy(u32set *a, u32set *b) {*a = *b; }
void u32setClr(u32set *a)            {a->val = 0; }


