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

#include "yrxlib.h"

static map_t lblpool;

/**************************************************/

/* = Handling labels */


/**************************************************/

#define lbl_clr(b,c) (b[(c)>>4] &= ~(1<<((c) & 0xF)))

#define lbl_set(b,c) (b[(c)>>4] |=  (1<<((c) & 0xF)))

#define lbl_tst(b,c) (b[(c)>>4]  &  (1<<((c) & 0xF)))

#define lbl_zro(b)   memset(b,0,sizeof(lbl_bits))

#define lbl_cpy(a,b) memcpy(a,b,sizeof(lbl_bits))

#define lbl_neg(b)     do {uint8_t i; for(i=0; i<16; i++) (b)[i] ^=  0xFFFF; } while (0)
#define lbl_minus(a,b) do {uint8_t i; for(i=0; i<16; i++) (a)[i] &= ~(b)[i]; } while (0)
#define lbl_or(a,b)    do {uint8_t i; for(i=0; i<16; i++) (a)[i] |=  (b)[i]; } while (0)
#define lbl_and(a,b)   do {uint8_t i; for(i=0; i<16; i++) (a)[i] &=  (b)[i]; } while (0)

#define lbl_isempty(b) (((b) == yrxLblEpsilon) || ((b) == yrxLblLambda) ||\
                        (!((b)[0] || (b)[1] || (b)[2] || (b)[3] || \
                           (b)[4] || (b)[5] || (b)[6] || (b)[7] || \
                           (b)[8] || (b)[9] || (b)[10]|| (b)[11]|| \
                           (b)[12]|| (b)[13]|| (b)[14]|| (b)[15])))

#define lbl_eq(a,b)    (((a) == (b)) || \
                        ((a) && (b) && (memcmp(a,b,sizeof(lbl_bits)) == 0)))

#define lbl(l) mapAdd(lblpool, l);

static int c__;

#define hex(c) ((c__=c),('0' <= c__ && c__ <= '9')? c__ - '0' :\
                        ('A' <= c__ && c__ <= 'F')? c__ - 'A' + 10:\
                        ('a' <= c__ && c__ <= 'f')? c__ - 'a' + 10: -1)\

#define oct(c) ((c__=c),('0' <= c__ && c__ <= '7')? c__ - '0' : -1)

static lbl_t lbl_bmp(char *s)
{
  int8_t negate = 0;
  int8_t range = 0;
  int    last = ' ';
  int    c;
  int    i = 0;
  int    h;

  static lbl_bits ll;
  lbl_bits l2;

  lbl_zro(ll);
  if (s && s[0]) {

    if (*s == '[') {
      s++;
      if (*s == '^') {s++; negate = 1;}
    }

    c = *s;
    while ( c != '\0' && c != ']') {
      if (c == '-')
        range = 1;
      else if (c == '.') {
        lbl_zro(ll); lbl_neg(ll); break;
      }
      else {
        if (c == '\\') {
          s++;
          if (*s == '\0') yrxParseErr(511,"Unexpected \\");

          c = -1;

          lbl_zro(l2);

          switch (*s) {
            case 'h':
            case 'H': for (last='A'; last <= 'F'; last++) lbl_set(l2,last);
                      for (last='a'; last <= 'f'; last++) lbl_set(l2,last);
            case 'd':
            case 'D': for (last='0'; last <= '9'; last++) lbl_set(l2,last);
                      break;

            case 'o':
            case 'O': for (last='0'; last <= '7'; last++) lbl_set(l2,last);
                          break;

            case 'u':
            case 'U': for (last='A'; last <= 'Z'; last++) lbl_set(l2,last);
                          break;

            case 'w':
            case 'W': lbl_set(l2,'_');
                      for (last='0'; last <= '9'; last++) lbl_set(l2,last);
            case 'a':
            case 'A': for (last='A'; last <= 'Z'; last++) lbl_set(l2,last);
            case 'l':
            case 'L': for (last='a'; last <= 'z'; last++) lbl_set(l2,last);
                          break;

            case 's':
            case 'S': for (last=8; last <= 13; last++) lbl_set(l2,last);
                          lbl_set(l2,' ');
                          break;

            case 'y':
            case 'Y': lbl_set(l2,' ');lbl_set(l2,'\t');
                          break;

            case 'n': c='\n'; break;
            case 'r': c='\r'; break;
            case 'b': c='\b'; break;
            case 't': c='\t'; break;
            case 'v': c='\v'; break;
            case 'f': c='\f'; break;

            case 'x': c = 0;
                      if ((h = hex(s[1])) >= 0) {c = h; s++; }
                      if ((h = hex(s[1])) >= 0) {c = (c<<4) | h; s++;}

                      break;

            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                      c = oct(*s);
                      if ((h = oct(s[1]))>=0) {c = (c<<3) | h ; s++;}
                      if ((h = oct(s[1]))>=0) {c = (c<<3) | h ; s++;}
                      break;

            default : c = *s;

          }
        }
        if (c >= 0) {
          if (range)
            while (++last < c) lbl_set(ll,last);

          last = c;
          lbl_set(ll,last);
        }
        else {
         if (isupper(s[i])) lbl_neg(l2);
         lbl_or(ll,l2);
         lbl_zro(l2);
        }
        range = 0;
      }
      c = *++s;
    }
    if (range)  lbl_set(ll,'-');
    if (negate) lbl_neg(ll);
  }
  return &ll[0];
}

