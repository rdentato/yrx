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


#define CPB 0x02  /*  000000 10  Capture Begin                 */
#define CPE 0x03  /*  000000 11  Capture End                   */

#define MTC 0x01  /* 00 0000 01  MaTCh                         */
#define MRK 0x05  /* 00 0001 01  MaRK                          */

#define CMP 0x21  /* 00 1000 01  Compare                       */
#define GET 0x25  /* 00 1001 01  GET next input char           */
#define DIM 0x29  /* 00 1010 01                                */
#define NCP 0x2D  /* 00 1011 01  Number of Captures            */
#define NRX 0x31  /* 00 1100 01  Number of Regular eXpressions */
#define NOP 0x35  /* 00 1101 01  No OPeration                  */

#define ONF 0x04  /* 00 0001 00  jump ON Fail                  */
#define FEQ 0x08  /* 00 0010 00  Fail on Equal                 */
#define FGE 0x0C  /* 00 0011 00  Fail on Greater or Equal      */
#define FGT 0x10  /* 00 0100 00  Fail on Greater Than          */
#define FLE 0x14  /* 00 0101 00  Fail on Less or Equal         */
#define FLT 0x18  /* 00 0110 00  Fail on Less Than             */
#define FNE 0x1C  /* 00 0111 00  Fail on Not Equal             */

#define RET 0x20  /* 00 1000 00  RETurn                        */
#define JMP 0x24  /* 00 1001 00  JuMP                          */
#define JEQ 0x28  /* 00 1010 00  Jump on Equal                 */
#define JGE 0x2C  /* 00 1011 00  Jump on Less Than             */
#define JGT 0x30  /* 00 1100 00  Jump on Greater or Equal      */
#define JLE 0x34  /* 00 1101 00  Jump on Not Equal             */
#define JLT 0x38  /* 00 1110 00  Jump on Less or Equal         */
#define JNE 0x3C  /* 00 1111 00  Jump on Greater Than          */


static char *op[0x40];

#define opcode(a) op[a] = #a 
static void op_init(void)
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
}                                  
                                   
                                   

typdef struct {
  arc_t  *arc;
  uint8_t min;
  uint8_t max;
} rng2arc;


typedef uint32_t step;


#define setOP(s,x)   (((s) = ((s) & ~0xFF))  | ((x) & 0xFF))
#define getOP(s)     ((s) & 0xFF)
#define setARG1(s,x) (((s) = ((s) & ~0xFF00)) | (((x) & 0xFF) << 8))
#define getARG1(s)   (((s) & 0xFF00) >> 8)
#define setARG2(s,x) (((s) = ((s) & 0xFFFF)) | (((x) & 0xFFFF) << 16))
#define getARG2(s)   ((s) >> 16)

static ulv_t pgm; 


static void addop(uint8_t opcode, uint8_t arg1, uint16_t arg2)
{
  uint32_t step;
 
  printf("\t%s %u,%u\n",op[opcode], arg1, arg2);
  /* 
  step = (arg2 << 16) | (arg1 << 8) | opcode);
  pgm = ulvAdd(pgm,step);
  */
}

static void addtarget(uint8_t type, uint16_t sub, uint16_t state)
{
  printf("%c_%u_%u:\n",type, state, sub);
}


void yrxASM(void)
{
  uint32_t from;
  arc_t *a;
  uint8_t *pairs;

  from = yrxDFAStartState();

  while (from != 0) {
    addlbl('S',0,from);
    addop(GET,0,0);
    while ((a = yrxDFANextArc()) != NULL) {
      if (a->lbl == yrxLblLambda) 
      pairs = yrxLblPairs(a->lbl);
      while (pairs[0] <= pairs[1]) {
        if (pairs[0] == pairs[1]) {
          addop(CMP,pairs[0],0);
          addop(JEQ,0,a->to);
        }
        else if (pairs[1] > pairs[0] +1) {
        } 
            ;
        pairs += 2;
      }
      
      if (a->tags) {
        addlbl('A',0,from);
      }
    }
    from = yrxDFANextState(from);
  }
}
