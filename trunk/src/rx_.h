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
  unsigned char *bot[RX_MAXCAPT+1];
  unsigned char *eoc[RX_MAXCAPT+1];
  
  unsigned char *bol;

  unsigned char  casesensitive;
  unsigned char  escape;
  unsigned char  rxnum;
  unsigned char  failall;
  
} rx_extended;


#define END     0x00

#define CAPTR   0x00
#define BOC     0x08   /* 000 01xxx */
#define EOC     0x10   /* 000 10xxx */
#define CAPT    0x18   /* 000 11xxx */

#define SINGLE  0x20

#define MATCH   0x21   /* 001 00001 */
#define FAIL    0x22   /* 001 00010 */
#define FAILALL 0x23   /* 001 00011 */
#define PATTERN 0x24   /* 001 00100 1xxxxxxx 1xxxxxxx */
#define ONFEND  0x25   /* 001 00101 */
#define PEEKED  0x26   /* 001 00110 */

#define BKMAX   0x28   /* 001 01000 xxxxxxxx */
#define BACK    0x29   /* 001 01001 */

#define MINANY  0x2A   /* 001 01010 */
#define MIN     0x2B   /* 001 01011 */
#define BOL     0x2C   /* 001 01100 */
#define EOL     0x2D   /* 001 01101 */
#define CASE    0x2E   /* 001 01110 */
#define ESCAPE  0x2F   /* 001 01111 xxxxxxxx */

#define ESCANY  0x32   /* 001 10010 */
#define BRACED  0x33   /* 001 10011 */  
#define QSTR    0x37   /* 001 10111 */
#define NINT    0x38   /* 001 11000 */
#define NFLOAT  0x39   /* 001 11001 */
#define NHEX    0x3A   /* 001 11010 */
#define IDENT   0x3B   /* 001 11011 */
#define SPCS    0x3C   /* 001 11100 */

#define GOTO    0x40   /* 010 0xxxx 1xxxxxxx */
#define ONFAIL  0x50   /* 010 1xxxx 1xxxxxxx */

#define jmparg(p) (((((int)(p)[0]) & 0x0F) << 7) | (((int)(p)[1]) & 0x7F))

#define NOTCHR  0x60   /* 011 00000 xxxxxxxx */
#define STR     0x60   /* 011 xxxxx */ /* 0x60 - 0x7F*/

#define CCL     0x80   /* 1yx xxxxx */

#define ANYUPR  0xB0   /* 101 10000 */
#define ANYLWR  0xB1   /* 101 10001 */
#define ALNUM   0xB2   /* 101 10010 */
#define DIGIT   0xB3   /* 101 10011 */
#define XDIGIT  0xB4   /* 101 10100 */
#define SPACE   0xB5   /* 101 10101 */
#define PUNCT   0xB6   /* 101 10110 */
                               
#define CTRL    0xB7   /* 101 10111 */
#define WORDC   0xB8   /* 101 11000 */
#define SPCTAB  0xB9   /* 101 11001 */
#define ANY     0xBA   /* 101 11010 */
#define ALPHA   0xBB   /* 101 11011 */

#define ZERO    0xFA   /* 111 11100 */

static unsigned short opt_;
#define iscls(_x) (opt_=_x & 0xB0,(0xB0 <= (opt_) && (opt_) <= 0xBB))
#define okforclosure(_x) (optype(_x) == CCL)

#define optype(_n) (opt_=_n , (opt_ & CCL)? CCL : (opt_ & 0xE0))

#define STR_len(_n) ((_n) & 0x1F)
#define CCL_len(_n) ((opt_=_n & 0x3F) > 37? 0 : opt_+1 )

#define CAPT_num(_n)  (((_n) & 0x07)+1)
#define CAPT_type(_n) ((_n) & 0xF8)

int rx_isinccl_class(unsigned char n,const unsigned char *cb);
int rx_isinccl(unsigned char n, const unsigned char *ccl);


#endif /* RX__H */

