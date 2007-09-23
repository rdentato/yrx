#ifndef VEC_H
#define VEC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <strings.h>
#include "hul.h"


typedef struct {
  uint32_t s;  /* page size    */
  uint32_t p;  /* page number  */
  uint32_t n;  /* slot number  */
  uint32_t w;  /* water mark   */
  uint8_t *q;  /* slot pointer */
} vMark;

typedef struct {
  uint16_t      esz;
  uint16_t      npg;
  uint16_t      ksz;
  uint16_t      flg;
  uint32_t      lst;
  uint32_t      cnt;
  void         *aux;
  uint8_t     **arr;
  vMark         mrk;
} vec;

vec  *vecNew(uint16_t elemsize);

void *vecGet(vec *v, uint32_t ndx);

#define vecNext(v) vecGet(v,(v)->mrk.w + 1)

void *vecPrev(vec *v);
void *vecSet(vec *v, uint32_t ndx, void *elem);
void *vecSetL(vec *v, uint32_t ndx, void *elem,uint16_t len);
void *vecFree(vec *v);

uint32_t vecSize(vec *v);

#define vecCnt(v) ((v)->cnt)
#define vecNdx(v) ((vec *)((v)->aux))

#define VEC_ISSET    0x00000001
#define VEC_ANYNDX   UINT32_MAX

#define vecAdd(v,e) vecSet(v,VEC_ANYNDX,e);

vec *setNew(uint16_t esz,uint16_t ksz);

void *setAdd(vec *v,void *e);
void *setGet(vec *v,void *e);
void setDel(vec *v,void *elem);

#define setForeach(v,i,p) for (i=0, p = *((void **)vecGet(vecNdx(v),0));\
                               i < vecCnt(vecNdx(v));\
                               i++, p = *((void **)vecNext(vecNdx(v))) )

#define setFree vecFree

#endif  /* VEC_H */

