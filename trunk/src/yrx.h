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

#ifndef YRX_H
#define YRX_H

typedef struct {
  unsigned char match;
  unsigned char ncapt;
  void *capt;
} yrxResult;

#define yrxMatch(y)      (y)->match
#define yrxNumCapt(y)    (y)->ncapt

#define yrxBegin(y,n) (((n) >= (y)->ncapt)? \
                         (yrxPosType)0 : \
                         ((yrxPosType *)((y)->capt))[2*(n)])
                         
#define yrxEnd(y,n)   (((n) >= (y)->ncapt)? \
                         (yrxPosType)0 : \
                         ((yrxPosType *)((y)->capt))[2*(n)+1])

#define yrxLen(y,n)   (((n) >= (y)->ncapt)? 0 : \
                           ((yrxPosType *)((y)->capt))[2*(n)+1] - \
                           ((yrxPosType *)((y)->capt))[2*(n)])

#define yrxFPuts(y,n,f) while (yrxLen(y,n) > 0) { \
                          fwrite(yrxBegin(y,n),1,yrxLen(y,n),f);\
                          break;\
                        }
                        
#define yrxPuts(y,n) yrxFPuts(y,n,stdout) 

#endif /* YRX_H */
