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


#ifndef VEC_H
#define VEC_H


/* = Standard |#include|s */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
#include "pstdint.h"
#define strdup _strdup
#else
#include <stdint.h>
#endif

#include "hul.h"

/**********************/

#ifndef vecErr
#define vecErr err
#endif

/**********************/
typedef struct {
  uint16_t      esz;
  uint16_t      npg;
  uint32_t      cnt;
  uint8_t     **pgs;
  uint16_t      aux;
  uint16_t      cur_p;  /* cur page number  */
  uint16_t      cur_s;  /* cur page size    */
  uint16_t      cur_n;  /* cur slot number  */
  uint32_t      cur_w;  /* cur water mark   */
  uint8_t      *cur_q;  /* cur slot pointer */
} vec;

typedef vec *vec_t;

typedef void (*vecCleaner)(void *);

vec      *vecNew        (uint16_t elemsize);
void     *vecGet        (vec_t v, uint32_t ndx);
void     *vecSet        (vec_t v, uint32_t ndx, void *elem);
void     *vecFreeClean  (vec_t v, vecCleaner cln);
uint32_t  vecSize       (vec_t v);

#define   vecFree(v)    vecFreeClean(v,NULL)

#define vecGetVal(v,n,t)   (*((t *)vecGet(v,n)))
#define vecSetVal(v,n,e,t) do { t e__ = e; vecSet(v,n,&e__);} while(0)

#define vecNxtNdx(v)  ((v)->cur_w == VEC_NULLNDX? 0 : (v)->cur_w + 1)

#define vecNext(v)     vecGet(v, vecNxtNdx(v))
#define vecPrev(v)     (((v)->cur_w == 0 || (v)->cur_w == VEC_NULLNDX) \
                           ? NULL \
                           : vecGet(v,(v)->cur_w - 1))

#define vecAdd(v,e)    vecSet(v, vecCnt(v), e)

#define vecCnt(v)      ((v)->cnt)
#define vecPos(v)      ((v)->cur_w)
#define vecFirst(v)    (vecCnt(v)>0? vecGet(v,0) : NULL);
#define vecLast(v)     (vecCnt(v)>0? vecGet(v,vecCnt(v)-1) : NULL);

#define VEC_NULLNDX   UINT32_MAX
#define VEC_DELETED   ((void*)vecNew)

/**********************/

#define stk_t         vec_t

#define stkNew        vecNew
#define stkFree(v)    vecFree(v)
#define stkDepth(v)   ((v)->cnt)
#define stkPush(v,e)  vecSet((vec_t)v, stkDepth(v), e)
#define stkIsEmpty(v) ((v) == NULL || stkDepth(v) == 0)
#define stkPop(v)     (stkIsEmpty(v)? 0    : stkDepth(v)--)
#define stkTop(v)     (stkIsEmpty(v)? NULL : vecGet(v, stkDepth(v)-1))

#define stkPushVal(v,e,t)  do {t e__ = e; stkPush(v,&e__);} while(0)
#define stkTopVal(v,t)     (stkIsEmpty(v) \
                              ? (t)NULL \
                              : vecGetVal(v, stkDepth(v)-1, t))

#define stkNth(v,n)   (stkIsEmpty(v) || n >= stkDepth(v)\
                          ? NULL\
                          : vecGet(v, n))

#define stkReset(v)   (v == NULL\
                          ? 0 \
                          : ( stkDepth(v) = 0,\
                              (v)->cur_w = VEC_NULLNDX) )

/**********************/

#define buf_t       vec_t

uint32_t   bufSeek  (buf_t b,uint32_t pos);
uint32_t   bufPos   (buf_t b);
int        bufGetc  (buf_t b);
int        bufGets  (buf_t b,char *s);
uint32_t   bufPuts  (buf_t b, char *s);
uint32_t   bufPutc  (buf_t b,char c);
buf_t      bufNew   (void);

#define    bufFree  vecFree
#define    bufLen   ((b)->cnt)

