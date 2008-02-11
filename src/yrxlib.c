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

#define HUL_MAIN
#define YRX_MAIN

#include "yrxlib.h"

void yrxInit(void)
{
  yrxLblInit(); 
  yrxDFAInit();
  yrxASMInit();

  yrxFileIn  = stdin;
  yrxFileOut = stdout;
}

void yrxCleanup(void)
{
  yrxLblClean(); 
  yrxDFAClean();
  yrxASMClean();  
  if (yrxFileIn && yrxFileIn != stdin)
    fclose(yrxFileIn);
  if (yrxFileOut && yrxFileOut != stdout)
    fclose(yrxFileOut);
}

/**************************************************/

/* == A special flavour of stack
**  This function implements a stack of integers where values
** can be pushed only once.
**  For example in the code:
** {{
**    push(4);
**    ...
**    push(4);
** }}
** the value 4 will be pushed only the first time.
**  Values must be strictly positive as pushing 0 has the effect
** of resetting the stack.
*/

yrxStack *yrxStkNew()
{
  yrxStack *ys;
  
  ys = malloc(sizeof(yrxStack));
  if (ys == NULL) err(349,yrxStrNoMem);
  
  ys->stack  = usvNew();
  ys->pushed = bitNew();
  
  return ys;
}


yrxStack *yrxStkFree( yrxStack *ys)
{
  if (ys) {
    ys->stack  = usvFree(ys->stack);
    ys->pushed = bitFree(ys->pushed);
    free(ys);
  }
  return NULL; 
}

yrxStack *yrxStkReset(yrxStack *ys)
{
  if (ys != NULL) {
    ys->stack  = usvFree(ys->stack);
    ys->pushed = bitFree(ys->pushed);
  }
  return ys; 
}

state_t yrxStkPop(yrxStack *ys)
{
  if (ys == NULL) return 0;
  return usvPop(ys->stack);
}

yrxStack *yrxStkPushOnce(yrxStack *ys,state_t val)
{
  if (ys == NULL) ys = yrxStkNew();
  if (!bitTest(ys->pushed,val)) {
    ys->stack  = usvPush(ys->stack,val);
    ys->pushed = bitSet(ys->pushed,val);
  }
  
  return ys;  
}


int yrxStkPushed(yrxStack *ys,state_t val)
{
  if (ys == NULL) return 1;
  return (0 != bitTest(ys->pushed,val)); 
}


