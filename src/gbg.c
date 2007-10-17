
typedef struct blkNode {
  struct blkNode *lnk[2];
  uint8_t         elm[sizeof(uint32_t)];
} blkNode;

#define blkElm(p)      (p == NULL? NULL : (void *)(((blkNode *)(p))->elm))
#define blkNodePtr(p)  (p == NULL? NULL : (void *)((char*)(p) - offsetof(blkNode,elm)))
#define blkDeleted     ((void *)vecGet)
#define blkFreelst(v)  ((v)->aux0.p)
#define blkLeft(n)     (((blkNode *)(n))->lnk[0])
#define blkRight(n)    (((blkNode *)(n))->lnk[1])
#define blkLeftPtr(n)  (n == NULL? NULL :((blkNode *)(n))->lnk[0])
#define blkRightPtr(n) (n == NULL? NULL :((blkNode *)(n))->lnk[1])

#define blkReset(v)   (vecCnt(v) == 0 , v->aux0.p = NULL )
#define blkFree       vecFree
#define blkSet(v,p,e) ((e != NULL)? memcpy(p->elm,e,v->esz - offsetof(blkNode,elm)) : 0)

void    blkDel  (vec *v, void *e);
vec    *blkNew  (uint16_t elemsz);
void   *blkAdd  (vec *v,void *e);
void   *blkFirst(vec *v);
void   *blkNext (vec *v);
void   *blkPrev (vec *v);

/**********************/

#define stkNew        vecNew
#define stkFree(v)    vecFree((vec *)v)
#define stkDepth(v)   vecCnt((vec *)v)
#define stkPush(v,e)  vecSet((vec *)v,stkDepth(v),e)
#define stkIsEmpty(v) (((vec *)v) == NULL || stkDepth(v) == 0)
#define stkPop(v)     (stkIsEmpty(v)? 0    : (stkDepth(v)-= 1))
#define stkTop(v)     (stkIsEmpty(v)? NULL : vecGet((vec *)v, stkDepth(v)-1))

#define stkNth(v,n)   (stkIsEmpty(v) || n >= stkDepth(v)\
                          ? NULL\
                          : vecGet((vec *)v, n))

#define stkReset(v)   (stkIsEmpty(v)? 0    : (stkDepth(v) = 0,((vec *)v)->cur_w=VEC_ANYNDX))

/**********************/


#define lstNode    blkNode
#define lstHead(v) ((v)->aux1.p)
#define lstTail(v) blkLeftPtr(lstHead(v))
#define lstCur(v)  ((v)->aux2.p)
#define lstCnt(v)  ((v)->aux3.n)
#define lstFree    vecFree

vec *lstNew(uint16_t elemsz);
void *lstAppend(vec *v,void *elem);
void *lstInsert(vec *v,void *elem);
void *lstFirst(vec *v);
void *lstLast(vec *v);
void *lstNext(vec *v);
void *lstPrev(vec *v);

/**********************/
vec    *mapNew     (uint16_t elemsz, uint16_t keysz);
void   *mapGet     (vec *v, void *elem);
void   *mapFree    (vec *v);
void   *mapFirst   (vec *v);
void   *mapNext    (vec *v);
void   *mapAdd     (vec *v, void *elem);
void    mapDel     (vec *v, void *elem);
#define mapRoot(v) ((v)->aux2.p)
#define mapCnt(v)  ((v)->aux3.n)

void *mapNextSorted(vec *v);
void *mapFirstSorted(vec *v);

#endif  /* VEC_H */



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

uint16_t rndnum()
{
  static uint32_t x = 0;

  if (x == 0) x = (uint32_t) time(NULL);

  x = ((12957 * x) % 16381);

  return ( x >> 2) & 0x7FFF;
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
  dbgprintf(DBG_MSG,"mapsearch() ROOT: %p\n",node);

  stkReset(STACK(v));

  while (node != NULL) {
    stkPush(STACK(v),&parent);
    dbgprintf(DBG_MSG,"Pushed: %p\n",parent);
    cmp = memcmp(node->elm,elem,v->ksz);
    if (cmp == 0) return node;
    parent = (cmp < 0) ? &blkRight(node)
                       : &blkLeft(node);
    node = *parent;
  }
  stkPush(STACK(v),&parent);
  dbgprintf(DBG_MSG,"Pushed: %p\n",parent);
  return NULL;
}

