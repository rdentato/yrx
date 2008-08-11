/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */


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
      
      RX = rx_exec ("#\200\245\24\34iRX_SWITCH\34a((P\205\23()@\204P\207"
                    "\376) $\377@\220\60a)'#\200\210\24\34b/*\34'"
                    "#\200\206\24\34a\42'"
                    "#\200\214\24\22\306\200\204@\201 \204 '\42",s)
      sw
      while (*s) {
        RX = rx_match("^\YRX_SWITCH\Y\({(\B()|[^\)])*}\)",s,&err);
        
        if (RX) {
          s = rx_eoc(RX,0);
          NEXT(in_switch);
        }
        else if (err) return error(err); 
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
