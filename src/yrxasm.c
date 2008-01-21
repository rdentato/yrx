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

#define asmop(a,b,c,d,e,f,)

asmop('C','B','D',0x61,"0110 0001","Capture Begin (with Displacement)") 
asmop('C','B','Z',0x60,"0110 0000","Capture Begin (with Zero displacement)")
asmop('C','E','D',0x63,"0110 0011","Capture End (with Displacement)")     
asmop('C','E','Z',0x62,"0110 0010","Capture End (with Zero displacement)")

asmop('C','M','P',0x31,"0011 0001","Compare")
asmop('C','M','Z',0x30,"0011 0000","Compare with 0")

asmop('D','I','M',0x37,"0011 0111","")

asmop('F','E','Q',0x49,"0100 1001","Fail on Equal")
asmop('F','G','E',0x4A,"0100 1010","Fail on Greater or Equal")
asmop('F','G','T',0x4B,"0100 1011","Fail on Greater Than")
asmop('F','L','E',0x4C,"0100 1100","Fail on Less or Equal")
asmop('F','L','T',0x4D,"0100 1101","Fail on Less Than")    
asmop('F','N','E',0x4E,"0100 1110","Fail on Not Equal")    )

asmop('G','E','T',0x32,"0011 0010","GET next input char"))

asmop('J','E','Q',0x41,"0100 0001","Jump on Equal")           
asmop('J','G','E',0x42,"0100 0010","Jump on Less Than")       
asmop('J','G','T',0x43,"0100 0011","Jump on Greater or Equal")
asmop('J','L','E',0x44,"0100 0100","Jump on Not Equal")    )  
asmop('J','L','T',0x45,"0100 0101","Jump on Less or Equal")   
asmop('J','M','P',0x47,"0100 0111","JuMP")
asmop('J','N','E',0x46,"0100 0110","Jump on Greater Than")    

asmop('J','O','F',0x40,"0100 0000","Jump on Fail")

asmop('M','R','K',0x33,"0011 0011","MaRK tag")
asmop('M','T','C',0x35,"0011 0101","MaTCh tag")
asmop('N','C','P',0x36,"0011 0110","Number of Captures")
asmop('N','O','P',0x38,"0011 1000","No OPeration")
asmop('N','R','X',0x34,"0011 0100","Number of Regular eXpressions")
asmop('R','E','T',0x4F,"0100 1111","RETurn")

/*****/


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
