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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

#define HUL_MAIN
#include "hul.h"
#include "vec.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef BOOL
#define BOOL uint8_t
#endif


/**************************************************/
typedef struct Arc {
  struct Arc  *next;
  uint16_t    *tag;
  uint16_t   **lbl;
} Arc;


vec *arcs = NULL;
vec *states = NULL;

/**************************************************/
typedef uint16_t Lbl[17];
vec *lbls = NULL;

/**************************************************/

char     *cur_rx;
int       cur_pos;
uint16_t  cur_nrx;
uint8_t   esc;
uint16_t  capt;

uint16_t  cur_state;
uint16_t  nstates;


/**************************************************/

char *str =  NULL;
int   str_len = 0;
char *str2 =  NULL;

char *str_chk(int n)
{
  if (str == NULL || str_len < n) {
    str = realloc(str,n+4);
  }
  return str;
}

char *str_set(int i,int j)
{
  int n;

  str = str_chk(j-i+2);

  if (str != NULL) {
    str[0] = '@';
    for (n=1; i<j;n++,i++) str[n] = cur_rx[i];
    str[n] = '\0';
  }
  return str;
}

char *str_dup(char *s)
{
  if (s == NULL) s = str;
  if (str2) free(str2);
  str2 = strdup(s);
  return str2; 
}

/*************************************/

#define TAG_FIN  0xFF
#define TAG_MRK  0x7F
#define TAG_CB   0x00
#define TAG_CE   0x80
#define TAG_NONE 0x00

#define tag_code(t,n)((t) | ((n) << 8))
#define tag_nrx(t)   ((t) >> 8)
#define tag_type(t)  ((((t) & 0x7F) == 0x7F)? t : (t) & 0x80)
#define tag_capt(t)  ((((t) & 0x7F) == 0x7F)? 0 : (t) & 0x7F)

char *tag_str(uint16_t tag) {
  char* s;

  s=str_chk(10);

  switch (tag_type(tag)) {
    case TAG_MRK : sprintf(s,"MRK_%d", tag_nrx(tag)); break;
    case TAG_FIN : sprintf(s,"FIN_%d", tag_nrx(tag)); break;
    case TAG_CE  : sprintf(s,"CE%d_%d", tag_capt(tag), tag_nrx(tag)); break;
    case TAG_CB  : sprintf(s,"CB%d_%d", tag_capt(tag), tag_nrx(tag)); break;
  }

  return s;
}


/**************************************************/
yrxerr(char *errmsg)
{
  err("ERROR: %s\n%5d: %s\n%*s\n",errmsg,cur_nrx,cur_rx,cur_pos+8,"*");
}
/**************************************************/


#define lbl_clr(b,c) (b[c>>4] &= ~(1<<(c&0xF)))

#define lbl_set(b,c) (b[c>>4] |=  (1<<(c&0xF)))

#define lbl_tst(b,c) ((b[c>>4] & (1<<(c&0xF))))

#define lbl_zro(b) do { uint8_t i; for(i=0; i<16; i++) b[i] = 0; } while (0)
#define lbl_neg(b) do { uint8_t i; for(i=0; i<16; i++) b[i] ^= 0xFFFF; } while (0)

#define minus(a,b)   do { uint8_t i; for(i=0; i<16; i++) a[i] &= ~b[i]; } while (0)
#define lbl_or(a,b)  do { uint8_t i; for(i=0; i<16; i++) a[i] |=  b[i]; } while (0)
#define lbl_and(a,b) do { uint8_t i; for(i=0; i<16; i++) a[i] &=  b[i]; } while (0)

#define LBL_NOTGREEDY 0x0100

int __c;
#define hex(c) (__c=c,('0' <= __c && __c <= '9')? __c - '0' :\
                      ('A' <= __c && __c <= 'F')? __c - 'A'+10:\
                      ('a' <= __c && __c <= 'f')? __c - 'a'+10:-1)\
                      
#define oct(c) (__c=c,('0' <= __c && __c <= '7')? __c - '0' : -1)