static int lbl_rng(lbl_t bmp, uint16_t a, uint16_t *min, uint16_t *max)
{
  uint16_t b;

  b = a;
  if ((a & 0x0F) == 0)
    while ((a <= 255) && (bmp[a>>4] == 0)) a += 16;

  while (a <=255 && !lbl_tst(bmp,a)) a++;

  if (a > 255) return 0;
  b=a+1;
  while (b <= 255 && lbl_tst(bmp,b)) b++;
  b=b-1;

 *min = a;
 *max = b;
  return 1;
}

uint8_t *yrxLblPairs(lbl_t lb)
{
  uint8_t *s;
  uint16_t a,b;

  int i=0;

  s = yrxBufChk(512);

  if (!yrxLblEmpty(lb)) {
    a = 0;
    while (lbl_rng(lb, a, &a, &b) != 0) {
      s[i++] = (uint8_t)a;
      s[i++] = (uint8_t)b;
      a = b +1 ;
    }
  }
  s[i++] = 1;
  s[i] = '\0';
  return s;
}


static char *lbl_chr(char *s, uint16_t c)
{
  if (c <= 32   || c > 126  || c == '\\' || c == '"' ||
      c == '\'' || c == '[' || c == ']'  || c == '-' ) {
    sprintf(s,"\\x%02X",c);
    s += 3;
  }
  else {
    *s = (char)c;
  }
  s++;
  return s;
}


char *yrxLblStr(lbl_t lb)
{
  ucv_t s;
  uint16_t a,b;

  int i=0;

  s = yrxBufChk(512);

  if (!yrxLblEmpty(lb)) {
    a = 0;
    while (lbl_rng(lb, a, &a, &b) != 0) {
      s = lbl_chr(s, a);
      if (a != b) {
        if (b > (a+1)) *s++ = '-';
        s = lbl_chr(s, b);
      }
      a = b +1 ;
    }
  }

  s[i] = '\0';
  return yrxBuf;
}

/***************/

void yrxLblClean(void)
{
  lblpool = mapFree(lblpool); 
}

void yrxLblInit(void)
{
  lblpool = mapNew(sizeof(lbl_bits), NULL);

  if ( lblpool == NULL ) 
    err(601,yrxStrNoMem);
    
  yrxLblEpsilon = NULL;
  yrxLblLambda = (lbl_t)lblpool;
  
}


/****************/

lbl_t yrxLabel(char *l)
{
  return lbl(lbl_bmp(l));
}

lbl_t yrxLblUnion(lbl_t a, lbl_t b)
{
  lbl_bits c;
  
  if (yrxLblEmpty(a)) return b;
  if (yrxLblEmpty(b)) return a;
  
  lbl_cpy(c,a);
  lbl_or(c,b);
  
  return lbl(c);
}

lbl_t yrxLblIntersection(lbl_t a, lbl_t b)
{
  lbl_bits c;
  
  if (yrxLblEmpty(a) || yrxLblEmpty(b)) return yrxLblEpsilon;
  
  lbl_cpy(c,a);
  lbl_and(c,b);
  
  return lbl(c); 
}

lbl_t yrxLblMinus(lbl_t a, lbl_t b)
{
  lbl_bits c;
  
  if (yrxLblEmpty(a)) return yrxLblEpsilon;
  if (yrxLblEmpty(b)) return a;

  lbl_cpy(c,a);
  lbl_minus(c,b);
  
  return lbl(c);
}

int yrxLblEqual(lbl_t a, lbl_t b)
{
  return lbl_eq(a,b);
}

int yrxLblEmpty(lbl_t b)
{
  return lbl_isempty(b);
}

