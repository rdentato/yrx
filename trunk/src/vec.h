#ifndef VEC_H
#define VEC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <strings.h>
#include "hul.h"


typedef struct {
  uint16_t p;  /* page number  */
  uint32_t s;  /* page size    */
  uint32_t n;  /* slot number  */
  uint32_t w;  /* water mark   */
  uint8_t *q;  /* slot pointer */
} vMark;

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
  uint16_t      esz;
  uint16_t      ksz;
  uint16_t      npg;
  uint16_t      mrk_p;  /* cur page number  */
  uint16_t      mrk_s;  /* cur page size    */
  uint16_t      mrk_n;  /* cur slot number  */
  uint32_t      mrk_w;  /* cur water mark   */
  uint8_t *     mrk_q;  /* cur slot pointer */
} vec;

vec  *vecNew(uint16_t elemsize);

void *vecGet(vec *v, uint32_t ndx);

#define vecNext(v) vecGet(v, (v)->mrk_w == VEC_ANYNDX? 0 : (v)->mrk_w + 1)

void *vecPrev(vec *v);
void *vecSet(vec *v, uint32_t ndx, void *elem);
void *vecSetL(vec *v, uint32_t ndx, void *elem,uint16_t len);
void *vecFree(vec *v);

uint32_t vecSize(vec *v);

#define vecCnt(v) ((v)->aux0.n)
#define vecAux(v) ((v)->aux2.p)

#define VEC_ISSET    0x00000001
#define VEC_ANYNDX   UINT32_MAX

void  blkDel(vec *v, void *e);
vec  *blkNew(uint16_t elemsz);
void *blkAdd(vec *v,void *e);
#define blkFree vecFree

#define stkNew        vecNew
#define stkFree       vecFree
#define stkPush(v,e)  vecSet((vec *)v,vecCnt((vec *)v),e)
#define stkIsEmpty(v) (((vec *)v) == NULL || vecCnt((vec *)v) == 0)
#define stkPop(v)     (stkIsEmpty(v)? 0    : vecCnt((vec *)v)-- )
#define stkTop(v)     (stkIsEmpty(v)? NULL : vecGet((vec *)v,vecCnt((vec *)v)-1))
#define stkReset(v)   (vecCnt((vec *)v) = 0)
#endif  /* VEC_H */

