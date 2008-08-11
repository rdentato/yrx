/*  YRX - rx_main.c
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

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>
#include "rx.h"

unsigned char nfa[RX_MAXNFA];

void usage(void)
{
  fprintf(stderr,"RX v0.9b - (c)2008 Remo Dentato\n");
  fprintf(stderr,"Usage: rx [-l locale|-b] regexp [string]\n");
  exit(0);
}

int main (int argc, char **argv)
{
   char *err = NULL;
   int argn = 1;
   int j;
   int binonly = 0;
   rx_result rx;
   
   while ((argn < argc) && argv[argn][0] == '-') {
     switch (argv[argn++][1]) {
       case 'h': usage();
                 break;
       
       case 'l': if (argn < argc) 
                   setlocale(LC_CTYPE,argv[argn++]);
                 break;
                 
       case 'b': binonly = 1; break;
     }
   }
   if (!binonly)
     fprintf(stdout, "Locale: \"%s\"\n",setlocale(LC_CTYPE,NULL));
   
   if (argn < argc) {
     if (!binonly)
       fprintf(stdout,"Rex: \"%s\"\n",argv[argn]);
    *nfa = '\0';
     
     while (err == NULL && argn < argc) {    
       if (argv[argn][0] == '\0') {
         break;
       }
       err=rx_compile_add(argv[argn],nfa);
       argn++;
     }
     
     
     if (err == NULL) {
       if (!binonly)
         fprintf(stdout,"nfa: (%d)\n\t",strlen(nfa));
       fputs("\"",stdout);
       rx_hexdump(stdout,nfa);
       fputs("\"",stdout);
       if (binonly) exit(0);
       fputs("\n\n",stdout);
       rx_symdump(stdout,nfa);
       
       for (argn++ ;argn < argc; argn++) {
         printf("%s -> ",argv[argn]);
         if ((rx=rx_exec(nfa,argv[argn]))) {
           printf("[%d]",rx_matched(rx));
           for (j=0; j <=8 ; j++) { 
             fputs("  (",stdout);
             fwrite(rx_start(rx,j),rx_len(rx,j),sizeof(unsigned char),stdout);
             fputs(")",stdout);
           }
           fputs("\n",stdout);
         } 
         else
           printf("NO MATCH\n");
       }
     }
     else
       fprintf(stderr,"%s\n",err);
   }
   else usage();
   return(0);
}

