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
#else
#include <stdint.h>
#endif

#ifdef __POCC__
char * __cdecl strdup(const char *);
#endif

#include "hul.h"

/**********************/

typedef struct {
  uint16_t      esz;
  uint16_t      npg;
  uint16_t      aux;
  uint16_t      cur_p;  /* cur page number  */
  uint16_t      cur_s;  /* cur page size    */
  uint16_t      cur_n;  /* cur slot number  */
  uint32_t      cur_w;  /* cur water mark   */
  uint8_t      *cur_q;  /* cur slot pointer */
  uint8_t     **pgs;
  uint32_t      cnt;
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
#define vecSetVal(v,n,e,t)

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
#define stkPush(v,e)  vecSet(v,stkDepth(v),e)
#define stkIsEmpty(v) ((v) == NULL || stkDepth(v) == 0)
#define stkPop(v)     (stkIsEmpty(v)? 0    : (stkDepth(v)--))
#define stkTop(v)     (stkIsEmpty(v)? NULL : vecGet(v, stkDepth(v)-1))

#define stkPushVal(v,e,t)  do {t e__ = e; stkPush(v,&e__);} while(0)
#define stkTopVal(v,t)     (stkIsEmpty(v) \
                              ? (t)NULL \
                              : vecGetVal(v, stkDepth(v)-1,t))

#define stkNth(v,n)   (stkIsEmpty(v) || n >= stkDepth(v)\
                          ? NULL\
                          : vecGet(v, n))

#define stkReset(v)   (v == NULL\
                          ? 0 \
                          : ( stkDepth(v) = 0,\
                              (v)->cur_w = VEC_NULLNDX) )

/**********************/

#define bmp_t    vec_t

bmp_t   bmpNew   (void);
uint8_t bmpSet   (bmp_t b,uint32_t ndx);
uint8_t bmpClr   (bmp_t b,uint32_t ndx);
uint8_t bmpTest  (bmp_t b,uint32_t ndx);
uint8_t bmpFlip  (bmp_t b,uint32_t ndx);

#define bmpBlkSize 16
#define bmpBlkMask (bmpBlkSize-1)

#define bmpNew() vecNew(bmpBlkSize)
#define bmpFree vecFree

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

#endif  /* VEC_H */
