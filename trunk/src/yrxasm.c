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

/*****/

#define MTC 0x02  /* 0 00000 10  MaTCh                         */
#define MRK 0x03  /* 0 00000 11  MaRK                          */

#define CPB 0x06  /* 0 00001 10  Capture Begin                 */
#define CPE 0x07  /* 0 00001 11  Capture End                   */

#define NRX 0x21  /* 00 1000 01  Number of Regular eXpressions */
#define CMP 0x25  /* 00 1001 01  Compare                       */
#define DIM 0x29  /* 00 1010 01                                */
#define NCP 0x2D  /* 00 1011 01  Number of Captures            */
#define GET 0x31  /* 00 1100 01  GET next input char           */
#define NOP 0x35  /* 00 1101 01  No OPeration                  */

#define RET 0x04  /* 00 0001 00  RETurn                        */
#define REQ 0x08  /* 00 0010 00  Return on EQual               */
#define RGE 0x0C  /* 00 0011 00  Return on Greater or Equal    */
#define RGT 0x10  /* 00 0100 00  Return on Greater Than        */
#define RLE 0x14  /* 00 0101 00  Return on Less or Equal       */
#define RLT 0x18  /* 00 0110 00  Return on Less Than           */
#define RNE 0x1C  /* 00 0111 00  Return on Not Equal           */

#define JMP 0x24  /* 00 1001 00  JuMP                          */
#define JEQ 0x28  /* 00 1010 00  Jump on EQual                 */
#define JGE 0x2C  /* 00 1011 00  Jump on Less Than             */
#define JGT 0x30  /* 00 1100 00  Jump on Greater or Equal      */
#define JLE 0x34  /* 00 1101 00  Jump on Not Equal             */
#define JLT 0x38  /* 00 1110 00  Jump on Less or Equal         */
#define JNE 0x3C  /* 00 1111 00  Jump on Greater Than          */

#define isjmp(x) (((x) & 0x23) == 0x20)
#define istag(x) (((x) & 0x03) >  0x01)

uint8_t nargs(uint8_t op)
{
   switch (op & 0x03) {
     case 0x00: return ((op & 0x20) ? 1 : 0);
     case 0x01: if (op & 0x20) return ((op < GET)? 1 : 0);
     default  : /* includes MTC and MRK! */
                return 2;
   } 
}

typedef struct {
  uint8_t  op;
  uint8_t  arg1;
  uint16_t arg2;
  uint32_t targ;
} step_t;

static vec_t pgm; 

#define opcode(a) op[a] = #a 
static char *op[0x40];

void yrxASMInit(void)
{
  opcode(CPB);
  opcode(CPE);
  opcode(MTC);
  opcode(MRK);
  opcode(CMP);
  opcode(GET);
  opcode(DIM);
  opcode(NCP);
  opcode(NRX);
  opcode(NOP);
  opcode(REQ);
  opcode(RGE);
  opcode(RGT);
  opcode(RLE);
  opcode(RLT);
  opcode(RNE);
  opcode(RET);
  opcode(JMP);
  opcode(JEQ);
  opcode(JGE);
  opcode(JGT);
  opcode(JLE);
  opcode(JLT);
  opcode(JNE);
  
  pgm = vecNew(sizeof(step_t));
  if (pgm == NULL) err(934,yrxStrNoMem);
}

void yrxASMClean(void)
{
  if (pgm != NULL) vecFree(pgm); 
}

typedef struct {
  uint16_t arc;
  uint8_t  min;
  uint8_t  max;
} rng2arc;


static uint8_t *dmp_asmchr(uint8_t c)
{
  static char buf[8];
  
  if (c <= 32   || c > 126  || c == '\\' || c == '"' ||
      c == '\'') {
    sprintf(buf,"%02X",c);
  }
  else {
    buf[0] = '\''; buf[1] = c;
    buf[2] = '\''; buf[3] = '\0';
  }
  
  return buf;
}

