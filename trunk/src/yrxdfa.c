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


/*****/

void yrxNFAInit()
{}

void yrxNFAAddarc(state_t from, state_t to, char *l, tag_t tag)
{
  printf("nfa_addarc(%d, %d, \"%s\", %08X)\n",from,to,l,tag);
}

void yrxNFAClose()
{}