uint16_t *lbl_bmp(char *s)
{
  BOOL negate = FALSE;
  BOOL range = FALSE;
  int last;
  int c;
  int i=0;
  int h;
  
  static uint16_t ll[16];
  uint16_t l2[16];
  
  lbl_zro(ll);
  if (s && s[0]) {
    
    if (*s == '[') {
      s++;
      if (*s == '^') {s++; negate = TRUE;}
    }
    
    c = *s;
    while ( c != '\0' && c != ']') {
      if (c == '-')
        range = TRUE;
      else if (c == '.') {
        lbl_zro(ll); lbl_neg(ll); break;
      }
      else {
        if (c == '\\') {
          s++;
          if (*s == '\0') yrxerr("Unexpected \\");
          
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
                      s++;
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
        range = FALSE;
      }
      c = *++s;
    }
    if (range) lbl_set(ll,'-');
    if (negate) lbl_neg(ll);
  }
  return ll;
}


int lbl_rng(uint16_t *bmp, uint16_t a)
{
  char    *s;
  uint16_t b;
  uint16_t i=0;
  
  b=a;
  if ((a & 0x0F) == 0)
    while ((a <= 255) && (bmp[a>>4] == 0)) a += 16;
  while (!lbl_tst(bmp,a)) if (a++ == 256) return -1;
  b=a+1;
  while (b <= 255 && lbl_tst(bmp,b)) b++;
  b=b-1;
  return ((a<<8) | b);
}

/**************************************************/


void addarc(uint16_t from,uint16_t to,char *l,uint16_t tag)
{
  uint16_t *bmp=NULL;
  int a,b;
  
  printf("addarc(%d,%d,%d,\"",from,to,tag);
  
  /*printf("%05d %05d %04X ",from,to,tag);*/
  
  if (l && l[0]) {
    printf("%c",l[0]);
    bmp = lbl_bmp(l+1);
    a=0;
    while (a <= 255) {
      b = lbl_rng(bmp,a);
      if (b<0) break;
      a = b >> 8;
      if (32<= a && a <= 127) printf("%c",a);
      else printf("\\%03d",a);
      a = (b & 0xFF);
      if (32<= a && a <= 127) printf("%c",a);
      else printf("\\%03d",a);
      a++;
    }
  }
  
  printf("\")\n");
}

/**************************************************/

  /* 
       <expr>     ::= <term>+

       <term>     ::= \( <expr> ( '|' <expr> )*\)[\+\-\*\?]? |
                      \\E<escaped> | \\: |
                      \[eNQI][\+\-\*\?]? |
                      <cclass>[\+\-\*\?]?
                      
       <cclass>   ::= \[\e*\] | <escaped>

       <escaped>  ::= \\x\h?\h? | \\\o\o?\o? |
                      \\. | [^\|\*\+\-\?\(\)]
           
  */


uint16_t nextstate()
{
  if (cur_state == 65500)  yrxerr("More than 65500 states!!");
  return ++cur_state ;
}

int peekch(int n) {
  int c;
  int i = 0;
  while (TRUE) {
    c = cur_rx[cur_pos+i];
    if (c == '\0') return -1;
    if (i == n ) return c;
    i++;
  }
}

int nextch() {
  int c;
  c = cur_rx[cur_pos];
  if (c == '\0') return -1;
  cur_pos++;
  return c;
}

char* escaped()
{
   char * l;
   int c;
   int j;

   c = peekch(0);

   if ( c < 0 || c == '*' || c == '+' || c == '?' || c == '-' ||
        c == '(' || c == ')' || c == ']' || c == '[' || c == '|') {
     return NULL;
   }

   j=cur_pos;
   if (c == '\\') {
     c = nextch();
     c = nextch();
     if (c < 0) yrxerr ("Unexpected character");
     if (c == 'x') {
       c = peekch(0);
       if (isxdigit(c)) {
         c = nextch();
         c = peekch(0);
         if (isxdigit(c)) c = nextch();
       }
     }
     else if ('0' <= c && c <= '7') {
       c = peekch(0);
       if ('0' <= c && c <= '7') {
         c = nextch();
         c = peekch(0);
         if ('0' <= c && c <= '7') c = nextch();
       }
     }
   }
   else c = nextch();
   l = str_set(j,cur_pos);
   return l;
}

char *cclass()
{
   char* l;
   int c;
   int j;

   c = peekch(0);
   if (c < 0) return NULL;
   j=cur_pos;
   if (c == '[') {
     c=nextch();
     while (c != ']') {
       c = nextch();
       if ( c == '\\' ) c = (nextch(),0);
       if ( c < 0 ) yrxerr("Unterminated class");
     }
     l = str_set(j,cur_pos);
   }
   else l = escaped();

   return l;
}

uint16_t expr(uint16_t state);
  
uint16_t term(uint16_t state)
{
  uint16_t to,t1,alt,start;
  int c;
  uint8_t ncapt;
  char *l;
  
  c = peekch(0);
    
  if ( c < 0) return 0;

  if ( c == '(') {
    c = nextch();
    ncapt = capt++;
    if (ncapt>120)  yrxerr("Too many captures");
    
    start = nextstate();
    
    alt = nextstate();
    to  = nextstate();
    addarc(alt,to,"",tag_code(TAG_CE|ncapt,cur_nrx));
    

    t1 = expr(start);
    while (t1 > 0) {
      c = nextch();
      if (c != '|') break;
      addarc(t1,alt,"",TAG_NONE);    
      t1 = expr(start);
    }
    addarc(t1,alt,"",TAG_NONE);    
    if (c != ')') yrxerr("Unclosed capture");
    
    switch (peekch(0)) {
       case '?':  addarc(start,alt,"",TAG_NONE);
                  addarc(state,start,"",tag_code(TAG_CB|ncapt,cur_nrx));
                  c = nextch();
                  break;
                  
       case '*':  addarc(alt,start,"",TAG_NONE);
                  addarc(state,alt,"",tag_code(TAG_CB|ncapt,cur_nrx));
                  c = nextch();
                  break;
                  
       case '+':  addarc(alt,start,"",TAG_NONE);
                  addarc(state,start,"",tag_code(TAG_CB|ncapt,cur_nrx));
                  c = nextch();
                  break;

       default :  addarc(state,start,"",tag_code(TAG_CB|ncapt,cur_nrx));
                  break;
    }
    
    return to;
  }
  
  if ( c == '\\') {
    switch (peekch(1)) {
      case 'E' :  c = nextch(); c = nextch();
                  l = escaped();
                  if (l == NULL) yrxerr("Invalid escape sequence");
                  esc = l[1];  /*** BUG Need to properly handle \x and \0 ***/
                  if (esc == '\\') esc = l[2];
                  return state;
                  
      case ':' :  to = nextstate();
                  addarc(state,to,"",tag_code(TAG_MRK,cur_nrx));
                  c = nextch(); c = nextch();
                  return to;

      case 'e' :  c = nextch(); c = nextch(); 
                  c = peekch(0);
                  
                  l=str_chk(16);
                  sprintf(l,"@[^\\x%02X] @\\x%02X",esc,esc);
                  l[8]='\0';
                  if (c == '-') l[0] = '-';
                  
                  t1 = nextstate();
                  to = nextstate();
                  addarc(state,to,l,0);
                  addarc(state,t1,l+9,0);
                  addarc(t1,to,"@.",0);
                  switch (c) {
                    case '-' :  
                    case '*' :  addarc(state,to,"",0);
                    case '+' :  addarc(to,to,l,0);
                                addarc(to,t1,l+9,0);
                                c = nextch();
                                break;
                                
                    case '?' :  addarc(state,to,"",0);
                                c = nextch();
                                break;
                    default  :  break;           
                  }
                  return to;
                                       
      default  :  break;
    }
  }
  
  l = cclass();
  if (l != NULL) {
    to = nextstate();
    c = peekch(0);
    if ( c == '-') l[0] = '-';


    switch (c) {
      case '-' :
      case '*' :  addarc(state,to,"",0);
                  addarc(to,to,l,0);
                  c = nextch();
                  break;

      case '+' :  addarc(state,to,l,0);
                  addarc(to,to,l,0);
                  c = nextch();
                  break;

      case '?' :  addarc(state,to,"",0);
                  c = nextch();
      default  :  addarc(state,to,l,0);
                  break;
    }

    return  to;
  }
  
  return 0;
}

uint16_t expr(uint16_t state)
{
  uint16_t j = state;
  do {
    state = j;
    j = term(state);
  } while ( j > 0 )  ;
  return state;
}


uint16_t parse(char *rx,uint16_t nrx)
{
  uint16_t state;

  cur_rx  = rx;
  cur_pos = 0;
  cur_nrx = nrx;
  esc     = '\\';
  capt    = 1;

  if (arcs_lst == NULL)
    arcs_lst = vecNew(sizeof(Arc)); 
  
  if (state_lst == NULL)
    state_lst = vecNew(sizeof(Arc *)); 
  
  state =  expr(1);

  if (peekch(0) != -1) yrxerr("Unexpected character ");

  addarc(state,0,"",tag_code(TAG_FIN,cur_nrx));

  return state;
}

/*************************************/


void closedown()
{
  if (str != NULL) free(str);
  if (str2 != NULL) free(str2);
  if (arcs_lst != NULL) vecFree(arcs_lst);
  if (state_lst != NULL) vecFree(state_lst);
}

void init()
{
  cur_state = 1;
  atexit(closedown);
}

void dump()
{
  uint32_t i;
  Arc *a;
#if 0  
  for (i=0; i< vecCnt(states); i++) {
    a = *(Arc **)vecGet(states,i);
    while (a) {
      printf("%5d -> %-5d %c %04X %02X %02X\n",i,a->to,a->flg,a->tag,a->lmin,a->lmax);
      a = a->next;
    }
  }
#endif
}

int main(int argc, char **argv)
{

  init();

  /*parse("abc",1);*/

  if (argc > 1) {
    parse(argv[1],1);
  }
  dump();
  exit(0);
}
