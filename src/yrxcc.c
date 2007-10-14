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
#include <ctype.h>

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
  uint32_t     lbl; /* index into a buf buffer */
  uint32_t     tag; /* set of tags of the same rx */
  uint16_t     to;
} Arc;

vec_t graph = NULL;
buf_t lbls = NULL;

/**************************************************/

char     *cur_rx;
int       cur_pos;
uint16_t  cur_nrx;
uint8_t   esc;
uint16_t  capt;

uint16_t  cur_state;

/**************************************************/

char *str =  NULL;
int   str_len = 0;

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


/*************************************/

#define MAXCAPTURES  11

#define TAG_NONE   0
#define TAG_CE(n)  (1 << (n>>1))
#define TAG_CB(n)  (TAG_CE(n) + 1)
#define TAG_MRK    (1<<22)
#define TAG_FIN    (1<<23)


/* xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxx
**   nrx    ||
**          \\__MRK
**           \__FIN
*/

#define tag_code(t,n)((t) | ((n) << 24))
#define tag_nrx(t)   ((t) >> 8)
#define tag_type(t)  ((((t) & 0x7F) == 0x7F)? t : (t) & 0x80)
#define tag_capt(t)  ((((t) & 0x7F) == 0x7F)? 0 : (t) & 0x7F)


/**************************************************/
void yrxerr(char *errmsg)
{
  err(121,"ERROR: %s\n%5d: %s\n%*s\n",errmsg,cur_nrx,cur_rx,cur_pos+8,"*");
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

static int c__;
#define hex(c) (c__=c,('0' <= c__ && c__ <= '9')? c__ - '0' :\
                      ('A' <= c__ && c__ <= 'F')? c__ - 'A'+10:\
                      ('a' <= c__ && c__ <= 'f')? c__ - 'a'+10:-1)\

#define oct(c) (c__=c,('0' <= c__ && c__ <= '7')? c__ - '0' : -1)


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
  uint16_t b;

  b = a;
  if ((a & 0x0F) == 0)
    while ((a <= 255) && (bmp[a>>4] == 0)) a += 16;
  while (!lbl_tst(bmp,a)) if (a++ == 256) return -1;
  b=a+1;
  while (b <= 255 && lbl_tst(bmp,b)) b++;
  b=b-1;
  return ((a<<8) | b);
}

/**************************************************/


void addarc(uint16_t from,uint16_t to,char *l,uint32_t tag)
{
  uint16_t *lbmp=NULL;
  int  a,b;
  Arc  arc;
  vec_t arcs;

  if (tag == TAG_NONE) tag = tag_code(TAG_NONE,cur_nrx);

  arc.to = to;
  arc.tag = tag;
  arc.lbl = 0;
  if (l && l[0]) {
    arc.lbl = bufPos(lbls);
    bufPutc(lbls,l[0]);
    lbmp = lbl_bmp(l+1);
    a=0;
    while (a <= 255) {
      b = lbl_rng(lbmp,a);
      if (b<0) break;
      a = b >> 8;
      bufPutc(lbls,a);
      a = (b & 0xFF);
      bufPutc(lbls,a);
      a++;
    }
    bufPutc(lbls,'\0');
  }
  arcs = vecGetVal(graph, from, vec_t );
  if (arcs == NULL){
    arcs = vecNew(sizeof(Arc));
    vecSet(graph,from,&arcs);
  }

  vecAdd(arcs,&arc);
}

/**************************************************/

uint16_t stkonce(uint16_t val,char op)
{
  static stk_t stack = NULL;
  static bmp_t pushed = NULL;

  if (val == 0) op = 'Z';
  switch (op) {
    case 'Z' : stack  = stkFree(stack);
               pushed = bmpFree(pushed);
               val = 0;
               break;

    case 'O' : if (stkIsEmpty(stack)) val = 0;
               else val = stkTopVal(stack,uint16_t);
               stkPop(stack);
               break;

    case 'H' : if (stack == NULL) {
                 stack  = stkNew(sizeof(uint16_t));
                 pushed = bmpNew();
               }
               if (!bmpTest(pushed,val)) {
                 stkPush(stack,&val);
                 bmpSet(pushed,val);
               }
               break;
  }

  return val;
}

#define pushonce(v) stkonce(v,'H')
#define poponce()  stkonce(1,'O')

vec_t marked;

/* |copyarcs()| will always be called with the two states
   belonging to the same expression. That's why tags can
   be merged simply xor-ing them.
*/
void copyarcs(uint16_t fromst, uint16_t tost, uint32_t tags)
{
  vec_t arcsf, arcst;
  uint32_t k = 0;
  Arc *a;

  tags &= 0x00FFFFFF;

  arcst = vecGetVal(graph,tost,vec_t );
  arcsf = vecGetVal(graph,fromst,vec_t );

  while (k < vecCnt(arcsf)) {
    a = vecGet(arcsf,k++);
    a = vecAdd(arcst,a);
    a-> tag |= tags;
  }
}

