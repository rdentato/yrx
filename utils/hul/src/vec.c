
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
#include "dbg.h"
#include "errors.h"

#define PTRSTEP 1024
#define PGSTEP 512
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
  dbgprintf(0,"  %u %u %u\n",ndx,*p, *n);
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
      for (i=0; i<PTRSTEP; i++)
        v->arr[i] = NULL;
    }
    else v->npg = 0;
  }
}

vec *vecNew(uint32_t elemsize)
{
  vec *v;

  v= malloc(sizeof(vec));
  vecInit(v,elemsize);
  if (v->arr == NULL) {
    free(v);
    v=NULL;
  }
  return v;
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
    if (v->arr == NULL) err(554,errstr(ERR_OUTOFMEM));
    while ( t < v->npg)  v->arr[t++] = NULL;
    dbgprintf(0,"\t *** %d \n",v->npg);
  }

  v->mrk.s = pgsize(page);
  if (v->arr[page] == NULL) {
    v->arr[page] = calloc(v->mrk.s,v->esz);
  }
  if (v->arr[page] == NULL) {
     dbgprintf(1,"\t *** %d \n",v->npg);
     err(555,errstr(ERR_OUTOFMEM));
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

  if (v == NULL)  err(553,errstr(ERR_OUTOFMEM));
  ndx2pg(ndx,&page,&n);
  return vecslot(v,page,n);
}

void *vecNext(vec *v)
{
  if (v == NULL)  err(553,errstr(ERR_OUTOFMEM));
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
    h->msk  = PGSTEP-1;
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


#ifdef UTEST
#include "ut.h"
typedef struct htest {
  KEY uint32_t key;
      uint32_t data1;
      uint32_t data2;
} htest;

uint32_t hashlittle( const void *key, size_t length, uint32_t initval);
uint32_t htestHfn(htest *a)
{ uint32_t h=0x811c9dc5;
  h = hashlittle(&(a->key),sizeof(uint32_t),h);
  return h;
}

static int htestCmp(htest *a, htest *b)
{return a->key - b->key;}

static void htestCpy(htest *a, htest *b)
{ *a=*b; }

static void htestClr(htest *a)
{ a->key = 0; a->data1 = 0; a->data2 = 0;}

ht *htestTNew()
{ return htNew(sizeof(htest),htestHfn, htestCmp, htestCpy,htestClr); }

uint32_t ui16Hfn(uint16_t *a)
{ return *a; }

static int ui16Cmp(uint16_t *a, uint16_t *b)
{return (*a - *b);}

static void ui16Cpy(uint16_t *a, uint16_t *b)
{ *a = *b; }

static void ui16Clr(uint16_t *a)
{ *a = 0;}

ht *ui16TNew()
{ return htNew(sizeof(uint16_t), ui16Hfn, ui16Cmp, ui16Cpy, ui16Clr); }

int main(int argc, char **argv)
{
  vec *v;
  ht *h;
  uint32_t t,p,n;
  void *q,*r;

  TSTHDR("VEC Tests");

  TSTGROUP("Integer logarithm base 2");

  TST("llog(0)   == 0",(llog2(0) == 0));
  TST("llog(1)   == 0",(llog2(1) == 0));
  TST("llog(2)   == 1",(llog2(2) == 1));
  TST("llog(16)  == 4",(llog2(16) == 4));
  TST("llog(127) == 6",(llog2(127) == 6));
  if (! TSTRES) { fprintf(stderr,"\tllog(%d) = %d\n",127,llog2(127)); }
  t=(1<<12)-1; TST("llog((2^12)-1) == 12-1",(llog2(t) == 11));
  if (! TSTRES) { fprintf(stderr,"\tllog(%d) = %u\n",t,llog2(t)); }
  TST("llog((2^32)-1) == 31",(llog2(0xFFFFFFFF) == 31));

  if (! TSTRES) { fprintf(stderr,"\tllog = %d\n",llog2(0xFFFFFFFF)); }

  TSTGROUP("Index to page and subindex");

  ndx2pg(0,&p,&n);          TST("0          -> (0,0)",(p==0 && n==0));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP-1,&p,&n);   TST("PGSTEP-1   -> (0,PGSTEP-1)",(p==0 && n==PGSTEP-1));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP,&p,&n);     TST("PGSTEP     -> (1,0)",(p==1 && n==0));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP+1,&p,&n);   TST("PGSTEP+1   -> (1,1)",(p==1 && n==1));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(2*PGSTEP-1,&p,&n); TST("2*PGSTEP-1 -> (1,PGSTEP-1)",(p==1 && n==PGSTEP-1));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(2*PGSTEP,&p,&n);   TST("2*PGSTEP   -> (1,PGSTEP)",(p==1 && n==PGSTEP));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(2*PGSTEP+1,&p,&n); TST("2*PGSTEP+1 -> (1,PGSTEP+1)",(p==1 && n==PGSTEP+1));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(3*PGSTEP-1,&p,&n); TST("3*PGSTEP-1 -> (1,2*PGSTEP-1)",(p==1 && n==2*PGSTEP-1));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(3*PGSTEP,&p,&n);   TST("3*PGSTEP   -> (2,0)",(p==2 && n==0));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(3*PGSTEP+1,&p,&n); TST("3*PGSTEP+1 -> (2,1)",(p==2 && n==1));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }


