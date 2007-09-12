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

/*

@misc{ bagwell-fast,
  author = "Phil Bagwell",
  title = "Fast Functional Lists, Hash-Lists, Deques and Variable Length Arrays",
  url = "citeseer.ist.psu.edu/bagwell02fast.html" }
  
*/


#include "vls.h"

static void vlsInit(vls *v, int32_t esz, uint32_t ksz)
{
  if (v) {
    /* Ensure an element can store a (void *), will be used for the freelist */
    if (esz < sizeof(void *)) esz =  sizeof(void *);
    if (ksz < esz) esz = ksz;
    v->esz   = esz;
    v->ksz   = ksz;
    v->cur   = 0;
    v->len   = 0;
    v->flg   = 0;
    v->nxt   = NULL;
    v->aux   = NULL;   
  }
}

vls *vlsNew(uint16_t esz)
{
  vls *v;

  v = malloc(sizeof(vls));
  vecInit(v,esz,esz);
  return v;
}

void *vlsGet(vls *v,uint32_t ndx)
{
  vls_ptr *vp;
  void *p = NULL;
  uint32_t k = 0;
  uint32_t t = 1;
  
  if (v != NULL) {
    vp = v->ptr;
    while (vp != NULL && ndx > t) {
      t += 1 << ( k << 1); /* 2^(2*k) */
      k++;
      vp = vp->next;
    }
  }
  return p;
}