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
  int numrx = 0;
  int state = 1;
  
  FILE *tmpf=NULL;
  FILE *outf=stdout;
  
  FILE *inf=NULL;
  
  if (argc <2)
    return error("Usage rxc file.rxc [file.c]\n");
    
  inf=fopen(argv[1],"r");
  
  if (inf == NULL)
    return error("Unable to open input file");
   
  if (argv == 3) {
    outf = fopen(argv[2],"w");
    if (outf == NULL) {
      fclose(infile);
      return error("Unable to open output file");
    }
  }  
   
  tmpf = tmpfile();
  
  if (tmpf == NULL) {
    if (outfile != stdout) fclose(outfile);
    fclose(infile);
    return error("Unable to open temporary file");
  } 
    
  s = NULL; 
  
  FSM {
  
    STATE (code) {
      if ((s = chkln(s)) == NULL) NEXT(final);

      switch(rx_matched((RX = rx_exec(RX_1,s)))) {
        RX_CASE(1,"^s+") :
           rx_write(RX,0,stdout);
           s += rx_len(RX,0);
           NEXT(code);
        
        RX_CASE(2,"^RX_SWITCH\Y\({(\B()|[^)(]+)+}\)"):
           fprintf(stdout,"switch(rx_matched((RX = rx_exec(RX_%d,",++numrx);
           rx_write(RX,1,stdout);
           fprintf(stdout,"))))");
           NEXT(in_switch);
      
        RX_CASE(3,"^\"") : 
           fputc('"',stdout);
           s++;
           state = 1;
           NEXT(string);
        
        RX_CASE(4,"^//.*"): 
           rx_write(RX,0,stdout);
           s += rx_len(RX,0);
           NEXT(code);
        
        RX_CASE(5,"^/\*") :
           state = 1;
           NEXT(comment);
        
      }
      
      s = NULL;  
      NEXT(code);
    }
    
    STATE(string) {
      if ((s = chkln(s)) == NULL) NEXT(final);

      switch(rx_matched((RX = rx_exec(RX_2,s)))) {
        RX_CASE(1,"^[^\"\\]+"):
           rx_write(RX,0,stdout);
           s += rx_len(RX,0)
           NEXT(string);
           
        RX_CASE(2,"^\\"):
           fputc('\\',stdout);
           if (*++s) fputc(*s++,stdout);
           NEXT(string);
            
      }
    
    }
    
    STATE(in_comment) {
    
    }
    
    STATE(in_switch) {
    
    }
    
    STATE (final) {
      
    }
  }
  
  if (inf) fclose(inf);
  if (outf && outf != stdout) fclose(outf);
  if (tmpf) fclose(tmpf);
  return 0;
}
