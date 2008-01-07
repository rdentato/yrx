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
#define MAXCAPTURES  32

/**************************************************/
static const char *cur_rx  = NULL;
static int         cur_pos = 0;
static uint16_t    cur_nrx = 0;

static uint8_t     esc;
static uint16_t    capt;

/**************************************************/

void yrxParseErr(int errn, char *errmsg)
{
  err(errn,"%s\n%5d: %s\n%*s\n", errmsg,
                                cur_nrx, cur_rx, cur_pos+8,"*");
}

/**************************************************/

static state_t nextstate(void);
static uint16_t nstates = 1;

static state_t nextstate(void)
{
  if (nstates == 65500)
    yrxParseErr(502,"More than 65500 states!!");
  return ++nstates ;
}

/**************************************************/
/* = Parsing expressions
**  Parsing is implemented as a Recursive Descent Parser
** for the following grammar:

       <expr>     ::= <term>+

       <term>     ::=   \( <expr> ( '|' <expr> )* \) [\+\-\*\?]?
                      | \\E <escaped>
                      | \\:
                      | \[eNQI] [\+\-\*\?]?
                      | <cclass> [\+\-\*\?]?

       <cclass>   ::=   \[ \e* \]
                      | <escaped>

       <escaped>  ::=   \\x\h?\h?
                      | \\\o\o?\o?
                      | \\.
                      | [^\|\*\+\-\?\(\)]

 */


static char *str_set(int i,int j)
{
  int n = 0;

  yrxBufChk(j-i+2);
  if (yrxBuf != NULL) {
    while(i < j) {
      yrxBuf[n++] = cur_rx[i++];
    }
    yrxBuf[n] = '\0';
  }
  return (char *)yrxBuf;
}

static int peekch(int n) {
  int c;
  int i = 0;
  for(EVER) {
    c = cur_rx[cur_pos+i];
    if (c == '\0') return -1;
    if (i == n ) return c;
    i++;
  }
  return 0;
}

static int nextch(void) {
  int c;
  c = cur_rx[cur_pos];
  if (c == '\0') return -1;
  cur_pos++;
  return c;
}

