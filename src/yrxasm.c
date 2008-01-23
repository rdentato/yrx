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


#define CPB 0x02  /* 0 00000 10  Capture Begin                 */
#define CPE 0x03  /* 0 00000 11  Capture End                   */

#define MTC 0x01  /* 00 0000 01  MaTCh                         */
#define MRK 0x05  /* 00 0001 01  MaRK                          */

#define NRX 0x21  /* 00 1000 01  Number of Regular eXpressions */
#define CMP 0x25  /* 00 1001 01  Compare                       */
#define DIM 0x29  /* 00 1010 01                                */
#define NCP 0x2D  /* 00 1011 01  Number of Captures            */
#define GET 0x31  /* 00 1100 01  GET next input char           */
#define NOP 0x35  /* 00 1101 01  No OPeration                  */

#define RET 0x04  /* 00 0001 00  RETurn                        */
#define FEQ 0x08  /* 00 0010 00  Fail on Equal                 */
#define FGE 0x0C  /* 00 0011 00  Fail on Greater or Equal      */
#define FGT 0x10  /* 00 0100 00  Fail on Greater Than          */
#define FLE 0x14  /* 00 0101 00  Fail on Less or Equal         */
#define FLT 0x18  /* 00 0110 00  Fail on Less Than             */
#define FNE 0x1C  /* 00 0111 00  Fail on Not Equal             */

#define ONF 0x20  /* 00 1000 00  jump ON Fail                  */
#define JMP 0x24  /* 00 1001 00  JuMP                          */
#define JEQ 0x28  /* 00 1010 00  Jump on Equal                 */
#define JGE 0x2C  /* 00 1011 00  Jump on Less Than             */
#define JGT 0x30  /* 00 1100 00  Jump on Greater or Equal      */
#define JLE 0x34  /* 00 1101 00  Jump on Not Equal             */
#define JLT 0x38  /* 00 1110 00  Jump on Less or Equal         */
#define JNE 0x3C  /* 00 1111 00  Jump on Greater Than          */

#define isjmp(x) (((x) & 0x23) == 0x20)

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
  opcode(ONF);
  opcode(FEQ);
  opcode(FGE);
  opcode(FGT);
  opcode(FLE);
  opcode(FLT);
  opcode(FNE);
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


#define setOP(s,x)   ((s) = ((s) & ~0xFF)  | ((x) & 0xFF))
#define getOP(s)     ((s) & 0xFF)
#define setARG1(s,x) (((s) = ((s) & ~0xFF00)) | (((x) & 0xFF) << 8))
#define getARG1(s)   (((s) & 0xFF00) >> 8)
#define setARG2(s,x) (((s) = ((s) & 0xFFFF)) | (((x) & 0xFFFF) << 16))
#define getARG2(s)   ((s) >> 16)
#define setARG0(s,x) (((s) = ((s) & 0xFF)) | (((x) & 0xFFFFFF) << 8))
#define getARG0(s)   ((s) >> 8)

static void addop(uint8_t opcode, uint32_t arg)
{
  uint32_t step;

  /* printf(";\t0x%02X 0x%02X %d\n",opcode, opcode & 0xD3, isjmp(opcode));*/
  if (opcode == CMP) {
    printf("\t%s '%c'\n",op[opcode],arg);
  }
  else if (isjmp(opcode)) {
    printf("\t%s %c%u\n",op[opcode],arg & 0xFF,arg>>8);
  }
  else {
    switch (nargs(opcode)) {
      case 0: printf("\t%s\n",op[opcode]); break;
      case 1: printf("\t%s %X\n",op[opcode],arg); break;
      case 2: printf("\t%s %X\n",op[opcode],arg); break;
    }
  }
  /* 
  step = (arg2 << 16) | (arg1 << 8) | opcode);
  pgm = ulvAdd(pgm,step);
  */
}

#define targ(a,b) ((a) | ((b)<<8))

static void addtarget(uint32_t lbl)
{
  printf("%c%u:\n", lbl & 0xFF, lbl >> 8);
}


static void addtags(tagset_t ts)
{
}

void yrxASM(int optimize)
{
  uint32_t from;
  arc_t *a;
  uint8_t *pairs;
  tagset_t final_ts;
  uint16_t arcn = 0;

  from = yrxDFAStartState();

  while (from != 0) {
    addtarget(targ('S',from));
    arcn = 0;
    final_ts = NULL;
    
    a = yrxDFAFirstArc(from);
    
    if (a == NULL) err(978,"Unexpected state!");
    
    if (a->lbl == yrxLblLambda) {
      final_ts = a->tags;
      a = yrxDFANextArc();
      arcn++;
    }
    
    if (a != NULL) addop(GET,0); 
    
    while (a != NULL) {
      pairs = yrxLblPairs(a->lbl);
      while (pairs[0] <= pairs[1]) {
        if (pairs[0] == pairs[1]) {
          addop(CMP, pairs[0]);
          addop(JEQ, targ('S',a->to));
        }
        else {
          addop(CMP, pairs[0]);
          addop(FLT, 0);
          addop(CMP, pairs[1]);
          addop(JLT, targ('S', a->to));
        } 
        pairs += 2;
      }
      if (a->tags) {
      }
      a = yrxDFANextArc();
      arcn++;
    }
    if (final_ts != NULL) {
      addop(MTC,0);
    } 
      addop(RET,0); 
    from = yrxDFANextState(from);
  }
}
