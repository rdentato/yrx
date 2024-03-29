static unsigned char RX_1[]= { /* 101 */
 36,128,146, 44, 97, 35,185, 60, 99, 69, 82, 82, 80,132,186, 41,
 64,134, 42, 33, 36,128,140, 44, 97, 35, 80,132,186, 41, 64,134,
 42, 33, 36,128,139, 44, 80,132,181, 41, 64,134, 43,  1, 33, 36,
128,158, 44,185, 60, 80,132,179, 41, 64,134, 42,  8,  2, 92, 97,
 47, 80,133, 97, 47, 34, 64,132,186, 39, 64,139, 42, 16, 33, 36,
128,146, 44,  8, 55, 16,  9,185, 60, 17, 80,133, 96, 10, 41, 64,
135, 42, 33, 37,  0,
};
#define RX_SWITCH(r,s,n) switch ((r = rx_exec(RX_##n,s))? (s=rx_end(r,0),rx_matched(r)) : 0)
#line 1 "rx_test.rxc"
/* 
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
#include <strings.h>
#include <ctype.h>

#define RX_MAXNFA 4096
#include "rx.h"

unsigned char nfa[RX_MAXNFA];

#define ln_len 2048
char ln[ln_len];

FILE *inf;
FILE *outf;

int swnum = 0;
int rxnum = 0;
int lnnum = 0;

int error(char *errmsg)
{
  fprintf(stderr,errmsg);  
  exit(1);
}

#define READING  1
#define MATCHING 2

int main(int argc, char *argv[])
{
  char *s;
  char *err;
  int c;
  int state = READING;
  int argn = 1;  
  int sym = 0;
  int xprn = 1;
  rx_result r;
  
  char *fname = "(stdin)";
    
  *nfa = '\0';
  inf  = stdin;
  outf = stdout;

  while ((argn < argc) && (argv[argn][0] == '-')) {
    if (argv[argn][1] == 's') sym = 1;
    argn++;
  }
    
  if (argn < argc) {
    fname = argv[argn];
    inf = fopen(fname,"r");
    if (inf == NULL) error("ERR203: Unable to open input file");
  }
  
  while ((s = fgets(ln,ln_len,inf))) {
    /*fprintf(stderr,"-- %s\n",s);*/
    lnnum++;
    while (*s) {
      RX_SWITCH(r,s,1) { 
                                
        case 1 : break;
                             
        case 2 :
        case 3 : rx_write(r,0,outf);
                             break;
           
        case 4 :
                          if (state == MATCHING) {
                            *nfa = '\0';
                            xprn = 1;      
                            state = READING;                      
                          }
                          fprintf(outf,"%d/",xprn++);
                          rx_end(r,1)[-1] = '\0';
                          fwrite(rx_start(r,1)+1, rx_len(r,1)-2, 1, outf);
                          err = rx_compile_add(rx_start(r,1)+1,nfa);
                          fputc('/',outf);
                          if (err) {
                            fprintf(stderr,"%d:ERR205: %s\n",lnnum,err);
                            exit(1);
                          }
                          break;

        case 5 :
                          state = MATCHING;
                          rx_write(r,1,outf);
                          rx_write(r,2,outf);
                          rx_end(r,1)[-1] = '\0';
                          r = rx_exec(nfa,rx_start(r,1)+1);
                          fprintf(outf,"%d",rx_matched(r));
                          for (c=0; c <= RX_MAXCAPT ; c++) { 
                            fputs(" {",outf);
                            rx_write(r,c,outf);
                            fputs("}",outf);
                          }
                          break;
                          
        default :         if (*s) {
                            fputc(*s++,outf);
                            fputc('.',outf);
                          }
                          break;                        
      }            
    }
  }
    
  if (outf != stdout) fclose(outf);
  if (inf  != stdin)  fclose(inf);
  exit(0);
}
