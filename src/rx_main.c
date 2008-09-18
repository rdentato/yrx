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

#define RX_MAXNFA 4096
#include "rx.h"

unsigned char nfa[RX_MAXNFA];

#define MAXLNBUF   1024
unsigned char lnbuf[MAXLNBUF];

void usage(void)
{
  fprintf(stderr,"RX v0.9b - (c)2008 Remo Dentato\n");
  fprintf(stderr,"Usage: rx [-a] [-b] [-r rfile [-s sfile]] ['r' ...] [ '' 's' ...]\n");
  exit(1);
}

void error(char *err)
{
  if (err) {
    fprintf(stderr,err);
    exit(1);
  }
}

void readfile(char *fname)
{
  FILE *f = NULL;
  char *err = NULL;
  char *s;
  
  if (fname) f = fopen(fname,"r");
  if (f == NULL) error("Unable to open file");
  
  while (!err && (s = fgets(lnbuf,MAXLNBUF,f))) {
    while (*s) s++;
    if ((s > (char *)lnbuf) && (s[-1] == '\n'))
      s[-1] = '\0';
    err=rx_compile_add(lnbuf,nfa);
  }
  fclose(f);
  error(err);
}

int readargs(int argc, char **argv, int argn)
{
  char *err = NULL;
  
  while (err == NULL && argn < argc) {  
    if (argv[argn][0] == '\0') {
      argn++;
      break;
    }
    err=rx_compile_add((unsigned char *)argv[argn],nfa);
    argn++;
  }
     
  error(err);
  return argn;
}

void outmatch(char *str)
{
  int j;
  rx_result rx;

  printf("%s -> ",str);
  if ((rx=rx_exec(nfa,(unsigned char *)str))) {
    printf("[%d]",rx_matched(rx));
    for (j=0; j <= RX_MAXCAPT ; j++) { 
      fputs("  (",stdout);
      rx_write(rx,j,stdout);
      fputs(")",stdout);
    }
    fputs("\n",stdout);
  } 
  else
    printf("NO MATCH\n");
}

#define DUMP_STR 1
#define DUMP_ASM 2

int main (int argc, char **argv)
{
    int argn = 1;
    unsigned int opts = 0;
    char *s = NULL;
    FILE *f;
    
    *nfa = '\0';
      
    while ((argn < argc) && argv[argn][0] == '-') {
      switch (argv[argn++][1]) {
        case 'h': usage();
                  break;
        
        case 'l' : if (argn < argc) 
                     setlocale(LC_CTYPE,argv[argn++]);
                   break;
                  
        case 'b' : opts |= DUMP_STR; break;
        case 'a' : opts |= DUMP_ASM; break;
        
        case 't' : rx_trace_on(); break;
        
        case 'r' : if (argn >= argc) usage();       
                   readfile(argv[argn++]);
                   break;
  
        case 's' : if (argn >= argc) usage();  
                   s = argv[argn++];     
                   break;
  
      }
    }
    
    if (*nfa == '\0')
      argn = readargs(argc,argv,argn);
  
    if (*nfa == '\0')
      usage();
  
    if (opts & DUMP_STR) {
      fprintf(stdout,"nfa: (%d)\n\t",strlen((char *)nfa));
      fputs("\"",stdout);
      rx_dump_str(stdout,nfa);
      fputs("\"",stdout);
      fputs("\n\n",stdout);
    }
          
    if (opts & DUMP_ASM) {
      rx_dump_asm(stdout,nfa);
    }
    
    if (s != NULL) {
      f = fopen(s,"r");
      if (f) {
        while ((s = fgets(lnbuf,MAXLNBUF,f))) {
          while (*s) s++;
          if ((s > (char *)lnbuf) && (s[-1] == '\n'))
            s[-1] = '\0';
          outmatch(lnbuf);
        }
        fclose(f);
      }
    }
    
    while (argn < argc) {
      outmatch(argv[argn++]);
    }
        
    return(0);
}

