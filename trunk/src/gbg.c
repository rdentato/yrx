
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



#define lst_t vec_t


typedef struct lstNode {
  struct lstNode *next;
  int8_t          elem[sizeof(void *)];
} lstNode;


typedef struct lstVec {
  vec             nodes[1];
  vec             heads[1];
  struct mapNode *freelst;
} lstVec;

lst_t lstNew(uint16_t elemsz)
{
  lst_t l;

  l = malloc(sizeof(lstVec));
  if (l != NULL) {
    vecinit(l->nodes,elemsz + offsetof(lstNode, elem));
    vecinit(l->heads,sizeof(lstNode *));
    l->freelst = NULL;
  }
  return l;
}

void *lstIns(lst_t l, uint32_t head, void *e)
{

}

void *lstNewHead(lst_t l)
{
  lstNode *node;

  if (l->freelst != NULL) {
    node = l->freelst;
    l->freelst = node->next;
  }
  else {
    node = vecGet(l->nodes,vecCnt(l->nodes)++);
  }
  return l->elem;
}


lst



void *lstCons(lst_t l, void *e)
{

}

void *lstCar(lst_t l)
{
}

void *lstNth(lst_t l, uint32_t ndx)
{

}

lst_t lstSlice(lst_t l, uint32_t from, uint32_t to)
{

}

lst_t lstCat(lst_t l1, lst_t l2)
{
}

lst_t lstReverse(lst_t l)
{
}