static void dumpasm(uint32_t step)
{
  uint8_t opcode;
  uint32_t arg;
  uint8_t capnum;
  
  opcode  = step & 0xFF;
  arg     = step >> 8;
  
  if (opcode == CMP) {
    printf("\t%s %s\n",op[opcode],dmp_asmchr(arg));
  }
  else if (isjmp(opcode)) {
    switch(opcode & 0xC0) {
      case 0x40: capnum = 'A'; break;
      case 0x80: capnum = 'L'; break;
      case 0xC0: capnum = 'R'; break;
      default  : capnum = 'S'; break;
    }
    printf("\t%s %c%u\n",op[opcode & ~0xC0],capnum,arg);
  }
  else if (istag(opcode)) {
    capnum = (opcode & 0x7C) >> 2;
    opcode = (opcode & 0x03);
    
    if (capnum > 0) opcode |= 0x04;
    
    if (capnum == 0) {
      printf("\t%s %u",op[opcode & 0x03],arg & 0xFF);
    }
    else {
      char c = 'X';
      switch (opcode) {
        case CPB        : c ='('; break;
        case CPE        : c =')'; break;
      }
      printf("\t%c%02u %u",c,capnum,arg & 0xFF);
    }
    arg = arg >> 8;
    if (arg != 0) {
      printf(",%u",arg);
    }
    printf("\n");
  }
  else {
    switch (nargs(opcode)) {
      case 0: printf("\t%s\n",op[opcode]); break;
      case 1: printf("\t%s %X\n",op[opcode],arg); break;
      case 2: printf("\t%s %X\n",op[opcode],arg); break;
    }
  }  
}

static void addop(uint8_t opcode, uint32_t arg)
{
  uint32_t step;
  
  step = (arg << 8) | opcode ;
  dumpasm(step);
  
  /* 
  pgm = ulvAdd(pgm,step);
  */
}

static void addjmp(uint8_t opcode, uint8_t ty, uint32_t arg)
{
  switch(ty) {
    case 'A' : opcode |= 0x40 ; break ;
    case 'L' : opcode |= 0x80 ; break ;
    case 'R' : opcode |= 0xC0 ; break ;
  } 
  addop(opcode,arg);
}

#define targ(a,b) ((a) | ((b)<<8))

static void addtarget(uint32_t lbl)
{
  printf("%c%u:\n", lbl & 0xFF, lbl >> 8);
}

static void addtags(tagset_t ts)
{
  uint16_t k;
  uint8_t op;
  uint8_t capnum;
  uint32_t arg;
  
  if (ts != NULL) {
    for (k=0; k < ulvCnt(ts); k++) {
      op = yrxTagType(ts[k]);
      arg = (yrxTagDelta(ts[k]) << 8) | yrxTagExpr(ts[k]);
      if (op == TAG_MRK) 
        op=MRK;
      else if (op == TAG_FIN) 
        op=MTC;
      else {
        capnum = op & 0x1F;
        if (op >= 'a')
          op = CPB;
        else
          op = CPE;
        op = (op & 0x03) | (capnum << 2);
      }
      addop(op,arg);
    }
  }
}

static void linasm(state_t from, uint32_t first, ulv_t minmax)
{
  uint32_t k;
  uint8_t  pmin, pmax;
  uint32_t parc;
  uint8_t  jmpop;
  arc_t *a;

    for (k = 0; k < ulvCnt(minmax); k++) {
      pmin = minmax[k] >> 24;
      pmax = (minmax[k] >> 16) & 0x00FF;
      parc = minmax[k] & 0xFFFF;
      a = yrxDFAGetArc(from, parc);
      if (pmin == pmax) {
        addop(CMP, pmin);
        jmpop = JEQ;
      }
      else {
        if ( pmin > 0) {
          addop(CMP, pmin);
          addop(RLT, 0);
        }
        jmpop = JMP;
        if ( pmax < 255) {
          addop(CMP, pmax);
          jmpop = JLE;
        }
      }      
      if (a->tags) addjmp(jmpop, 'A', first + parc);
      else         addjmp(jmpop, 'S', a->to);
    }    
}

