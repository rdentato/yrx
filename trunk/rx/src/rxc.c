/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

/*

   RX_SWITCH(str) {
   
     case "^ab" : ...
      
     case "^c\d" : ...
   
   }


*/


#include <stdio.h>
#include <stdlib.h>
#include "rx.h"

int error(char *errmsg)
{
  fprintf(stderr,errmsg);  
  return 1;
}

#define MAXLEN 2048

char buf[MAXLEN];

#define chkln(x) ((x && *x ) ? x : fgets(buf,MAXLEN,f))

int main(int argc, char *argv[])
{
  rx_result RX;
  unsigned char *nfa;
  char *s;
  char *err = NULL;
  
  FILE *f;
  
  if (argc <2)
    return error("Usage rxc file.rxc > file.c\n");
    
  f=fopen(argv[1],"r");
  
  if (f == NULL)
    return error("Unable to open input file");
    
    
  s = NULL; 
  
  
  FSM {
  
    STATE (code) {
      if ((s = chkln(s)) == NULL) NEXT(final);

      switch(s) {
        RX_CASE(1,"^s+") :
           NEXT(code);
        
        RX_CASE(2,"^RX_SWITCH\Y\({(\B()|[^)(]+)+}\)"):
           NEXT(in_switch)
      
        RX_CASE(3,"^\"") : NEXT(string);
        
        RX_CASE(4,"^//.*")
      }
      
      fprintf(stdout,s);
      s = NULL;  
      NEXT(code);
    }
    
    STATE (final) {
      
    }
  }
  
  fclose(f);
  return 0;
}
