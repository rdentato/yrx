#define htCount(h)        ((h)->ecnt)

void *htFirst(ht *h);
void *htNext(ht *h);
ht   *htFree(ht *h);
void *htInsert(ht *h,void *e);
void *htDelete(ht *h,void *e);
void *htSearch(ht *h,void *e);
void *htIndex(ht *h);

ht *htInit(ht *h, uint32_t elemsize,void *hfn, void *cmp, void *cpy, void *clr);

#define htNew(e,f,p,y,r) htInit(NULL,e,f,p,y,r)
#define htCreate(t) htInit(NULL,sizeof(t),t##Hfn,t##Cmp,t##Cpy,t##Clr)


#define KEY

typedef struct u16map {
  uint16_t key;
  uint16_t val;
} u16map;


uint32_t u16mapHfn(u16map *a);
int u16mapCmp(u16map *a, u16map *b);
void u16mapCpy(u16map *a, u16map *b);
void u16mapClr(u16map *a);


typedef struct u32set {
  uint32_t val;
} u32set;

uint32_t u32setHfn(u32set *a);
int u32setCmp(u32set *a, u32set *b);
void u32setCpy(u32set *a, u32set *b);
void u32setClr(u32set *a);

uint32_t SuperFastHash (const uint8_t *data, int len, uint32_t hash);

typedef union {
  void    *p;
  int32_t  i;
  uint32_t u;
} he;

typedef struct hx {
  uint32_t   key;
  struct hx *next;
  he         elem;
} hx;

typedef struct {
  void      *flst;
  uint32_t   ecnt;
  uint32_t   msk;
  uint32_t  (*hfn)(void *a);
  int       (*cmp)(void *a, void *b);
  void      (*cpy)(void *a, void *b);
  void      (*clr)(void *a);
  vec arr;
  vec ndx;
} ht;


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
    vecInit(&(h->ndx),sizeof(hx));
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

   dbgprintf(DBG_MSG,"reash %d %d ...",h->ecnt, h->msk+1);
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
    vcleanup(&(h->ndx));
    vcleanup(&(h->arr));
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



/*
http://www.azillionmonkeys.com/qed/hash.html
*/
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash (const uint8_t *data, int len, uint32_t hash)
{
uint32_t tmp;
int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= data[sizeof (uint16_t)] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}


