

#ifdef UTEST
#include "ut.h"

static char nfa[RX_MAXNFA];
static char *err;
static char *res;
static rx_result rxs;
static char *str;

static char *match_str(unsigned char *r,
                       unsigned char *s1, unsigned char a, unsigned char b,
                       unsigned char *s2)
{
  res = rx_compile(r,nfa);
  if (res == NULL) {
    rxs = rx_exec(nfa,s1);
    UTUnsignedEqual(rx_len(rxs,0),a);
    UTPointersEqual(rx_start(rxs,0),s1+b);
    rxs = rx_exec(nfa,s2);
    UTPointersEqual(rxs,NULL);
    UTPointersEqual(rx_start(rxs,0),NULL);
    UTSignedEqual(rx_len(rxs,0),-1);
  }
  return res;
}
  
TESTCASE(match_string,"Match a plain string")
  res=match_str("abcd","123abcd456",4,3,"123456");
TESTRESULT(res)

TESTCASE(match_anchor_start,"Match a plain string (anchored at the beginning of line)")
  res=match_str("^abcd","abcd456",4,0,"123abcd456");
TESTRESULT(res)

TESTCASE(match_anchor_end,"Match a plain string (anchored at the end of line)")
  res=match_str("abcd$","123abcd",4,3,"123abcd456");
TESTRESULT(res)
  
TESTCASE(match_empty_str,"Match empty string")
  res=match_str("^$","",0,0,"abcd");
TESTRESULT(res)

TESTCASE(match_anchor_char,"Match non anchoring '^' and '$'")
  res=match_str("\\^ac\\$","0^ac$1",4,1,"abcd");
  res=match_str("a^b$c","0a^b$c1",5,1,"abcd");
TESTRESULT(res)
  
TESTCASE(match_rept,"Match repetition '*', '+', '?'")
  res=match_str("ab*c","0abbbc1",5,1,"0axcb1");
  res=match_str("ab*c","0ac1",2,1,"0axcb1");
  res=match_str("ab?c","0abc1",3,1,"0axcb1");
  res=match_str("ab?cde","0acde1",4,1,"0axcb1");
  res=match_str("ab+c","0abbbbc1",6,1,"0acb1");
  res=match_str("ab?c","0abc1",3,1,"0axcb1");
  res=match_str("ab?c","0ac1",2,1,"0axcb1");
  res=match_str("ab+c","0abbbc1",5,1,"0acb1");
TESTRESULT(res)

TESTCASE(match_rept2,"Match repetition '{}'")
  res=match_str("ab{0,}c","0abbbc1",5,1,"0axcb1");
  res=match_str("ab{}c","0ac1",2,1,"0axcb1");
  res=match_str("ab{0,1}c","0abc1",3,1,"0axcb1");
  res=match_str("ab{,1}cde","0acde1",4,1,"0axcb1");
  res=match_str("ab{1,}c","0abbbbc1",6,1,"0acb1");
  res=match_str("ab{0,1}c","0abc1",3,1,"0axcb1");
  res=match_str("ab{0,1}c","0ac1",2,1,"0axcb1");
  res=match_str("ab{1,}c","0abbbc1",5,1,"0acb1");
TESTRESULT(res)

#endif

