/*  YRX - rx_exec.c
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
** This code is a derivative work of the PUBLIC DOMAIN regex routines
** by Ozan Yigit: http://www.cs.yorku.ca/~oz/
**
*/

#include "rx_.h"

static unsigned char *match(rx_extended *r,unsigned char *str, const unsigned char *nfa);

int rx_isa(unsigned char c, unsigned char what)
{ 
  /*   The use of is...() functions ensure that the current
  ** locale will be used.
  */
  
  switch (what) {
    case ALPHA  :  return(isalpha(c));
    case ANYUPR :  return(isupper(c));
    case ANYLWR :  return(islower(c));
    case ALNUM  :  return(isalnum(c));
    case DIGIT  :  return(isdigit(c));
    case XDIGIT :  return(isxdigit(c));
    case SPACE  :  return(isspace(c));
    case PUNCT  :  return(ispunct(c));
    case WORDC  :  return(isalnum(c) || (c == '_'));
    case ANY    :  return(c != '\0');
    case CTRL   :  return(c && (c <= 0x07));
    case SPCTAB :  return(c == ' ' || c == '\t');
    case ZERO   :  return(c == '\0');
  }
  return 0;
}

int rx_isinccl_class(unsigned char n, const unsigned char *cb)
{
  register unsigned char k=cb[1];
  
  if (k != 0x80) {
    if ((k & 0x01) && rx_isa(n,ANYUPR)) return 1;
    if ((k & 0x02) && rx_isa(n,ANYLWR)) return 1;
    if ((k & 0x04) && rx_isa(n,ALNUM )) return 1;
    if ((k & 0x08) && rx_isa(n,DIGIT )) return 1;
    if ((k & 0x10) && rx_isa(n,XDIGIT)) return 1;
    if ((k & 0x20) && rx_isa(n,SPACE )) return 1;
    if ((k & 0x40) && rx_isa(n,PUNCT )) return 1;
  }
  return 0;
}

int rx_isinccl(unsigned char n, const unsigned char *ccl)
{
  unsigned short b = 0;
  unsigned short found = 0x40;
  const unsigned char *cb = ccl+2;
  unsigned char  k = (ccl[0] & 0x3F);
  
  if (*ccl == NOTCHR) {
    if (n && n != ccl[1]) found = 0; 
  } else if (*ccl == NOTCLS) {
    if (n && !rx_isa(n,ccl[1])) found = 0;
  } else if (!rx_isinccl_class(n,ccl)) { 
    while (k-- > 0) {
      b += (*cb & 0x80) ? (*cb & 0x3F) * 7:  7;
      if (b > n) break;
      cb++;
    }
    if (b <= n) found = 0;
    else if ((*cb & 0x80) == 0x00) {
      if ((*cb & (1 << (n % 7))) == 0x00) 
        found = 0;
    }
    else if ((*cb & 0x40) == 0x00) 
      found = 0;
  }
  return (found ^ (ccl[0] & 0x40)) ;
}

static char *qstr(unsigned char *e)
{
  char c;
  c=*e++;
  if (c == '\'' || c == '"') {
    while (1) {
      if (*e == '\0') break;
      if (*e == c) {
        if (*(e+1) == c) e+=2;
        else break;
      }
      if ((*e == '\\') && (*(e+1) != '\0')) e++;
      e++;
    }
  }
  else
    return NULL;
    
  if (*e != c) return NULL;
  return (e+1);
}

static int skip(const unsigned char *nfa) 
{
  switch (optype(*nfa)) {
    case STR    : return (STR_len(*nfa)+1);
    case CCL    : return (CCL_len(*nfa)+2);
  } 
  return 1;
}

rx_result rx_exec(const unsigned char *nfa, unsigned char *str)
{
  unsigned char *matches=NULL;
  
  static rx_extended rex; 
  
  if (str == NULL) return NULL;
  
  rex.goal = NULL;
  rex.casesensitive = 1;
  rex.bol = str;
  rex.rxnum = 0;
    
  do {
    rex.rxnum = 0;
    if ((matches = match(&rex,str,nfa)) || rex.failall) break;
  } while (*str++);

  if (matches) {
    rex.boc[0] = str;
    rex.eoc[0] = rex.goal? rex.goal: matches;
    return ((rx_result *)&rex);
  }
  else
    return NULL;
}

#define FAILED() {fail = 1; break;}

#define MAX_OFSTACK 128

static unsigned char *of_stack[MAX_OFSTACK];
static unsigned char  of_loop[MAX_OFSTACK/2];
static unsigned char  of_cnt = 0;

#define of_pop_()   (of_cnt > 0? of_stack[--of_cnt] : 0)
#define of_pop(n,s) (s=of_pop_(),n=of_pop_())

