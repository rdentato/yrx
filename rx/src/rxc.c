static unsigned char RX_1[]= {
 35,128,134, 20, 98, 47, 42, 39, 35,128,133, 20, 18,  6, 39, 35,
128,132, 20, 23, 39, 35,128,136, 20, 98, 47, 47, 17, 11, 39, 35,
128,140, 20, 18,198,128,134, 32,132, 32,130,  2, 39, 35,128,143,
 20,100, 99, 97,115,101, 28, 40, 23, 48, 28, 97, 58, 39, 35,128,
174, 20,105, 82, 88, 95, 83, 87, 73, 84, 67, 72, 28, 97, 40, 40,
 27, 48, 97, 44, 41, 80,133, 19, 40, 41, 64,135, 80,138, 18,194,
128,133, 96, 32, 36,255, 64,147, 31,  1, 49, 97, 41, 39, 34,  0,

};
static unsigned char RX_2[]= {
 35,128,134, 20, 98, 42, 47, 39, 35,128,134, 20, 18,254, 42, 39,
 34,  0,
};
#line 1 "rxc.rxc"
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

FILE *tmpf;
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

#define IN_CODE    1
#define IN_COMMENT 2

int main(int argc, char *argv[])
{
  char *s;
  char *err;
  int c;
  int state = IN_CODE;  
  rx_result r;
  
  char *fname = "(stdin)";
    
  *nfa = '\0';
  inf  = stdin;
  outf = stdout;
  
  tmpf = tmpfile();
  if (tmpf == NULL) error("ERR202: Unable to open temp file");
  
  if (argc > 1) {
    fname = argv[1];
    inf = fopen(fname,"r");
    if (inf == NULL) error("ERR203: Unable to open input file");
  }
  
  while ((s = fgets(ln,ln_len,inf))) {
    ++lnnum;
    while (*s) {
      switch(state) {
      
        case IN_CODE :
          switch(rx_matched((r = rx_exec(RX_1,s)))) {
            case 1 : state = IN_COMMENT;
            case 2 : 
            case 3 : 
            case 4 : 
            case 5 : s = rx_end(r,0);
                              rx_write(r,0,tmpf);
                              break;
               
            case 6 :
                              s = rx_end(r,0);
                              fprintf(tmpf,"case %d :",++rxnum);
                              rx_end(r,1)[-1] = '\0';
                              err = rx_compile_add(rx_start(r,1)+1,nfa);
                              if (err) {
                                fprintf(stderr,"%d:ERR205: %s\n",lnnum,err);
                                exit(1);
                              }
                              break;

            case 7 :
                              s = rx_end(r,0);
                              if (*nfa) {
                                fprintf(outf,"static unsigned char RX_%d[]= {\n",swnum);
                                rx_dump_num(outf,nfa);
                                fprintf(outf,"\n};\n");
                              }
                              fprintf(tmpf,"switch(rx_matched((");
                              rx_write(r,1,tmpf);
                              fprintf(tmpf," = rx_exec(RX_%d,", ++swnum);
                              rx_write(r,2,tmpf);
                              fprintf(tmpf,"))))");
                              
                              rxnum = 0;
                              *nfa = '\0';
                              break;      

                              
            default :         if (*s) fputc(*s++,tmpf);
                              break;                        
          }      
          break;
          
        case IN_COMMENT :
        
          switch(rx_matched((r = rx_exec(RX_2,s)))) {
            case 1 : state = IN_CODE;
            case 2 : s = rx_end(r,0); 
                             rx_write(r,0,tmpf);
                             break;
               
            default :        if (*s) fputc(*s++,tmpf);
                             break;         
          }      
          break;
            
        default :  error("Unexpected state!");
                   break;                        
      }
    }
  }
  
  if (*nfa) {
    fprintf(outf,"static unsigned char RX_%d[]= {\n",swnum);
    rx_dump_num(outf,nfa);
    fprintf(outf,"\n};\n");
  }
  
  fprintf(outf,"#line 1 \"%s\"\n",fname);
  
  fseek(tmpf,SEEK_SET,0);
  
  while ((c = fgetc(tmpf)) != EOF) 
    fputc(c,outf);
  
  if (outf != stdout) fclose(outf);
  if (inf  != stdin)  fclose(inf);
  fclose(tmpf);
  exit(0);
}