void *mapAdd(vec *v, void *elem)
{
  blkNode *p,**q;
  uint16_t r;
  uint16_t n;

  parent = (blkNode **)&mapRoot(v);

  p = mapsearch(v,mapRoot(v),elem);
  dbgprintf(DBG_OFF,"mapAdd() PTR: %p\n",p);
  if (p == NULL) {
    p = blkAdd(v,NULL);
   *parent = p;
    mapCnt(v)++;
    dbgprintf(DBG_MSG,"mapAdd() ADDED: %p TO: %p\n",p,parent);

    /* ROTATE */

    #ifdef DEBUG
    { int kk;
      void ***qq;

      kk = stkDepth(STACK(v));
      while (kk > 0) {
        kk--;
        qq = stkNth(STACK(v),kk);
        dbgprintf(DBG_MSG," %2d q = %p *q = %p **q = %p (%p)\n",kk,qq,*qq,(**qq == NULL ? NULL:**qq),NULL);
      }
    }
    #endif
    
    r = (1 + (rndnum() % mapCnt(v))) << 1 ;
    dbgprintf(DBG_MSG,"# RAND:%d\n",r);
    while (1) {
      n = stkDepth(STACK(v));
      if (n == 1) break;
      n = (n * (n+1)) << 1;
      if (r > n) break;
      q = *(void **)stkTop(STACK(v));
      stkPop(STACK(v));
      parent = *(void **)stkTop(STACK(v));
      dbgprintf(DBG_MSG,"  Rotate (parent = %p) (q = %p)",*parent,q);
      if (*parent == q)  {
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
  dbgprintf(DBG_MSG,"mapDel() PTR: %p\n",p);
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
    dbgprintf(DBG_MSG,"mapDel() Deleted: %p FROM: %p\n",p,parent);
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



/**** BLK ****/

#define BLKFLIST(v)  ((v)->aux0.n)

vec *blkNew(uint16_t elemsz)
{
  vec *v;

  v = vecNew(elemsz);
  if (v != NULL) BLKFLIST(v) = VEC_NULL;

  return v;
}

void blkDel(vec *v, void *p, uint32_t n)
{
  if (p == NULL)
  *(uint32_t *)p = BLKFLIST(v);
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


/**** DOUBLE LINKED LIST ****/

#define lstnxtblk blkRightPtr
#define lstprvblk blkLeftPtr

vec *lstNew(uint16_t elemsz)
{
  vec *v = blkNew(elemsz);

  if (v != NULL) {
    lstHead(v) = NULL;
    lstCnt(v)  = 0;
  }

  return v;
}

static void lstfrontadd(blkNode *h,blkNode *p)
{
  if (h == NULL) {
    blkRight(p) = p;
    blkLeft(p) = p;
  }
  else {
    blkLeft(p) = blkLeft(h);
    blkRight(p) = h;
    blkRight(blkLeft(p)) = p;
    blkLeft(h) = p;
  }
}

static blkNode *lstappend(vec *v,void *elem)
{
  blkNode *p=NULL;

  p = blkAdd(v,elem);
  lstfrontadd(lstHead(v),p);
  if (lstHead(v) == NULL) lstHead(v) = p;
  lstCnt(v)++;
  lstCur(v) = p;
  return p;
}

void *lstAppend(vec *v,void *elem)
{
   return lstappend(v,elem)->elm;
}

void *lstInsert(vec *v,void *elem)
{
  lstHead(v) = lstappend(v,elem);
  return ((blkNode *)lstHead(v))->elm;
}

static void lstremove(vec *v, blkNode *p)
{
  if (p != NULL) {
    if (lstnxtblk(p) == p) {
      lstHead(v) = NULL;
    }
    else {
      blkRight(blkLeft(p)) = blkRight(p);
      blkLeft(blkRight(p)) = blkLeft(p);
    }
    if (p == lstHead(v)) lstHead(v) = blkRight(p);
    blkDel(v,p);
    lstCur(v) = lstHead(v) ;
  }
}

void lstHeadDel(vec *v)
{
  lstremove(v,lstHead(v));
}

void lstTailDel(vec *v)
{
  lstremove(v,lstTail(v));
}

void *lstAddBefore(vec *v,blkNode *h,void *elem)
{
  blkNode *p = NULL;
  if (h != NULL) {
    p = blkAdd(v,elem);
    lstfrontadd(h,p);
  }
  return p;
}

void *lstFirst(vec *v)
{
  lstCur(v) = lstHead(v);
  if (lstCur(v) == NULL) return NULL;
  return ((blkNode *)lstCur(v))->elm;
}

void *lstLast(vec *v)
{
  lstCur(v) = lstprvblk(lstHead(v));
  if (lstCur(v) == NULL) return NULL;
  return ((blkNode *)lstCur(v))->elm;
}

void *lstNext(vec *v)
{
  lstCur(v) =  lstnxtblk(lstCur(v));
  if (lstCur(v) == lstHead(v)) lstCur(v) = NULL;
  if (lstCur(v) == NULL) return NULL;
  return ((blkNode *)lstCur(v))->elm;
}

void *lstPrev(vec *v)
{
  if (lstCur(v) == lstHead(v)) lstCur(v) = NULL;
  else lstCur(v) =  lstprvblk(lstCur(v));
  if (lstCur(v) == NULL) return NULL;
  return ((blkNode *)lstCur(v))->elm;
}


/**** MAP ****/

#define STACK(v) ((v)->aux1.p)




vec *mapNew(uint16_t elemsz, uint16_t keysz)
{
  vec *v = blkNew(elemsz);

  if (v != NULL) {
    v->ksz = keysz;
    STACK(v) = stkNew(sizeof(blkNode *));
    if (STACK(v) != NULL) {
      mapRoot(v)  = NULL;
      mapCnt(v) = 0;
    }
    else {
      free(v);
      v = NULL;
    }
  }

  return v;
}

void *mapFree(vec *v)
{
  if (STACK(v) != NULL) stkFree(STACK(v));
  return vecFree(v);
}





STATIC setNode *rotRight(setNode *pivot)
{
  setNode *q;

  if (pivot == NULL) return NULL;
  q = LEFTptr(pivot);
  if (q == NULL) return pivot;

  LEFTptr(pivot)  = RIGHTptr(q);
  pivot->size    -= setTreeSize(q->left) + 1 ;

  q->right = pivot;
  q->size += setTreeSize(pivot->right) + 1;

  return q;
}

STATIC setNode *rotLeft(setNode *pivot)
{
  setNode *q;

  if (pivot == NULL) return NULL;
  q = pivot->right;
  if (q == NULL) return pivot;

  pivot->right = q->left;
  pivot->size -= setTreeSize(q->right) + 1;

  q->left  = pivot;
  q->size += setTreeSize(pivot->left) + 1;

  return q;
}

static setNode **parent;

static setNode *setsearch(set *t, void *elem)
{
  int cmp;
  setNode *node;

  stkReset(t->stck);
  parent = &(t->root);
  node   = *parent;
  while (node != NULL) {
    stkPush(t->stck,&parent);
    cmp = t->cmp(elem,node->elem);
    if (cmp == 0) return node;
    parent = (cmp > 0) ? &(node->right)
                       : &(node->left);
    node = *parent;
  }
  stkPush(t->stck,&parent);
  return NULL;
}

void *setAdd(set *t, void *elem)
{
  setNode *p,***q;
  uint16_t r;
  uint16_t n;

  p = setsearch(t,elem);

  if (p == NULL) {  /* Node is to be inserted! */
    /* {{ Get a new node for the tree */
    if (t->freelst != NULL) {
      p = t->freelst;
      t->freelst = t->freelst->left;
    }
    else {
      p = vecGet(t->tree, t->tree->cnt);
      t->tree->cnt++;
    }
    p->left = NULL;
    p->right = NULL;
    p->size = 0;
    /* }} */

   *parent = p;  /* attach the new node */

    /* {{ Adjust subtree sizes and rotate to preserve random properties */
    r = 1;
    while (!stkIsEmpty(t->stck)) {
      q = stkTop(t->stck); stkPop(t->stck);
      parent = *q;
      (*parent)->size ++;
      if (r != 0) {
        r = rndmum % (*parent)->size;
      }
      else
    }
  }

  /* Set or modify the element data */
  if (elem != NULL)
    memcpy(p->elem,elem, t->tree->esz - offsetof(setNode,elem));

  return p->elem;
}

#define L  0
#define R  1
#define X  2

void setDel(set *t, void *elem)
{
  setNode *p,***q;;
  int r = 0;

  p = setsearch(t,elem);

  dbgprintf(DBG_OFF,"setDel() PTR: %p\n",p);

  if (p != NULL) {

    while (!stkIsEmpty(t->stck)) {
      q = stkTop(t->stck); stkPop(t->stck);
      (**q)->size --;
    }

    while (r != X) {
      /* TODO: probability should depend on sub tree size
       *       (OR delete from the bigger subtree?)
       */
      if (p->right != NULL && p->left == NULL) r = L;
      else if (p->right == NULL && p->left != NULL) r = R;
      else if (p->right == NULL && p->left == NULL) r = X;
      else r = rndnum() & 1;

      switch (r) {
        case L :    *parent = rotLeft(p);
                   (*parent)->size --;
                     parent = &((*parent)->left);
                     break;

        case R :    *parent = rotRight(p);
                   (*parent)->size --;
                     parent = &((*parent)->right);
                     break;

        case X :    *parent = NULL;
                    /* {{ Add p to free list */
                     p->left = t->freelst;
                     t->freelst = p;
                    /* }} */
                     break;
      }
    }
    dbgprintf(DBG_OFF,"setDel() Deleted: %p FROM: %p\n",p,parent);
  }
  return;
}




/**********************/

/* Using a LCG to ensure independency from how |rand()| has been implemented */
static uint32_t LCG(uint32_t x0)
{
  static uint32_t x = 0;
  x = (2147001325 * (x+x0)) + 715136305;  /*  1.6 BCPL */
  return  x;
}
#define rndnum() LCG(0)

/**********************/

#define LEFT  0
#define RIGHT 1
#define LEFTptr(n)  (n->lnk[LEFT])
#define RIGHTptr(n) (n->lnk[RIGHT])


static void flip(setNode *p, int sz)
{
  if (p->size == 0) return;
  if (p->size < 0)
    p->size = p->size + (sz -1);
  else
    p->size = p->size - (sz -1);
}

static void *treeins(set *t, void *e)
{
  int n,c;
  setNode *node, **parent;

  setNode  rootnode[1];

  if (t->root == NULL) {
    node = newnode(t,e);
    t->tree->cnt++;
  }
  else {

    rootnode->size = t->tree->cnt;
    rootnode->right = t->root;


    node = rootnode;

    n = t->tree->cnt;

    while (*parent) {
      c = t->cmp((*parent)->elem,e);
      if (c == 0) break;
      n = (*parent)->size;
      if ((rndnum() % (n+1)) == 0) {
        *parent = rootins(*parent,e);
        break;
      }
      if (c > 0) parent = &(t->right);
      else parent = &(t->left);
    }
  }

  return node;
}


/**********************/


#include <time.h>
set *setNew(int16_t elemsize, int (*cmp)())
{
  set *t;

  t = malloc(sizeof(set));
  if (t != NULL) {
    t->root = NULL;
    t->freelst = NULL;
    t->cmp = cmp;
    vecInit(t->tree,elemsize + offsetof(setNode,elem));
    vecInit(t->stck,sizeof(pn));
    LCG((uint32_t)time(NULL)); /* randomize */
  }
  return t;
}


void *setFree(set *t)
{
  if (t != NULL) {
    vcleanup(t->tree);
    vcleanup(t->stck);
    free(t);
  }
  return NULL;
}





/**********************/

typedef vec *arr;

typedef struct arrVal {
  uint32_t  tt;
  union {
    char          *str;
    uint32_t       nmu;
    int32_t        nmi;
    void          *ptr;
    char           chr;
    arr            arp;
  } vv;
} arrVal;

#define UNS   1
#define STR   2
#define INT   3
#define PTR   4
#define CHR   5
#define ARR   5
#define DEL   0

#define arrNew()      ((arr)vecNew(sizeof(arrVal)))

#define arrSet(a,ndx,T,v) arrSet_##T(a,ndx,v)

#define arrGet(a,ndx,T) arrGet_##T(a,ndx)
#define arrGet_V(a,n)  ((arrVal *)vecGet((vec *)a,n))

arrVal *arrSet_INT(arr a,uint32_t ndx, int n);
arrVal *arrSet_UNS(arr a,uint32_t ndx, uint32_t n);
arrVal *arrSet_STR(arr a,uint32_t ndx, char *s);
int32_t arrGet_INT(arr a,uint32_t ndx);
int32_t arrGet_UNS(arr a,uint32_t ndx);
char   *arrGet_STR(arr a,uint32_t ndx);

arr     arrFree(arr a);

/**********************/

typedef struct setNode {
  struct setNode *lnk[2];
  int             size;  /* */
  uint8_t         elem[sizeof(uint32_t)];
} setNode;

typedef struct set {
  setNode   *root;
  setNode   *freelst;
  int      (*cmp)(void *, void *);
  vec        tree[1];
  vec        stck[1];
} set;

set *setNew(int16_t elemsize, int (*cmp)());
void *setAdd(set *t, void *e);
void *setFree(set *t);
void setDel(set *t, void *elem);

#define setCnt(t) (t == NULL? 0 : setTreeSize(t->root))
#define setNodePtr(e) ((setNode *)(e - offsetof(setNode,elem)))

#ifdef DEBUG
#define setTreeSize(tn) (tn == NULL? 0 : tn->size)
setNode *rotLeft(setNode *pivot);
setNode *rotRight(setNode *pivot);
#endif


/*******/

arr arrFree(arr a)
{
  uint16_t i,n;
  arrVal *p;

  if (a != NULL) {
    if (a->pgs != NULL) {
      for (i = 0; i < a->npg; i++) {
        if (a->pgs[i] != NULL) {
           n = pgsize(i);
           p = (arrVal *)(a->pgs[i]);
           while (n > 0) {
             switch (p->tt) {
               case STR: free(p->vv.str); break;
               case ARR: vecFree(p->vv.arp); break;
             }
             p++; n--;
           }
           free(a->pgs[i]);
        }
        a->pgs[i] = NULL;
      }
      free(a->pgs);
      a->pgs = NULL;
    }
  }
  free(a);
  return NULL;
}


static arrVal *arrSet_X(arr a, uint32_t ndx, uint16_t tt, arrVal *v)
{
  arrVal *p;

  p = arrGet_V((vec*)a,ndx);

  switch (p->tt) {
    case  STR: if (p->vv.str != NULL) free(p->vv.str); break;
    case  ARR: p->vv.arp = arrFree(p->vv.arp); break;
  }

  switch (tt) {
    case INT : p->vv.nmi = v->vv.nmi; break;
    case UNS : p->vv.nmu = v->vv.nmu; break;
    case STR : p->vv.str = strdup(v->vv.str); break;

  }
  p->tt = tt;
  if (ndx >= a->cnt) a->cnt = ndx+1;
  return p;
}

arrVal *arrSet_INT(arr a,uint32_t ndx, int n)
{
   arrVal v;
   v.vv.nmi = n;
   return arrSet_X(a,ndx,INT,&v);
}

arrVal *arrSet_UNS(arr a,uint32_t ndx, uint32_t n)
{
   arrVal v;
   v.vv.nmu = n;
   return arrSet_X(a,ndx,UNS,&v);
}

arrVal *arrSet_STR(arr a,uint32_t ndx, char *s)
{
   arrVal v;
   v.vv.str = s;
   return arrSet_X(a,ndx,STR,&v);
}

int32_t arrGet_INT(arr a,uint32_t ndx)
{
  arrVal *v;

  if ((v = vecGet((vec*)a,ndx)) == NULL) return 0;
  if (v->tt != INT) return 0;
  return v->vv.nmi;
}

int32_t arrGet_UNS(arr a,uint32_t ndx)
{
  arrVal *v;

  if ((v = vecGet((vec*)a,ndx)) == NULL) return 0;
  if (v->tt != UNS) return 0;
  return v->vv.nmu;
}

char *arrGet_STR(arr a,uint32_t ndx)
{
  arrVal *v;

  if ((v = vecGet((vec*)a,ndx)) == NULL) return emptystr;
  if (v->tt != STR) return emptystr;
  return v->vv.str;
}
#define bmp           vec



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

typedef  union {
  char          *str;
  uint32_t       unm;
  int32_t        snm;
  void          *ptr;
  char           chr;
} vecVal;


typedef struct mapNode {
  struct mapNode *lnk[2];
  vecVal          key;
} mapNode

typedef struct mapNodeElem {
  struct mapNode node;
  uint8_t        elem[sizeof(void *)];
} mapNodeElem;


/*******/


#if 0
char *

char *strAdd(stp_t pool, char *s)
{
  char *t;

  t = stpinsert(pool,s);
  return t;
}

void strDel(char *s)
{

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
  uint32_t        hashkey;
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

#endif

/*****************/

typedef vec_t map_t;

mapAdd(map_t T,void *e)
{
  uint32_t node = 0;
  uint32_t depth = 0;
  uint32_t node = VEC_NULL;

  while (mapPTR(t,node) != NULL) {
    if (mapCMP(t,node,e) < 0) {
      node = mapLEFT(node);
    }
    else {
      n0 = node;
      node = mapRIGHT(node);
    }
    depth++;
  }

}







/**********************/

typedef struct lstNode {
  struct lstNode *next;
  union {
     struct lstNode *nextfree;
     int8_t          elem[sizeof(void *)];
  }                 data[1];
} lstNode;

#define lstDELETED   ((void *)vecNew)

typedef struct lstVec {
  vec            vec[1];
  struct lstNode *tail;
  struct lstNode *free;
  uint32_t        cnt;
} lstVec;

#define lst_t lstVec *

#define lstHead(l) (l->tail != NULL? l->tail->next : NULL)
#define lstTail(l) (l->tail)
#define lstNodeElem(node) (node != NULL? (void *)(node->data->elem) : NULL)

#define lstFirst(l) (l->tail != NULL? (void*)(l->tail->next->data->elem) : NULL)
#define lstLast(l)  (l->tail != NULL? (void*)(l->tail->data->elem) : NULL)

#define lstnode(n)  ((lstNode *)(((uint8_t *)n)-offsetof(lstNode,data)))

void *lstNext(lst_t l, void *ref);

lst_t lstNew(uint16_t elemsz);

void *lstInsAfter(lst_t l,void *ref,void *e);
void *lstInsHead(lst_t l, void *e);
#define lstInsTail(l,e)  (lstInsHead(l,e), l->tail = l->tail->next, (void*)(l->tail->data->elem))

void *lstFreeClean(lst_t l, vecCleaner cln);
#define lstFree(l) lstFreeClean(l,NULL)

#define lstLen(l) (l->cnt)