#define of_empty()  (of_cnt == 0)
#define of_reset()  (of_cnt = 0)

#define of_push(n,s) (of_cnt < MAX_OFSTACK+1 ? \
                        (of_loop[of_cnt / 2] = 0, \
                         of_stack[of_cnt++] = (unsigned char *)(n), \
                         of_stack[of_cnt++] = (unsigned char *)(s)) : 0)   

#define of_inc_loop()  (of_cnt > 0? ++of_loop[(of_cnt>>1)-1] : 0)
#define of_num_loop()  (of_cnt > 0? of_loop[(of_cnt>>1)-1] : 254)

static unsigned char *match(rx_extended *r, unsigned char *str, const unsigned char *nfa)
{
  unsigned char n, c;
  unsigned short min, max, k;
  register unsigned char *s = str;
  unsigned char *start, *t, *p;
  unsigned fail = 0;
  int back = 0;

  while (*nfa != END) {
    /*fprintf(stderr,"--> %p %02x %02X\n",nfa,*nfa,optype(*nfa));fflush(stderr);*/ 
    start = s;
    switch (optype(*nfa)) {
    
      case GOTO  : if ((*nfa & 0xF0) == ONFAIL) {
                     of_push(nfa+1+jmparg(nfa), start);
                   }
                   else {
                     k = jmparg(nfa);
                     /* fprintf(stderr,">>> %d %d %p ->",k,back,nfa);*/
                     nfa = nfa +  (back ? -k : k);
                     /* fprintf(stderr," %p\n",nfa); fflush(stderr); */
                   }
                   back = 0;
                   nfa++;
                   break;
      
      case SINGLE : if (iscls(*nfa)) {                     
                      if (*s == '\0' || !rx_isa(*s++,*nfa)) FAILED();
                    }
                    else {  
                      switch (*nfa) {
                        case BOL:  if (s != r->bol) FAILED(); break;
                        case EOL:  if (*s)          FAILED(); break;

                        case CASE: r->casesensitive ^= 1; break;
                        
                        case QSTR: if ((s=qstr(s)) == NULL ) FAILED();
                                    break;
                                    
                        case NINT: if ((*s == '+' || *s == '-') && isdigit(s[1])) s+=2;
                                   while (isdigit(*s)) s++;
                                   if (s == start) FAILED();
                                   break;
                            
                        case ESCAPE: r->escape = *s;
                                     if (r->escape == ' ') r->escape = '\0';
                                     break;
                                   
                        case SPCS: while (isspace(*s)) s++;
                                   break;

                        case BRACED: k=0; c = *++nfa; n = *++nfa;
                                    if (*s != c) FAILED();
                                    k++;
                                    while (*++s && k > 0) {
                                      if (*s == c) k++;
                                      else if (*s == n) k--;
                                    }
                                    if (k) FAILED();
                                    
                                    break;
                                                                      
                        case IDENT: while (rx_isa(*s,WORDC)) s++;
                                    if (s == start) FAILED();
                                    break;
                                   
                        case NHEX: if ((*s == '0') &&
                                       (s[1] == 'x' || s[1] == 'X') &&
                                        isxdigit(s[2]))
                                      s += 3;
                                   while (isxdigit(*s)) s++;
                                   if (s == start) FAILED();
                                   break;
                           
                        case NFLOAT: 
                                   if ((*s == '+' || *s == '-') && isdigit(s[1])) s+=2;
                                   while (isdigit(*s)) s++;
                                   if (*s == '.') { 
                                     s++;
                                     while (isdigit(*s)) s++;
                                   }
                                   if (s == start) FAILED();
                                   break;
                           
                         case REPT:  min = *++nfa;
                                     max = *++nfa;
                                     if (min == 255) min = 0;
                                     if (max == 255) max = 65535;
                                     goto clo;
                         case OPT:   min = 0; max = 1    ; goto clo;
                         case REPT0: min = 0; max = 65535; goto clo;
                         case REPT1: min = 1; max = 65535; goto clo;
                                clo:
                                   k = 0;
                                   t = s;
                                   n = optype(nfa[1]);
                                   c = *s; 
                                   while (c && k<max) {
                                     if (n == STR) {
                                       if (r->casesensitive == 0) c=tolower(c);
                                       if (c != nfa[2])  break;
                                     }
                                     else if (n == SINGLE) {
                                       if (!rx_isa(c,nfa[1])) break;
                                     }
                                     else if (n == CCL) {
                                       if (!rx_isinccl(c,nfa+1)) break;
                                     }
                                     else FAILED();
                                     c=s[++k]; 
                                   }
                                   s += k;
                                   n=*nfa;
                                   nfa += skip(nfa+1);
                                   #if 0
                                   while (k>=min && s >=t) {
                                     /* TODO: REMOVE RECURSION TO OPTIMIZE */
                                     if ((p=match(r,s,nfa+1))) return p;
                                     s--; k--;
                                   }        
                                   #else
                                     if (min <= k && k <= max) break;
                                   #endif                           
                                   FAILED();
                                   break;
                                   
                         case EMPTY : break;
                         
                         case BKMAX:  if (of_inc_loop() < *++nfa)
                                        back = 1;  /* enable back goto */
                                      else
                                        nfa += 2; /* skip GOTO */
                                      /*fprintf(stderr,"<< %d\n",of_num_loop());*/
                                       break;
                         case MIN   :  if (of_num_loop() >= *++nfa) break;
                                       FAILED();
                                       break; 
                         
                         case MATCH  : return s;
                                   
                         case FAILALL: r->failall = 1;
                         case FAIL   : FAILED();
                                       break;
                                       
                         case PEEKED : of_pop(t,s);
                                       break;
                                       
                         case ONFEND : of_pop(t,t);
                                       break;
                                       
                         case PATTERN: r->rxnum++;
                                       r->failall = 0;
                                       k = (nfa[1] & 0x7F) << 7 | (nfa[2] & 0x7F);
                                       /*fprintf(stderr,"RX: %d %d\n",r->rxnum, k);*/
                                       of_reset();
                                       of_push(nfa+1+k,  start);
                                       for (k=0; k<=RX_MAXCAPT; k++)
                                         r->boc[k] = r->eoc[k] = NULL;
                                       nfa += 2;
                                       break;
                   
                      }
                    }
                    break;
      
      case STR    : for (n= STR_len(*nfa); *s && (n > 0); n--) {
                      c=*s++;
                      if (r->casesensitive == 0) c = tolower(c);
                      if (*++nfa != c) FAILED();
                    }
                    if (n>0) FAILED();
                    break;
                    
      case CAPTR  : n=CAPT_num(*nfa);
                    switch (CAPT_type(*nfa)) {
                      case BOC:  r->boc[n] = s; /* Ensure capture is empty */
                      case EOC:  r->eoc[n] = s; break;
                      case CAPT: t = r->boc[n];
                                 while (*s && t < r->eoc[n]) {
                                   if (*s++ != *t++) FAILED();
                                 }
                                 break;
                    }
                    break;
                    
      case CCL    : if (*s == '\0' || !rx_isinccl(*s++, nfa)) FAILED();
                    nfa += CCL_len(*nfa) +1;
                    break;
                    
      default     : FAILED();
    }
    
    if (fail) {
      if (of_empty()) return NULL;
      of_pop(nfa,s);
      fail = 0;
    }
    nfa++;
  }

  return s;
}