#if 0
  TSTGROUP("Integer sqrt");

  TST("lsqrt(16)  == 4",(lsqrt(16) == 4));
  TST("lsqrt(20)  == 4",(lsqrt(20) == 4));
  TST("lsqrt(126) == 4",(lsqrt(126) == 11));
  TST("lsqrt(1338754921) == 36589",(lsqrt(1338754921) == 36589));
  TST("lsqrt(2677509842UL) == 51744",(lsqrt(2677509842UL) == 51744));
  TST("lsqrt(4294967295UL) == 65535",(lsqrt(4294967295UL) == 65535));
  TST("lsqrt(4294836225UL) == 65535",(lsqrt(4294836225UL) == 65535));

  TSTGROUP("Index to page and subindex");

  ndx2pg(0,&p,&n);          TST("0   -> (0,0)",(p==0 && n==0));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP/2,&p,&n);   TST("S/2 -> (0,S/2)",(p==0 && n==(PGSTEP/2)));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP-1,&p,&n);   TST("S-1 -> (0,S-1)",(p==0 && n==(PGSTEP-1)));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP,&p,&n);     TST("S   -> (1,0)",(p==1 && n==0));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP+1,&p,&n);   TST("S+1 -> (1,1)",(p==1 && n==1));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP*2,&p,&n);   TST("S*2 -> (1,S)",(p==1 && n==(PGSTEP)));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(PGSTEP*2+1,&p,&n);   TST("S*2+1 -> (1,S+1)",(p==1 && n==(PGSTEP+1)));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(pgsize(3),&p,&n);   TST("pgsize(3)",(((p*(p+1)*PGSTEP)/2 + n) == pgsize(3)));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(pgsize(3)+5,&p,&n); TST("pgsize(3)+5",(((p*(p+1)*PGSTEP)/2 + n) == pgsize(3)+5));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(pgsize(4),&p,&n);   TST("pgsize(4)",(((p*(p+1)*PGSTEP)/2 + n) == pgsize(4)));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(pgsize(5)+6,&p,&n);   TST("pgsize(5)+6",(((p*(p+1)*PGSTEP)/2 + n) == pgsize(5))+6);
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(12899920,&p,&n);  TST("12899920",(((p*(p+1)*PGSTEP)/2 + n) == 12899920));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(912909125,&p,&n);  TST("912909125",(((p*(p+1)*PGSTEP)/2 + n) == 912909125));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(2412899920UL,&p,&n);  TST("2412899920",((uint32_t)(((p*(p+1)/2)*PGSTEP) + n) == 2412899920UL));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(3412899920UL,&p,&n);  TST("3412899920",(((p*(p+1)/2)*PGSTEP + n) == 3412899920UL));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

  ndx2pg(4294836225UL,&p,&n);  TST("4294836225",(((p*(p+1)/2)*PGSTEP + n) == 4294836225UL));
  if (! TSTRES) { fprintf(stderr,"\tp=%d n=%d\n",(unsigned int)p,(unsigned int)n); }