static char* escaped(void)
{
  char * l;
  int    c;
  int    j;

  c = peekch(0);

  if ( c < 0 || c == '*' || c == '+' || c == '?' || c == '-' ||
       c == '(' || c == ')' || c == ']' || c == '[' || c == '|') {
    return NULL;
  }

  j = cur_pos;
  if (c == '\\') {
    c = nextch();
    c = nextch();
    if (c < 0) yrxParseErr (503,"Unexpected character (\\)");
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

static char *cclass(void)
{
  char* l;
  int c;
  int j;

  c = peekch(0);
  if (c < 0) return NULL;
  j = cur_pos;
  if (c == '[') {
    c = nextch();
    while (c != ']') {
      c = nextch();
      if (c == '\\') c = (nextch(), 0);
      if (c < 0) yrxParseErr(504,"Unterminated class");
    }
    l = str_set(j, cur_pos);
  }
  else l = escaped();

  return l;
}

static state_t expr(state_t state);

static state_t term(state_t state)
{
  state_t to,t1,alt,start;
  int c;
  uint16_t ncapt;
  char *l;
  lbl_t l1,l2;

  c = peekch(0);

  if ( c < 0) return 0;

  if ( c == '(') {
    c = nextch();
    ncapt = capt++;
    if (ncapt >= MAXCAPTURES) yrxParseErr(505,"Too many captures");

    start = nextstate();

    alt = nextstate();
    to  = nextstate();
    yrxNFAAddarc(alt,to,yrxLblEps,yrxTag(TAG_CE(ncapt),cur_nrx,0));

    t1 = expr(start);
    while (t1 > 0) {
      c = nextch();
      if (c != '|') break;
      yrxNFAAddarc(t1,alt,yrxLblEps,yrxTagNone);
      t1 = expr(start);
    }
    if (c != ')') yrxParseErr(506,"Unclosed capture");
    yrxNFAAddarc(t1,alt,yrxLblEps,yrxTagNone);

    /* state -> (1)
    ** start -> (2)
    ** alt   -> (3)
    ** to    -> (4)
    ** t1    -> (5)
    */
    
    switch (peekch(0)) {
       case '?':  yrxNFAAddarc(start, alt, yrxLblEps, yrxTagNone);
                  yrxNFAAddarc(state, start, yrxLblEps, yrxTag(TAG_CB(ncapt), cur_nrx,0));
                  c = nextch();
                  break;

       case '+':  yrxNFAAddarc(alt, start, yrxLblEps, yrxTagNone);
                  yrxNFAAddarc(state, start, yrxLblEps, yrxTag(TAG_CB(ncapt),cur_nrx,0));
                  c = nextch();
                  break;

       case '*':  yrxNFAAddarc(alt, start, yrxLblEps, yrxTagNone);
                  yrxNFAAddarc(state, alt, yrxLblEps, yrxTag(TAG_CB(ncapt), cur_nrx,0));
                  c = nextch();
                  break;

       default :  yrxNFAAddarc(state, start, yrxLblEps, yrxTag(TAG_CB(ncapt),cur_nrx,0));
                  break;
    }

    return to;
  }

  if ( c == '\\') {
    switch (peekch(1)) {
      case 'E' :  c = nextch(); c = nextch();
                  l = escaped();
                  if (l == NULL) yrxParseErr(507,"Invalid escape sequence");
                  esc = l[1];  /*** TODO: Need to properly handle \x and \0 ***/
                  if (esc == '\\') esc = l[2];
                  return state;

      case ':' :  to = nextstate();
                  yrxNFAAddarc(state, to, yrxLblEps, yrxTag(TAG_MRK,cur_nrx,0));
                  c = nextch(); c = nextch();
                  return to;

      case 'e' :  c = nextch(); c = nextch();
                  c = peekch(0);

                  l = (char *)yrxBufChk(16);
                  
                  sprintf(l,"[^\\x%02X]%c\\x%02X",esc,0,esc);
                  l1 = yrxLabel(l);
                  l2 = yrxLabel(l+8);
                  
                  t1 = nextstate();
                  to = nextstate();
                  yrxNFAAddarc(state, to, l1, yrxTagNone);
                  yrxNFAAddarc(state, t1, l2, yrxTagNone);
                  yrxNFAAddarc(t1, to, yrxLabel("."), yrxTagNone);
                  switch (c) {
                    case '*' :  yrxNFAAddarc(state, to, yrxLblEps, yrxTagNone);
                    case '+' :  yrxNFAAddarc(to, to, l1, yrxTagNone);
                                yrxNFAAddarc(to, t1, l2, yrxTagNone);
                                c = nextch();
                                break;

                    case '?' :  yrxNFAAddarc(state, to, yrxLblEps, yrxTagNone);
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
    
    l1 = yrxLabel(l);
   
    switch (c) {
      case '*' :  yrxNFAAddarc(state, to, yrxLblEps, yrxTagNone);
                  yrxNFAAddarc(to, to, l1, yrxTagNone);
                  c = nextch();
                  break;

      case '+' :  yrxNFAAddarc(state, to, l1, yrxTagNone);
                  yrxNFAAddarc(to, to, l1, yrxTagNone);
                  c = nextch();
                  break;

      case '?' :  yrxNFAAddarc(state, to, yrxLblEps, yrxTagNone);
                  c = nextch();
      default  :  yrxNFAAddarc(state, to, l1, yrxTagNone);
                  break;
    }
    return  to;
  }
  return 0;
}

static state_t expr(state_t state)
{
  state_t j = state;
  do {
    state = j;
    j = term(state);
  } while ( j > 0 )  ;
  return state;
}

static state_t parse(const char *rx,uint16_t nrx)
{
  state_t state;

  cur_rx  = rx;
  cur_pos = 0;
  cur_nrx = nrx;
  esc     = '\\';
  capt    = 0;

  state =  expr(1);

  if (peekch(0) != -1) yrxParseErr(508,"Unexpected character ");

  yrxNFAAddarc(state, 0, yrxLblEps, yrxTag(TAG_FIN, cur_nrx,0));

  return state;
}

void yrxParse(char **rxs, int rxn)
{
  int i;
 
  if (rxn < 1 || 250 < rxn)
    yrxParseErr(501,"Invalid number of argument");
    
  for (i = 0; i < rxn; i++) {
    parse(rxs[i],i+1);
  }
  
  yrxDFA();
  
}


