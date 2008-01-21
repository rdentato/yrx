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

#define asmop(x,a,b,c,d,e,f) static char op##x[] = {a,b,c,d};

asmop(CBD,'C','B','D',0x61,"0110 0001","Capture Begin (with Displacement)") 
asmop(CBZ,'C','B','Z',0x60,"0110 0000","Capture Begin (with Zero displacement)")
asmop(CED,'C','E','D',0x63,"0110 0011","Capture End (with Displacement)")     
asmop(CEZ,'C','E','Z',0x62,"0110 0010","Capture End (with Zero displacement)")
          
asmop(CMP,'C','M','P',0x31,"0011 0001","Compare")
asmop(CMZ,'C','M','Z',0x30,"0011 0000","Compare with 0")
          
asmop(DIM,'D','I','M',0x37,"0011 0111","")
          
asmop(FEQ,'F','E','Q',0x49,"0100 1001","Fail on Equal")
asmop(FGE,'F','G','E',0x4A,"0100 1010","Fail on Greater or Equal")
asmop(FGT,'F','G','T',0x4B,"0100 1011","Fail on Greater Than")
asmop(FLE,'F','L','E',0x4C,"0100 1100","Fail on Less or Equal")
asmop(FLT,'F','L','T',0x4D,"0100 1101","Fail on Less Than")    
asmop(FNE,'F','N','E',0x4E,"0100 1110","Fail on Not Equal")
          
asmop(GET,'G','E','T',0x32,"0011 0010","GET next input char")
          
asmop(JEQ,'J','E','Q',0x41,"0100 0001","Jump on Equal")           
asmop(JGE,'J','G','E',0x42,"0100 0010","Jump on Less Than")       
asmop(JGT,'J','G','T',0x43,"0100 0011","Jump on Greater or Equal")
asmop(JLE,'J','L','E',0x44,"0100 0100","Jump on Not Equal")
asmop(JLT,'J','L','T',0x45,"0100 0101","Jump on Less or Equal")   
asmop(JMP,'J','M','P',0x47,"0100 0111","JuMP")
asmop(JNE,'J','N','E',0x46,"0100 0110","Jump on Greater Than")    
          
asmop(JOF,'J','O','F',0x40,"0100 0000","Jump on Fail")
          
asmop(MRK,'M','R','K',0x33,"0011 0011","MaRK tag")
asmop(MTC,'M','T','C',0x35,"0011 0101","MaTCh tag")
asmop(NCP,'N','C','P',0x36,"0011 0110","Number of Captures")
asmop(NOP,'N','O','P',0x38,"0011 1000","No OPeration")
asmop(NRX,'N','R','X',0x34,"0011 0100","Number of Regular eXpressions")
asmop(RET,'R','E','T',0x4F,"0100 1111","RETurn")

/*****/



asmop(CPB,'C','B','D',0x02,"0000 0010","Capture Begin") 
asmop(CPE,'C','E','D',0x03,"0000 0011","Capture End")     
asmop(DIM,'D','I','M',0x05,"0000 0101","")
asmop(FEQ,'F','E','Q',0x06,"0000 0100","Fail on Equal")
asmop(FGE,'F','G','E',0x08,"0000 1000","Fail on Greater or Equal")
asmop(MTC,'M','T','C',0x09,"0000 1001","MaTCh tag")
asmop(FGT,'F','G','T',0x0C,"0000 1100","Fail on Greater Than")
asmop(MRK,'M','R','K',0x0D,"0000 1101","MaRK tag")
asmop(FLE,'F','L','E',0x10,"0001 0000","Fail on Less or Equal")
asmop(FLT,'F','L','T',0x14,"0001 0100","Fail on Less Than")    
asmop(FNE,'F','N','E',0x18,"0001 1000","Fail on Not Equal")
asmop(JOF,'O','N','F',0x20,"0010 0000","jump ON Fail")
asmop(CMP,'C','M','P',0x21,"0010 0001","Compare")
asmop(CMP,'C','M','Z',0x31,"0011 0001","Compare")
asmop(JEQ,'J','E','Q',0x24,"0010 0100","Jump on Equal")           
asmop(GET,'G','E','T',0x25,"0010 0101","GET next input char")
asmop(JGE,'J','G','E',0x28,"0010 1000","Jump on Less Than")       
asmop(NCP,'N','C','P',0x29,"0010 1001","Number of Captures")
asmop(JGT,'J','G','T',0x2C,"0010 1100","Jump on Greater or Equal")
asmop(NOP,'N','O','P',0x2D,"0010 1101","No OPeration")
asmop(JLE,'J','L','E',0x30,"0011 0000","Jump on Not Equal")
asmop(JLT,'J','L','T',0x34,"0011 0100","Jump on Less or Equal")   
asmop(JNE,'J','N','E',0x38,"0011 1000","Jump on Greater Than")    
asmop(NRX,'N','R','X',0x39,"0011 1001","Number of Regular eXpressions")
asmop(JMP,'J','M','P',0x3C,"0011 1100","JuMP")
asmop(RET,'R','E','T',0x3D,"0011 1101","RETurn")
                                   
                                   
                                   

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
  pgm = ulvAdd(pgm,(arg2 << 16) | (arg1 << 8) | opcode));
}



void yrxASM(void)
{
  uint32_t from;
  arc_t *a;
  uint8_t *pairs;

  from = yrxDFAStartState();

  while (from != 0) {
    while ((a = yrxDFANextArc()) != NULL) {
      pairs = yrxLblPairs(a->lbl);
      while (pairs[0] <= pairs[1]) {
        if (pairs[0] != pairs[1]) {
          if (pairs[1] > pairs[0] +1) 
            ;
        }
        pairs += 2;
      }
      
      if (a->tags) {
      }
    }
    from = yrxDFANextState(from);
  }
}