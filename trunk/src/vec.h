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
#ifdef _MSC_VER
#include <pstdint.h>
#else
#include <stdint.h>
#endif
#include <stddef.h>
#include <string.h>
#include "hul.h"

typedef union {
    uint32_t  n;
    void     *p;
} aux_t;

typedef struct {
  uint8_t     **arr;
  uint32_t      cnt;
  aux_t         aux0;
  aux_t         aux1;
  aux_t         aux2;
  aux_t         aux3;
  uint16_t      esz;
  uint16_t      ksz;
  uint16_t      npg;
  uint16_t      cur_p;  /* cur page number  */
  uint16_t      cur_s;  /* cur page size    */
  uint16_t      cur_n;  /* cur slot number  */
  uint32_t      cur_w;  /* cur water mark   */
  uint8_t *     cur_q;  /* cur slot pointer */
} vec;

vec      *vecNew  (uint16_t elemsize);
void     *vecGet  (vec *v, uint32_t ndx);
void     *vecSet  (vec *v, uint32_t ndx, void *elem);
void     *vecFree (vec *v);
uint32_t  vecSize (vec *v);

#define vecNext(v) vecGet(v, (v)->cur_w == VEC_ANYNDX? 0 : (v)->cur_w + 1)
#define vecPrev(v) (((v)->cur_w == 0 || (v)->cur_w == VEC_ANYNDX) \
                     ? NULL \
                     : vecGet(v,(v)->cur_w - 1))
#define vecCnt(v) ((v)->cnt)

#define VEC_ANYNDX   UINT32_MAX

/**********************/

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

#define stkReset(v)   (stkIsEmpty(v)? 0    : (stkDepth(v) = 0,((vec *)v)->cur_w=VEC_ANYNDX))

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

