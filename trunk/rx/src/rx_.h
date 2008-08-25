/*  YRX - rx_.h
**  (C) 2006 by Remo Dentato (rdentato@users.sourceforge.net)
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
**
*/

#ifndef RX__H
#define RX__H
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef _GCC
#include <strings.h>
#endif

#include "rx.h"

#define RX_MAXCAPT 8

typedef struct {
  unsigned char *boc[RX_MAXCAPT+1];
  unsigned char *eoc[RX_MAXCAPT+1];
  
  unsigned char *bol;

  unsigned char  casesensitive;
  unsigned char  escape;
  unsigned char  rxnum;
  unsigned char  failall;
  
} rx_extended;


#define END     0x00
#define SINGLE  0x00

#define ANYUPR  0xB0   /* 101 10001 */
#define ANYLWR  0xB1   /* 101 10010 */
#define ALNUM   0xB2   /* 101 10011 */
#define DIGIT   0xB3   /* 101 10100 */
#define XDIGIT  0xB4   /* 101 10101 */
#define SPACE   0xB5   /* 101 10110 */
#define PUNCT   0xB6   /* 101 10111 */
                   
#define CTRL    0xB7   /* 101 11000 */
#define WORDC   0xB8   /* 101 11001 */
#define SPCTAB  0xB9   /* 101 11010 */
#define ANY     0xBA   /* 101 11011 */
#define ZERO    0xBB   /* 101 11100 */
#define ALPHA   0xBC   /* 101 11101 */
#define ESCANY  0xBD   /* 101 11110 */

#define iscls(_x) (0xB0 <= (_x) && (_x) <= 0xBD)

#define okforclosure(_x) (optype(_x) == CCL)

#define OPT     0x0F   /* 000 01111 */  /* ? */
#define REPT    0x10   /* 000 10000 */  /* < */
#define REPT0   0x11   /* 000 10001 */  /* * */
#define REPT1   0x12   /* 000 10010 */  /* + */
#define BRACED  0x13   /* 000 10011 */  
#define BOL     0x14   /* 000 10100 */
#define EOL     0x15   /* 000 10101 */
#define ESCAPE  0x16   /* 000 10111 */
#define QSTR    0x17   /* 000 10111 */
#define NINT    0x18   /* 000 11000 */
#define NFLOAT  0x19   /* 000 11001 */
#define NHEX    0x1A   /* 000 11010 */
#define IDENT   0x1B   /* 000 11011 */
#define SPCS    0x1C   /* 000 11100 */
#define CASE    0x1D   /* 000 11101 */
#if 0
#define BOW     0x1E   /* 000 11110 */
#define EOW     0x1F   /* 000 11111 */
#else
#define MAX     0x1E   /* 000 11110 */
#define MIN     0x1F   /* 000 11111 */
#endif 

#define ONFEND  0x20   /* 001 00000 */
#define FAIL    0x21   /* 001 00001 */
#define FAILALL 0x22   /* 001 00010 */
#define PATTERN 0x23   /* 001 00011 */
#define BKMAX   0x24   /* 001 00100 */
#define EMPTY   0x25   /* 001 00101 */
#define PEEKED  0x26   /* 001 00111 */
#define MATCH   0x27   /* 001 00111 */

#define CAPTR   0x20
#define BOC     0x28   /* 001 01xxx */
#define EOC     0x30   /* 001 10xxx */
#define CAPT    0x38   /* 001 11xxx */

#define GOTO    0x40   /* 010 0xxxx 1xxxxxxx*/
#define ONFAIL  0x50   /* 010 1xxxx 1xxxxxxx*/

#define jmparg(p) (((((int)(p)[0]) & 0x0F) << 7) | (((int)(p)[1]) & 0x7F))

#define STR     0x60   /* 011 xxxxx */ /* 0x60 - 0x7F*/

#define CCL     0x80   /* 1yx xxxxx */

#define NOTCHR  0xFE   /* 111 11110 */
#define NOTCLS  0xFF   /* 111 11111 */

static unsigned short opt_;
#define optype(_n) (opt_=_n , ((opt_ < 0x28) ? SINGLE : (opt_ & CCL)? CCL : (opt_ & 0xE0)))

#define STR_len(_n) ((_n) & 0x1F)
#define CCL_len(_n) (opt_=_n & 0x3F, (opt_ > 37? 0 : opt_))

#define CAPT_num(_n)  (((_n) & 0x07)+1)
#define CAPT_type(_n) ((_n) & 0xF8)

int rx_isinccl_class(unsigned char n,const unsigned char *cb);
int rx_isinccl(unsigned char n, const unsigned char *ccl);


#endif /* RX__H */

