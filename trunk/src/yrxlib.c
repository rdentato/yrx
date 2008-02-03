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
