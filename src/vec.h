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
  uint32_t  esz;
  uint32_t  npg;
  vMark     mrk;
  uint8_t **arr;
} vec;

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

vec  *vecNew(uint32_t elemsize);
void *vecGet(vec *v, uint32_t ndx);
void *vecNext(vec *v);
void *vecSet(vec *v, uint32_t ndx, void *elem);
void *vecFree(vec *v);

uint32_t vecSize(vec *v);

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

#endif  /* VEC_H */