static void binasm(state_t from, uint32_t first, ulv_t minmax)
{
  int i,j,k;
  uint8_t  pmin, pmax;
  uint32_t parc;
  ulv_t stck=NULL;  
  arc_t *a;
  uint8_t  jmpop;
  uint32_t cnt_T = 0;
  static uint32_t cnt_L = 1;

  if (ulvCnt(minmax) > 0) {
    stck = ulvPush(stck,0);
    stck = ulvPush(stck,ulvCnt(minmax) - 1);
    while (ulvDepth(stck) > 1) {
      j = ulvPop(stck);
      i = ulvPop(stck);
      k = (i+j)/2;
      pmin = minmax[k] >> 24;
      pmax = (minmax[k] >> 16) & 0x00FF;
      parc = minmax[k] & 0xFFFF;
      
      a = yrxDFAGetArc(from, parc);
      
      addtarget(targ('L',cnt_L + k));
      jmpop = JLE;
      if (pmin == 0 && pmax == 255) {
        jmpop = JMP;
      }
      else if (((k+1) <= j) && (k-1) >= i) {
        cnt_T++;
        addop(CMP,pmax);
        addjmp(JGT,'L',cnt_L + ((k+1)+j)/2 );
        if (pmin != pmax) 
          addop(CMP,pmin);
        else
          jmpop = JEQ;
      } 
      else if ((k-1) >= i) {
        addop(CMP,pmax);
        addop(RGT,0);
        if (pmin != pmax) 
          addop(CMP,pmin);
        else
          jmpop = JEQ;
      }
      else if ((k+1) <= j) {
        addop(CMP,pmin);
        addop(RLT,0);
        if (pmin != pmax) 
          addop(CMP,pmin);
        else
          jmpop = JEQ;
      }
      else {
        addop(CMP,pmin);
        if (pmin != pmax) {
          addop(RLT,0);
          addop(CMP,pmax);
          addop(RGT,0);
        }
        else {
          addop(RNE,0);
        }
        jmpop = JMP;
      }
      
      if (a->tags) addjmp(jmpop, 'A', first + parc);
      else         addjmp(jmpop, 'S', a->to);        
      
      if ((k+1) <= j) {
        stck = ulvPush(stck,k+1);
        stck = ulvPush(stck,j);
      }
      if ( (k-1) >= i) {
        stck = ulvPush(stck,i);
        stck = ulvPush(stck,k - 1);
      }
    }
  }
  cnt_L += ulvCnt(minmax);
  stck = ulvFree(stck);
}

void yrxASM(int optimize)
{
  uint32_t from;
  uint8_t *pairs;
  tagset_t final_ts;
  uint32_t arcn = 1;
  uint32_t k;
  ulv_t minmax = NULL;
  arc_t *a;

  from = yrxDFAStartState();

  while (from != 0) {
    addtarget(targ('S',from));
    final_ts = NULL;
    
    k = 0;
    a = yrxDFAGetArc(from, 0);
    if (a == NULL) err(978,"Unexpected state!");
    
    if (a->lbl == yrxLblLambda) {
      final_ts = a->tags;
      a = yrxDFAGetArc(from, ++k);
    }
    
    minmax = ulvReset(minmax);
    
    if (a != NULL) addop(GET,0); 
    if (final_ts) addtags(final_ts);

    while (a != NULL) {
      pairs = yrxLblPairs(a->lbl);
      while (pairs[0] <= pairs[1]) {
        minmax = ulvAdd(minmax, (pairs[0] << 24) |
                                (pairs[1] << 16) | k);
        pairs += 2;
      }
      a = yrxDFAGetArc(from, ++k);
    }
    minmax = ulvSort(minmax);
    
    /*linasm(from, arcn, minmax);*/
    binasm(from, arcn, minmax);

    addop(RET,0); 
    
    k = 0;
    while ((a = yrxDFAGetArc(from, k++))) {
      if ((a->lbl != yrxLblLambda) && (a->tags != NULL)) {
        addtarget(targ('A',arcn+k-1));
        addtags(a->tags);    
        addjmp(JMP, 'S', a->to);
      }
    }

    arcn += k;
    from = yrxDFANextState(from);
  }
  minmax = ulvFree(minmax);
}