/**********************/

#define mapRoot(node)     ((node)->root)
#define mapLnkLeft(node)  ((node)->lnk[0])
#define mapLnkRight(node) ((node)->lnk[1])
#define mapLeft(node)     (node == NULL ? NULL : mapLnkLeft(node))
#define mapRight(node)    (node == NULL ? NULL : mapLnkRight(node))
#define mapKeySz(map)     ((map)->nodes->aux)
#define mapCnt(map)       ((map)->cnt)
#define mapNodePtr(p)     (p == NULL? NULL : (void *)(((char *)(p)) - offsetof(mapNode, elem)))

typedef int (*mapCmp_t)();

typedef struct mapNode {
  struct mapNode *lnk[2];
  int8_t          elem[sizeof(void *)];
} mapNode;

typedef struct mapVec {
  vec             nodes[1];
  vec             stack[1];
  struct mapNode *root;
  struct mapNode *freelst;
  uint32_t        cnt;
  mapCmp_t        cmp;
} mapVec;

typedef mapVec *map_t;

map_t    mapNew         (uint16_t elemsz, int (*cmp)());
map_t    mapFreeClean   (map_t m, vecCleaner cln);
void    *mapAdd         (map_t m, void *e);
void     mapDel         (map_t m, void *e);
void    *mapGet         (map_t m, void *e);
void    *mapFirst       (map_t m);
void    *mapNext        (map_t m);

uint32_t mapMaxDepth    (map_t m);

#define mapFree(m) mapFreeClean(m,NULL)


/**********************/

typedef struct blk_ {
  uint16_t slt;
  uint16_t cnt;
  uint8_t  elem[1];
} blk;

typedef blk *blk_t;

uint8_t   *blkNew    (uint8_t ty);
uint8_t   *blkSetInt (uint8_t *b, uint16_t ndx, uint32_t val, uint8_t ty);
uint8_t   *blkSetPtr (uint8_t *b, uint16_t ndx, void *val);
uint32_t   blkGetInt (uint8_t *b, uint16_t ndx, uint8_t ty);
void*      blkGetPtr (uint8_t *b, uint16_t ndx);
uint8_t   *blkFree   (uint8_t *b);
uint8_t   *blkAppend (uint8_t *b, uint8_t *a, uint8_t sz);
uint8_t   *blkPushInt(uint8_t *b, uint32_t val, uint8_t ty);
uint32_t   blkTopInt (uint8_t *b, uint8_t  ty);
uint32_t   blkPopInt (uint8_t *b, uint8_t  ty);

#define blkCHR 1
#define blkU16 2
#define blkU32 3
#define blkPTR 4

#define blkCHRsz sizeof(char)
#define blkU16sz sizeof(uint16_t)
#define blkU32sz sizeof(uint32_t)
#define blsPTRsz sizeof(void *)

int blkU32cmp (const void *a, const void *b);
int blkU16cmp (const void *a, const void *b);
int blkCHRcmp (const void *a, const void *b);

void blkUniq(uint8_t *b,uint8_t sz);

#define blkNodePtr(b) ((blk_t)(((uint8_t *)b) - offsetof(blk,elem)))
#define blkCnt(b)     (blkNodePtr(b)->cnt)
#define blkSlt(b)     (blkNodePtr(b)->slt)


/**********************/

typedef uint32_t *ulv_t ;

#define ulvNew()      (ulv_t)blkNew(blkU32)
#define ulvFree(b)    (ulv_t)blkFree((uint8_t *)b)

#define ulvCnt        blkCnt
#define ulvSlt        blkSlt

#define ulvGet(b,n)   ((uint32_t)blkGetInt((uint8_t *)b, n, blkU32))
#define ulvSet(b,n,v) ((ulv_t)blkSetInt((uint8_t *)b, n, (uint32_t)v, blkU32))

