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

#define UID 500


/**************************************************/
static const char *cur_rx  = NULL;
static int         cur_pos = 0;
static uint16_t    cur_nrx = 0;

static uint8_t     esc;
static uint16_t    capt;

/**************************************************/

static ucv_t buf = NULL;

static ucv_t buf_chk(int n)
{
  if (buf == NULL)
    buf = ucvNew();
  if (ucvSlt(buf) < n)
    buf = ucvSet(buf,n,'\0');
  return buf;
}

/**************************************************/

static void parse_err(int errn, char *errmsg)
{
  err(UID + errn,"%s\n%5d: %s\n%*s\n", errmsg,
                                cur_nrx, cur_rx, cur_pos+8,"*");
}

/**************************************************/

static state_t nextstate(void);
static uint16_t nstates = 1;

static state_t nextstate(void)
{
  if (nstates == 65500)
    parse_err(2,"More than 65500 states!!");
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

  buf = buf_chk(j-i+2);
  if (buf != NULL) {
    buf[n++] = '@';
    while(i < j) {
      buf[n++] = cur_rx[i++];
    }
    buf[n] = '\0';
  }
  return (char *)buf;
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
    if (c < 0) parse_err (3,"Unexpected character (\\)");
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
      if (c < 0) parse_err(4,"Unterminated class");
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

  c = peekch(0);

  if ( c < 0) return 0;

  if ( c == '(') {
    c = nextch();
    ncapt = capt++;
    if (ncapt >= MAXCAPTURES) parse_err(5,"Too many captures");

    start = nextstate();

    alt = nextstate();
    to  = nextstate();
    yrxNFAAddarc(alt,to,"",tag_code(TAG_CE(ncapt),cur_nrx,0));

    t1 = expr(start);
    while (t1 > 0) {
      c = nextch();
      if (c != '|') break;
      yrxNFAAddarc(t1,alt,"",TAG_NONE);
      t1 = expr(start);
    }
    if (c != ')') parse_err(6,"Unclosed capture");
    yrxNFAAddarc(t1,alt,"",TAG_NONE);

    /* state -> (1)
    ** start -> (2)
    ** alt   -> (3)
    ** to    -> (4)
    ** t1    -> (5)
    */
    
    switch (peekch(0)) {
       case '?':  yrxNFAAddarc(start, alt, "", TAG_NONE);
                  yrxNFAAddarc(state, start, "", tag_code(TAG_CB(ncapt), cur_nrx,0));
                  c = nextch();
                  break;

       case '+':  yrxNFAAddarc(alt, start, "", TAG_NONE);
                  yrxNFAAddarc(state, start, "", tag_code(TAG_CB(ncapt),cur_nrx,0));
                  c = nextch();
                  break;

       case '*':  yrxNFAAddarc(alt, start, "", TAG_NONE);
                  yrxNFAAddarc(state, alt, "", tag_code(TAG_CB(ncapt), cur_nrx,0));
                  c = nextch();
                  break;

       default :  yrxNFAAddarc(state, start, "", tag_code(TAG_CB(ncapt),cur_nrx,0));
                  break;
    }

    return to;
  }

  if ( c == '\\') {
    switch (peekch(1)) {
      case 'E' :  c = nextch(); c = nextch();
                  l = escaped();
                  if (l == NULL) parse_err(7,"Invalid escape sequence");
                  esc = l[1];  /*** TODO: Need to properly handle \x and \0 ***/
                  if (esc == '\\') esc = l[2];
                  return state;

      case ':' :  to = nextstate();
                  yrxNFAAddarc(state, to, "", tag_code(TAG_MRK,cur_nrx,0));
                  c = nextch(); c = nextch();
                  return to;

      case 'e' :  c = nextch(); c = nextch();
                  c = peekch(0);

                  l = (char *)buf_chk(16);
                  sprintf(l,"@[^\\x%02X] @\\x%02X",esc,esc);
                  l[8]='\0';
                  if (c == '-') l[0] = '-';

                  t1 = nextstate();
                  to = nextstate();
                  yrxNFAAddarc(state, to, l, TAG_NONE);
                  yrxNFAAddarc(state, t1, l+9, TAG_NONE);
                  yrxNFAAddarc(t1, to, "@.", TAG_NONE);
                  switch (c) {
                    case '-' :
                    case '*' :  yrxNFAAddarc(state, to, "", TAG_NONE);
                    case '+' :  yrxNFAAddarc(to, to, l, TAG_NONE);
                                yrxNFAAddarc(to, t1, l+9, TAG_NONE);
                                c = nextch();
                                break;

                    case '?' :  yrxNFAAddarc(state, to, "", TAG_NONE);
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
      case '*' :  yrxNFAAddarc(state, to, "", TAG_NONE);
                  yrxNFAAddarc(state, state, l, TAG_NONE);
                  c = nextch();
                  break;

      case '+' :  yrxNFAAddarc(state, to, l, TAG_NONE);
                  yrxNFAAddarc(to, to, l, TAG_NONE);
                  c = nextch();
                  break;

      case '?' :  yrxNFAAddarc(state, to, "", TAG_NONE);
                  c = nextch();
      default  :  yrxNFAAddarc(state, to, l, TAG_NONE);
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

  if (peekch(0) != -1) parse_err(8,"Unexpected character ");

  yrxNFAAddarc(state, 0, "", tag_code(TAG_FIN, cur_nrx,0));

  return state;
}

void yrxParse(char **rxs, int rxn)
{
  int i;

  if (rxn < 1 || 250 < rxn)
    parse_err(1,"Invalid number of argument");
    
  yrxNFAInit();

  for (i = 0; i < rxn; i++) {
    parse(rxs[i],i+1);
  }
  
  yrxNFAClose();
  
  buf = ucvFree(buf);
}