#endif
  TSTGROUP("Create vectors()");

  v = vecNew(sizeof(uint32_t));
  TST("New vector"  ,(v != NULL));
  TST("npg and esz set" ,(v && v->npg == PTRSTEP && (v->esz=sizeof(uint32_t))));


  TSTGROUP("Adding elements");

  for (t=0; t<20; t++) vecSet(v,100-t,&t);
  TST("v[90]=10"  ,(*((uint32_t *)vecGet(v,90)) == 10));

  TST("At least a page!",(v->npg == PTRSTEP));

  t=104; vecSet(v,65538,&t);
  TST("Element set properly",(*((uint32_t *)vecGet(v,65538)) == t));

  t=*((uint32_t *)vecGet(v,PGSTEP*2));
  TST("Unset elements are 0",(t == 0));
  if (! TSTRES) { fprintf(stderr,"\tval= %08X\n",t); }

  t=*((uint32_t *)vecGet(v,PGSTEP*3+7));
  TST("As well as non existant pages",(t == 0));


/*
  t=104; vecSet(v,65425538,&t);
  TST("Element set properly",(*((uint32_t *)vecGet(v,65425538)) == t));
*/
  TSTGROUP("vecNext()");


  n=100; vecSet(v,PGSTEP-1,&n);
  n/=10; vecSet(v,PGSTEP,  &n);
  n/=10; vecSet(v,PGSTEP+1,&n);

  q = vecGet(v,PGSTEP-1); n = *((uint32_t *)q);
  t = v->mrk.p;
  q = vecNext(v);  n += *((uint32_t *)q);
  q = vecNext(v);  n += *((uint32_t *)q);
  TST("vecNext cross page",(( n == 111) && ((t+1) == v->mrk.p)));
  if (! TSTRES) { fprintf(stderr,"\tn= %d t= %d mrk.p=%d\n",n,t,v->mrk.p); }

  r = vecGet(v,0);
  n=PGSTEP*6+4; p=0;
  for (t=1; t<n;t++) {
     r=vecNext(v);
     q=vecGet(v,t);
     if (q != r) { t=n+4; }
     dbgprintf(0," --> %d\n",t);
  }
  TST("lots of vecGet",(t==n));
  TSTGROUP("vecFree()");

  v=vecFree(v);
  TST("v is now NULL", (v == NULL));