void removeeps()
{
  uint16_t from;
  uint32_t k;
  Arc *a,*b;
  vec_t arcs;

  marked = vecNew(sizeof(uint16_t));

  pushonce(1);

  while ((from = poponce()) != 0) {
    vecSet(marked,from,&from);

    arcs = vecGetVal(graph, from, vec_t );
    k = 0;
    if (arcs != NULL)  {
      vecSet(marked,from,&from);
      while (k < vecCnt(arcs)) {
        a = vecGet(arcs,k);
        if (a->to != 0) {
          pushonce(a->to);
          if (a->lbl == 0) {
            if (vecGetVal(marked, a->to, uint16_t) == from)
              yrxerr("Expression contains empty closures");
            vecSet(marked, a->to, &from);
            dbgoff("Copy from %d to %d\n",a->to,from);
            copyarcs(a->to, from, a->tag);
            /* Now delete the arc replacing it with the last one*/
            b = vecGet(arcs, vecCnt(arcs)-1);
           *a = *b; /* Copy arcs */
            vecCnt(arcs)--;
            k--; /* Ensure the copied arc will be processed! */
          }
        }
        k++;
      }
    }
  }

  pushonce(0);
  marked = vecFree(marked);

}

uint32_t intersect(uint32_t &a, uint32_t &b, uint32_t &c)
{

}

void mergearcs()
{
  uint16_t from;
  uint32_t k,i;
  Arc *a,*b;
  vec_t arcs;

  marked  = vecNew(sizeof(uint16_t));

  pushonce(1);

  while ((from = poponce()) != 0) {
    vecSet(marked,from,&from);

    arcs = vecGetVal(graph, from, vec_t );
    k = 0;
    if (arcs != NULL)  {
      vecSet(marked,from,&from);
      while (k < vecCnt(arcs)) {
        a = vecGet(arcs,k);
        k++;
      }
    }
  }

  pushonce(0);
  marked = vecFree(marked);
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
    if (ncapt >= MAXCAPTURES)  yrxerr("Too many captures");

    start = nextstate();

    alt = nextstate();
    to  = nextstate();
    addarc(alt,to,"",tag_code(TAG_CE(ncapt),cur_nrx));

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
                  addarc(state,start,"",tag_code(TAG_CB(ncapt),cur_nrx));
                  c = nextch();
                  break;

       case '*':  addarc(alt,start,"",TAG_NONE);
                  addarc(state,alt,"",tag_code(TAG_CB(ncapt),cur_nrx));
                  c = nextch();
                  break;

       case '+':  addarc(alt,start,"",TAG_NONE);
                  addarc(state,start,"",tag_code(TAG_CB(ncapt),cur_nrx));
                  c = nextch();
                  break;

       default :  addarc(state,start,"",tag_code(TAG_CB(ncapt),cur_nrx));
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
                  addarc(state,to,l,TAG_NONE);
                  addarc(state,t1,l+9,TAG_NONE);
                  addarc(t1,to,"@.",TAG_NONE);
                  switch (c) {
                    case '-' :
                    case '*' :  addarc(state,to,"",TAG_NONE);
                    case '+' :  addarc(to,to,l,TAG_NONE);
                                addarc(to,t1,l+9,TAG_NONE);
                                c = nextch();
                                break;

                    case '?' :  addarc(state,to,"",TAG_NONE);
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
      case '*' :  addarc(state,to,"",TAG_NONE);
                  addarc(to,to,l,0);
                  c = nextch();
                  break;

      case '+' :  addarc(state,to,l,TAG_NONE);
                  addarc(to,to,l,0);
                  c = nextch();
                  break;

      case '?' :  addarc(state,to,"",TAG_NONE);
                  c = nextch();
      default  :  addarc(state,to,l,TAG_NONE);
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
  capt    = 0;

  state =  expr(1);

  if (peekch(0) != -1) yrxerr("Unexpected character ");

  addarc(state,0,"",tag_code(TAG_FIN,cur_nrx));

  return state;
}

/*************************************/

void statescleanup(vec_t *arcs)
{
  vecFree(*arcs);
}

void closedown()
{
  if (str != NULL) free(str);
  vecFreeClean(graph,(vecCleaner)statescleanup);
  bufFree(lbls);
  pushonce(0);
  vecFree(marked);
}

void init()
{
  cur_state = 1;
  graph = vecNew(sizeof(vec_t));
  lbls = bufNew();
  bufPuts(lbls,"");

  atexit(closedown);
}

void dump()
{
  uint32_t i,from;
  Arc *a;
  vec_t arcs;
  char lbl[260];

  pushonce(0);
  pushonce(1);
  while ((from = poponce()) != 0) {
    arcs = vecGetVal(graph, from, vec_t );
    if (arcs != NULL)  {
      for (i = vecCnt(arcs), a = vecGet(arcs,0) ;
           i>0; a = vecNext(arcs),i--) {
        bufSeek(lbls, a->lbl);
        bufGets(lbls, lbl);
        printf("%5d -> %-5d %08X %s\n", from, a->to, a->tag,lbl);
        if (a->to != 0) pushonce(a->to);
      }
    }
  }
  pushonce(0);
}

int main(int argc, char **argv)
{
  int i;

  init();
  for ( i = 1; i < argc; i++) {
    parse(argv[i],i);
  }
  removeeps();
  /*mergearcs();*/
  dump();
  exit(0);
}

