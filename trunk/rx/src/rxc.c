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
  int n;
  
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

      switch(rx_matched((RX = rx_exec("",s)))) {
        RX_CASE(1,"^s+") :
           rx_write(RX,0,stdout);
           s += rx_len(RX,0);
           NEXT(code);
        
        RX_CASE(2,"^RX_SWITCH\Y\({(\B()|[^)(]+)+}\)"):
           NEXT(in_switch);
      
        RX_CASE(3,"^\"") : 
           fputc('"',stdout);
           s++;
           NEXT(string);
        
        RX_CASE(4,"^//.*"): 
           rx_write(RX,0,stdout);
           s += rx_len(RX,0);
           NEXT(code);
        
        RX_CASE(5,"^/*") : NEXT(in_comment);
        
      }
      
      s = NULL;  
      NEXT(code);
    }
    
    STATE(string) {
    
    }
    
    STATE(in_comment) {
    
    }
    
    STATE(in_switch) {
    
    }
    
    STATE (final) {
      
    }
  }
  
  fclose(f);
  return 0;
}
