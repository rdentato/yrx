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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

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

#define vecFree(v)         vecFreeClean(v,NULL)

#define vecGetVal(v,n,t)   ((t *)vecGet(v,n))[0]
#define vecSetVal(v,n,e,t) do { t e__ = e; vecSet(v,n,&e__);} while(0)

#define vecNxtNdx(v)       ((v)->cur_w == VEC_NULLNDX? 0 : (v)->cur_w + 1)

#define vecNext(v)         vecGet(v, vecNxtNdx(v))
#define vecPrev(v)         (((v)->cur_w == 0 || (v)->cur_w == VEC_NULLNDX) \
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

#define stp_t map_t

stp_t stpNew  (void);
void *stpFree (stp_t pool);
char *stpAdd  (stp_t pool, char *str);
char *stpGet  (stp_t pool, char *str);
char *stpDel  (stp_t pool, char *str);

#define stpCnt  mapCnt

/**********************/

#define bmp_t    vec_t

#define bmpBlkSize 4
#define bmpBlkMask (bmpBlkSize-1)

typedef uint32_t  bmpBlk[bmpBlkSize];

uint32_t bmpSet   (bmp_t b,uint32_t ndx);
uint32_t bmpClr   (bmp_t b,uint32_t ndx);
uint32_t bmpTest  (bmp_t b,uint32_t ndx);
uint32_t bmpFlip  (bmp_t b,uint32_t ndx);

#define bmpNew()  vecNew(sizeof(bmpBlk))
#define bmpFree   vecFree
#define bmpCnt    vecCnt

typedef enum {
  bmp_AND = 1, bmp_OR, bmp_NEG, bmp_ZRO, bmp_SET, bmp_SUB
} bmp_op;

void bmpOp(bmp_t a, bmp_t b, bmp_op op);


/**********************/

typedef struct blk_ {
  uint16_t slt;
  uint16_t cnt;
  uint8_t  elem[1];
} blk;

typedef blk *blk_t;

uint8_t   *blkNew    (uint8_t ty);
uint16_t   blkCnt    (uint8_t *b);
uint16_t   blkSlt    (uint8_t *b);
uint8_t   *blkSetInt (uint8_t *b, uint16_t ndx, uint32_t val, uint8_t ty);
uint8_t   *blkSetPtr (uint8_t *b, uint16_t ndx, void *val);
uint32_t   blkGetInt (uint8_t *b, uint16_t ndx, uint8_t ty);
void*      blkGetPtr (uint8_t *b, uint16_t ndx);
uint8_t   *blkFree   (uint8_t *b);

#define blkCHR 1
#define blkU16 2
#define blkU32 3
#define blkPTR 4

/**********************/

#define ulv_t blk_t

#define ulvNew()  (uint32_t *)blkNew(blkU32)
#define ulvFree(b) (uint32_t *)blkFree((uint8_t *)b)

#define ulvCnt(b)  blkCnt((uint8_t *)b)
#define ulvSlt(b)  blkSlt((uint8_t *)b)

#define ulvGet(b,n)   ((uint32_t)blkGetInt((uint8_t *)b, n, blkU32))
#define ulvSet(b,n,v) ((uint32_t *)blkSetInt((uint8_t *)b, n, (uint32_t)v, blkU32))

#define    ulvAdd(b,v) ulvSet(b, ulvCnt(b), v)

/**********************/



#endif  /* VEC_H */