void *lstLen(lst_l)
{
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




$ans = 0/* http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx */

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

  _dbgmsg("** %p %u\n",e,len);

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

stp_t stpNew()
{
  stp_t pool;

  if ((pool = malloc(sizeof(stp))) != NULL) {
    vecinit(pool->str,sizeof(char *));
    pool->msk = 3;
    pool->maxlen = 0;
  }

  return pool;
}


static void stpfree(void *e)
{
  char **s=e;
  if (s != NULL && *s != VEC_DELETED && *s != NULL) {
    _dbgmsg("-- %s\n",*s);
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

#define TOODENSE(p)  (((stpN(p) - stpCnt(p)) *4) < stpN(p))


/*
#define TOODENSE(p)  (((stpN(p) - stpCnt(p)) *4) < stpN(p))

#define TOOSPARSE(p) ( (stpCnt(p) * 2) < (stpN(p)))*/

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

  max = pool->msk+1;
  pool->msk = 2 * pool->msk +1;
  _dbgmsg(" ***** Rehash: (%u) %u,%u [%u < %u]\n",stpCnt(pool),max,pool->msk,pool->maxlen,(2*llog2(stpCnt(pool)+1)));
  stpM(pool) = 0;
  pool->maxlen = 0;
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
  for(EVER) {
    s = vecGet(pool->str,key);

    if (s == NULL || *s == NULL) return s;

    if (*s == VEC_DELETED) {
      if (del != NULL) {
       *del = s;
      }
    }
    else {
      if (strcmp(str,*s) == 0)   return s;
    }
    t = *s;
    _dbgmsg("XX %4d (%u) %s (%s)\n",key,inc, str,
     (t == VEC_DELETED)? "(del)" : (t == NULL) ? "(null)" : t);

    if (++stpM(pool) > pool->maxlen) pool->maxlen = stpM(pool);
    key = (key + inc) & pool->msk;
  }
  return NULL;

}

char *stpAdd(stp_t pool, char *str)
{
  char **s,**d;

  if (TOODENSE(pool)) stprehash(pool,DOUBLE_SIZE);

  _dbgmsg("ADD: %s\n", str);
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

#define jsw(x) h = ( h << 1 | h >> 31 ) ^ T[x]

#if 0
static uint32_t mappriority(void *node)
{

  /* this is the FNV hash! */
  uint8_t *p;
  uint32_t h = 0x811C9DC5;

  h = scrambleint((uint32_t)node);
  return h;
  p = (uint8_t *)&node;

  h = (h * 16777619) ^ p[0];
  h = (h * 16777619) ^ p[1];
  h = (h * 16777619) ^ p[2];
  h = (h * 16777619) ^ p[3];

  return h;
}
#endif




typedef struct lplNode {
  struct lplNode *nxt;
  int8_t          elem[sizeof(void *)];
} lplNode;

typedef struct lplVec {
  vec             nodes[1];
  struct lplNode *freelst;
} lplVec;

typedef struct lstHead {
  vec_t vec;
  lplNode *head;
  lplNode *tail;
} lstHead;

typedef lplVec  *lpl_t;

lpl_t  lplNew(uint16_t elemsz);
lpl_t  lplFreeClean(lpl_t pool, vecCleaner cln);
void  *lplNewList(lpl_t pool);
void  *lstNext(void *node_elem);
void   lstRelease(lpl_t pool, void *head);

#define lplFree(l) lplFreeClean(l,NULL)




#define lplNodePtr(n) ((lplNode *)(((uint8_t *)(n))-offsetof(lplNode,elem)))

lpl_t lplNew(uint16_t elemsz)
{
  lpl_t l;

  l = malloc(sizeof(lplVec));
  if (l != NULL) {
    vecinit(l->nodes,elemsz + offsetof(lplNode, elem));
    l->freelst = NULL;
  }
  return l;
}

lpl_t lplFreeClean(lpl_t pool, vecCleaner cln)
{
  uint32_t k;
  lplNode *p;

  if (pool != NULL) {
    if (cln != NULL) {
      for (k = 0; k < vecCnt(pool->nodes); k++) {
        p = vecGet(pool->nodes,k);
        if (p != NULL && p->nxt != VEC_DELETED)
          cln(p->elem);
      }
    }
    veccleanup(pool->nodes, NULL);
    free(pool);
  }
  return NULL;
}

static lplNode *newlstnode(lpl_t pool)
{
  lplNode  *node;
  lplNode **q;

  if (pool->freelst != NULL) {
    node = (void *)(pool->freelst);
    q = (void *)&(node->elem);
    pool->freelst = *q;
  } 
  else {
    node = vecGet(pool->nodes, vecCnt(pool->nodes)++);
  }
  node->nxt = NULL;

  return node;
}

void *lplNewList(lpl_t pool)
{
   lplNode *h;

   h = newlstnode(pool);
   if (h != NULL) {
     memset(h->elem, 0, pool->nodes->esz - offsetof(lplNode,elem));
   }
   return h->elem;
}

void *lstNext(void *node_elem)
{
  lplNode *node;

  if (node_elem != NULL) {
    node = lplNodePtr(node_elem)->nxt;
    if (node != NULL) return node->elem;
  }

  return NULL;
}

void *lstIns(void *node_elem)
{
  lplNode *node;

  if (node_elem != NULL) {
    node = lplNodePtr(node_elem);
    if (node != NULL) return node->elem;
  }
  return NULL;
}

void lstRelease(lpl_t pool, void *head)
{
  lplNode  *node;
  lplNode  *t;
  lplNode **q;

  if (head != NULL) {
    node = lplNodePtr(head);
    t = pool->freelst;
    do {
      q = (void *)&(node->elem);
     *q = t;
      t = node;
      node = node->nxt;
      t->nxt = VEC_DELETED;
    } while (node != NULL);
    pool->freelst = t;
  }
}




#define bmpClrAll(a)      bmpOp(a,NULL,bmp_ZRO)
#define bmpSub(a)         bmpOp(a,b,bmp_SUB)
#define bmpSetAll(a,max) (bmpSet(a,max),bmpOp(a,NULL,bmp_SET)

void bmpOp(bmp_t a, bmp_t b, bmp_op op)
{
  uint32_t *p, *q;
  uint32_t i;
  uint32_t n;

  i = bmpCnt(a);
  if (b != NULL && bmpCnt(b) > i) i = bmpCnt(b);

  if (i > 0) {
    i = (i / 32) / 4;

    do {
      p = vecGet(a,i);
      q = vecGet(b,i);
      for (n = 0; n <= bmpBlkSize; n++) {
        switch (op) {
          case bmp_AND: *p++ &=  *q++        ; break;
          case bmp_OR:  *p++ |=  *q++        ; break;
          case bmp_NEG: *p++ ^=   0xFFFFFFFF ; break;
          case bmp_ZRO: *p++  =   0x00000000 ; break;
          case bmp_SET: *p++  =   0xFFFFFFFF ; break;
          case bmp_SUB: *p++ &= ~*q++        ; break;
        }
      }
    } while (i-- > 0);
  }
}