{
  htest htst;
  htest *hp,*hq;

  TSTHDR("Hash tests");

  TSTGROUP("Create hash table()");

  h=htestTNew();
  TST("Really created?",(h!=NULL));
  TST("0 elements for now", (htCount(h) == 0));

  htst.key = 123; htst.data1=0xF00D;
  hp=htInsert(h,&htst);

  TSTGROUP("Insert elements");

  TST("Insert key 123", ((hp != NULL) &&  (hp->key == 123) && (hp != &htst) && (hp->data1 == 0xF00D)));
  if (!TSTRES) fprintf(stderr,"\t 0x%p\n",hp);

  htst.data1 = 0xBAFF0;
  hq=htInsert(h,&htst);
  TST("Reinsert same key", ((hq == hp) && (hq->data1 == 0xBAFF0)));

  hq->data1 = 0x1DEA;
  hp == htSearch(h,&htst);
  TST("Search existing key", ((hq == hp) && (hp->data1 == 0x1DEA)));

  /**/

  #define NELEMS 1501005
  for (t = 0; t < NELEMS; t++) {
     dbgprintf(0,"%d ",t);
     htst.key = 10400-t;
     htst.data2 = t;
     hq=htInsert(h,&htst);
     if (hq == NULL) fprintf(stderr,"RIARGH!\n");
  }

  TST("Inserted 380 keys", (htCount(h) == 381));
  if (!TSTRES) fprintf(stderr,"\t htCount = %d\n",htCount(h));

  TSTGROUP("Search elements");

  htst.key--;
  hp = htSearch(h,&htst);
  TST("Search non existing key", (NULL == hp));
  if (!TSTRES) fprintf(stderr,"\t 0x%p (%p %p)\n",hp,hq,&htst);

  htst.key = 123; hp = htSearch(h,&htst);
  TST("Search existing key (2)", ((NULL != hp) && (hp->key == htst.key)));
  if (!TSTRES) fprintf(stderr,"\t 0x%p (%p %p)\n",hp,hq,&htst);

  TSTGROUP("Delete elements");

  n = h->ecnt;
  hq = htSearch(h,&htst);
  hp = htDelete(h,&htst);
  TST("Deleted a key", ((h->ecnt == n-1) && (h->flst == hq) && (NULL == hp)));

  hp = htSearch(h,&htst);
  TST("Serching a deleted a key", (NULL == hp));
  if (!TSTRES) fprintf(stderr,"\t 0x%p (%p %p)\n",hp,hq,&htst);

  htst.key = 123; htst.data1 = 0xACAC1A;
  hp=htInsert(h,&htst);
  TST("Inserted key 0", ((NULL != hp) && (hp->data1==0xACAC1A) && (h->flst == NULL)));

  q = h->flst;
  n = h->ecnt;
  hp=htDelete(h,&htst);

  TST("Free list initialized", ((q==NULL) && (n == (h->ecnt+1)) && (h->flst == hq)));
  if (!TSTRES) fprintf(stderr,"\t %d %d (%p %p)\n",n,h->ecnt,h->flst,hq);

  htst.data1 = 0xF0CACC1A;
  hp=htInsert(h,&htst);
  TST("Reusing 1 free slot", ((n==h->ecnt) && (hq == hp) && (h->flst == NULL) &&(hp->data1 == 0xF0CACC1A)));
  if (!TSTRES) fprintf(stderr,"\t %d %d (%p %p) %p %X\n",n,h->ecnt,hp,hq,h->flst,hp->data1);

  for (t = 0; t < NELEMS ; t++) {
     htst.key = 10400-t;
     hq=htDelete(h,&htst);
  }
  TST("Deleting MANY keys", (NULL == hq));

  n=h->ecnt;
  htst.key = 1; htst.data1 = 0xC1CC18;
  hp=htInsert(h,&htst);
  htst.key = 2; htst.data1 = 0xF1BB1A;
  hp=htInsert(h,&htst);
  TST("Inserted key 1 and 2", (n==h->ecnt-2));
  htst.key = 2; hp=htDelete(h,&htst);
  htst.key = 1; hp=htDelete(h,&htst);

  TSTGROUP("Delete Hash table");

  h=htFree(h);
  TST("h is now NULL", (h == NULL));


  TSTGROUP("Hash table iterator");

  h=htestTNew();
  TST("hashtable recreated",(h != NULL));
  n = 100;
  for (t = 0; t < 100 ; t++) {
     htst.key = t;
     hq=htInsert(h,&htst);
     hq->data1 = n;
     n+=10;
  }
  TST("Added 100 elements",(h->ecnt == 100));

  hq = htFirst(h); n = 0;
  while (hq != NULL) {
    /*fprintf(stderr,"%d -> (%d,%d)\n",n++,hq->key, hq->data1);*/
    hq = htNext(h);
  }

  {
    htest **hql;

    hql = htIndex(h);
    TST("Got index for the table",(hql != NULL));
    for (t = 0; t < 100 ; t++) {
      hq=hql[t];
     /* fprintf(stderr,"%d -> (%d,%d)\n",t,hq->key, hq->data1);*/
    }
    free(hql);
  }

  h=htFree(h);
  TST("h is NULL again", (h == NULL));

  TSTGROUP("Hash table with no vec array");

  h=ui16TNew();

  for (t=10;t<40;t++)
    htInsert(h,&t);
  TST("Added 30 elements",(h->ecnt == 30));
  t=32;
  hq = htSearch(h,&t); t=0;
  TST("Got 32",(hq != NULL && *((uint16_t *)hq) == 32));

  h=htFree(h);
  TST("h is NULL again", (h == NULL));

}
  TSTSTAT();
  exit(0);
}

#endif