#define rx_boc ((rx_extended *)rx)->boc
#define rx_eoc ((rx_extended *)rx)->eoc

int rx_iterate(const unsigned char *nfa, unsigned char *str, int (* f)(rx_result))
{
  unsigned char *s;
  int cnt = 0;
  int n;
  rx_result rx;
  s = str;
  if (s != NULL) {
    while (*s) {
      rx = rx_exec(nfa,s);
      /*fprintf(stderr,"*** %p %d %d %s\n",s,cnt,rx_len(rx,0),s); */
      if (rx == NULL || (n = rx_len(rx,0)) == 0 || f(rx)) break;
      s = rx_eoc[0]; cnt++;
    }
  }
  return cnt;
}

int rx_len(rx_result rx, unsigned char n)
{
  if ((rx == NULL) || (n > 8) || rx_boc[n] == NULL) return 0;
  return rx_eoc[n] - rx_boc[n];
}

char *rx_start(rx_result rx, unsigned char n)
{
  if ((rx == NULL) || (n > 8)) return NULL;
  return rx_boc[n];
}

char *rx_end(rx_result rx, unsigned char n)
{
  if ((rx == NULL) || (n > 8)) return NULL;
  return rx_eoc[n];
}

unsigned char rx_matched(rx_result rx)
{
  if (rx == NULL) return 0;
  return ((rx_extended *)rx)->rxnum;
}

char *rx_cpy(rx_result rx, unsigned char n, unsigned char *s)
{
  if ((rx == NULL) || (n > 8) || rx_boc[n] == NULL) return NULL;
  strncpy(s,rx_boc[n],rx_eoc[n] - rx_boc[n]);
  s[rx_eoc[n] - rx_boc[n]] = '\0';
  return s;
}

rx_result rx_match(const unsigned char *pattern, unsigned char *str, char **err)
{
   char *e;
   static unsigned char nfa[RX_MAXNFA];
   
   e = rx_compile(pattern,nfa);
   if (err) *err = e;
   if (e) return NULL;
   
   return rx_exec(nfa,str);
}

