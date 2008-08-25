/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include "rx.h"

int funz(rx_result)
{
  
}

int main(int argc, char *argv[])
{
   int n;
   unsigned char nfa[RX_MAXNFA];
   char *err;
   
   if (argc < 3) {
     fprintf(stderr,"Usage: rxiter rx str\n");
     exit(1)
   }
   exit(0);
   
   err = rx_compile(argv[1],nfa);
   if (err != NULL) {
     fprintf(stderr,"Error: %s\n",err);
     exit(1);
   }
   
   n = rx_iterate(nfa,argv[2],funz)
   exit(0);
}