#define ulvAdd(b,v)     ulvSet(b, ulvCnt(b), v)
#define ulvAppend(a,b)  (ulv_t)blkAppend(a,b,blkU32sz)
#define ulvSort(b)      qsort(b, ulvCnt(b), blkU32sz, blkU32cmp)
#define ulvUniq(b)      blkUniq((uint8_t *)b, blkU32sz)
#define ulvPush(b,v)    ((bit_t)blkPushInt((uint8_t *)b,v,blkU32))
#define ulvPop(b)       ((uint32_t)blkPopInt((uint8_t *)b,blkU32))
#define ulvTop(b)       ((uint32_t)blkTopInt((uint8_t *)b,blkU32))

/**********************/

typedef uint16_t *usv_t ;

#define usvNew()        (usv_t)blkNew(blkU16)
#define usvFree(b)      (usv_t)blkFree((uint8_t *)b)

#define usvCnt          blkCnt
#define usvSlt          blkSlt

#define usvGet(b,n)     ((uint16_t)blkGetInt((uint8_t *)b, n, blkU16))
#define usvSet(b,n,v)   ((usv_t)blkSetInt((uint8_t *)b, n, (uint16_t)v, blkU16))

#define usvAdd(b,v)     usvSet(b, usvCnt(b), v)
#define usvAppend(a,b)  (usv_t)blkAppend((uint8_t *)a,(uint8_t *)b,blkU16sz)
#define usvSort(b)      qsort(b, usvCnt(b), blkU16sz, blkU16cmp)
#define usvUniq(b)      blkUniq((uint8_t *)b, blkU16sz)

/**********************/

typedef uint8_t *ucv_t ;

#define ucvNew()        (ucv_t)blkNew(blkCHR)
#define ucvFree(b)      (ucv_t)blkFree(b)

#define ucvCnt          blkCnt
#define ucvSlt          blkSlt

#define ucvGet(b,n)     ((uint8_t)blkGetInt(b, n, blkCHR))
#define ucvSet(b,n,v)   ((ucv_t)blkSetInt(b, n, (uint8_t)v, blkCHR))

#define ucvAdd(b,v)     ucvSet(b, ucvCnt(b), v)
#define ucvAppend(a,b)  (ucv_t)blkAppend((uint8_t *)a,(uint8_t *)b,blkCHRsz)
#define ucvSort(b)      qsort(b, ucvCnt(b), blkCHRsz, blkCHRcmp)
#define ucvUniq(b)      blkUniq((uint8_t *)b, blkCHRsz)


/**********************/

typedef void **vpv_t ;

#define vpvNew()      (vpv_t)blkNew(blkPTR)
#define vpvFree(b)    (vpv_t)blkFree((uint8_t *)b)

#define vpvCnt        blkCnt
#define vpvSlt        blkSlt

#define vpvGet(b,n)   ((void *)blkGetPtr((uint8_t *)b, n))
#define vpvSet(b,n,v) ((vpv_t)blkSetPtr((uint8_t *)b, n, (void *)v))

#define vpvAdd(b,v)   vpvSet(b, vpvCnt(b), v)

/**********************/

typedef ulv_t bit_t;

#define bitNew        ulvNew
#define bitFree       ulvFree

#define bitSet(b,n)  ((bit_t)ulvSet(b, ((n)>>5), \
                             ulvGet(b, (n)>> 5) | (1 << ((n) & 0x1F))))

#define bitClr(b,n)  ((bit_t)ulvSet(b, ((n)>>5), \
                             ulvGet(b, (n)>> 5) & ~(1 << ((n) & 0x1F))))

#define bitTest(b,n)  (ulvGet(b, (n)>>5) & (1 << ((n) & 0x1F)))

#define bitFlip(b,n) ((bit_t)ulvSet(b, ((n)>>5), \
                             ulvGet(b, (n)>> 5) ^ (1 << ((n) & 0x1F))))

bit_t bitNeg(bit_t b, uint32_t max);
                              
/**********************/


#endif  /* VEC_H */

