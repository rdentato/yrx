/*  RX - bgsub.c
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
*/

#include "rx_.h"
#include "bstrlib.h"

bstring *bgsub(const unsigned char *nfa, bstring str,
                                             int *n, char *(*fsub)(rx_result))
{ 
  
  return(NULL);
}

/*
** removes z and substitute a->y and b->w       
*/
char *fsub(rx_result rx)
{
  char *letter;
  letter = rx_start(rx,0);
  if (letter == NULL) return NULL;
  if (rx_length(rx,0)
  switch (*letter) {
    case 'z' : return "";
    case 'a' : return "y";
    case 'b' : return "w";
    default  : return NULL;
  }
  return NULL;
}  
  

int main()
{
  bgsub(rx_compile("([^azb]*)([azb])"));
  exit(0);
}
