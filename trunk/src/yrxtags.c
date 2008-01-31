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

#include "yrx.h"

uint8_t *yrxTagsStr(tagset_t a)
{
  uint8_t *s;
  char *p;
  uint16_t k=0;
  
  s = yrxBufChk(512);
 
  p = (char *)s;
  if (a != NULL) {
    for (k=0; k < ulvCnt(a); k++) {
      sprintf(p,"%c%X ", yrxTagType(a[k]), yrxTagExpr(a[k]));
      while (*p) p++;
      
      if (yrxTagDelta(a[k]) > 0) {
        sprintf(p-1,"_%X ", yrxTagDelta(a[k]));
        while (*p) p++;
      }
    }
  }
  
  *p = '\0';
  return s;
}

tagset_t yrxTagset(tag_t tag)
{
  tagset_t ts;
  
  if (tag == yrxTagNone) return NULL;
  
  ts = ulvNew();
  if (ts != NULL) {
    ts = ulvSet(ts,0,tag);
  }
  return ts;
}

tagset_t yrxTagsFree(tagset_t a)
{
  return ulvFree(a);
}

static tag_t tag_sel(tag_t a, tag_t b)
{
  tag_t t;
  
  if (a > b) { t = a; a = b; b = t; }
  
  if (yrxTagType(a) > TAG_CB(0)) return b; /* max */
  else return a; /* min */
}

#define tag_cmpExprType(a,b) ((int)((a) >> 16) - (int)((b) >> 16))
#define tag_cmpExpr(a,b)     ((int)((a) >> 24) - (int)((b) >> 24))

tagset_t yrxTagsUnion(tagset_t a, tagset_t b)
{
  uint32_t j=0,k=0;
  tagset_t c=NULL;
  int cmp;
  
  if (a == b || b == NULL)
    return a;

  if (a == NULL)
    return yrxTagsDup(b);

  while (j < ulvCnt(a)  && k < ulvCnt(b)) {
    cmp = tag_cmpExprType(a[j],b[k]);
    if (cmp < 0)
      c = ulvAdd(c,a[j++]);
    else if (cmp > 0)
      c = ulvAdd(c,b[k++]);
    else  /* Same expression and same type! */
      c = ulvAdd(c, tag_sel(a[j++], b[k++]));
  }
  while (j < ulvCnt(a))
    c = ulvAdd(c,a[j++]);
    
  while (k < ulvCnt(b))
    c = ulvAdd(c,b[k++]);
    
  ulvFree(a);
  return c;
}

tagset_t yrxTagsDifference(tagset_t a, tagset_t b)
{
  uint32_t j=0,k=0;
  tagset_t c=NULL;
  int cmp;
  
  if (a != b && a != NULL) {
    if (b == NULL)
      return a;
    
    while (j < ulvCnt(a)  && k < ulvCnt(b)) {
      cmp = tag_cmpExprType(a[j],b[k]);
      if (cmp < 0)
        c = ulvAdd(c,a[j++]);
      else if (cmp > 0)
        k++;
      else { /* Same expression and same type! */
        j++; k++;
      }
    }
    while (j < ulvCnt(a))
      c = ulvAdd(c,a[j++]);
  }  
  ulvFree(a);
  return c;  
}

tagset_t yrxTagsIntersection(tagset_t a, tagset_t b)
{
  uint32_t j=0,k=0;
  tagset_t c=NULL;
  int cmp;
  int expr;
  
  if (a == b || b == NULL)
    return a;
  
  if (a == NULL)
    return yrxTagsDup(b);
    
  while (j < ulvCnt(a)  && k < ulvCnt(b)) {
    cmp = tag_cmpExpr(a[j],b[k]);  
    if (cmp < 0)
      c = ulvAdd(c,a[j++]);
    else if (cmp > 0)
      c = ulvAdd(c,b[k++]);
    else { /* Same expression! */
      expr = yrxTagExpr(a[j]);
      do  {
        cmp = tag_cmpExprType(a[j],b[k]);
        if (cmp < 0)
          j++;
        else if (cmp > 0)
          k++;
        else { /* Same expression and same type. */
          if (a[j] == b[k]) /*  also same delta! */
            c = ulvAdd(c, a[j]);
          k++; j++;      
        }
      } while ( j < ulvCnt(a) && k < ulvCnt(b) && 
                                        tag_cmpExpr(a[j],b[k] == 0));
      /* discard other tags of the same expression */
      while (j < ulvCnt(a) && yrxTagExpr(a[j]) == expr)
        j++;
      while (k < ulvCnt(b) && yrxTagExpr(b[k]) == expr)
        k++;
    }
  }
  while (j < ulvCnt(a))
    c = ulvAdd(c,a[j++]);
  
  while (k < ulvCnt(b))
    c = ulvAdd(c,b[k++]);
    
  ulvFree(a);
  return c;  
}

int yrxTagsEmpty(tagset_t a)
{
  return (a == NULL || ulvCnt(a) == 0);
}

tagset_t yrxTagsIncrement(tagset_t a)
{
  uint32_t k;

  if (a != NULL) {
    for (k = 0; k < ulvCnt(a); k++) {
      if (yrxTagDelta(a[k]) < 65530) a[k]++;
      else err(801,"Tag displacement out or range");
    }
  }
  return a;
}

tagset_t yrxTagsDecrement(tagset_t a)
{
  uint32_t k;

  if (a != NULL) {
    for (k = 0; k < ulvCnt(a); k++) {
      if (yrxTagDelta(a[k]) > 0) a[k]--;
      else err(805,"Tag displacement out or range");
    }
  }
  return a;
}
