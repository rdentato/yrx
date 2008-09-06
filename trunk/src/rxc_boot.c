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

int swnum = 0;
int rxnum = 0;
int lnnum = 0;

int error(char *errmsg)
{
  fprintf(stderr,errmsg);  
  exit(1);
}

#define chk(x,e) if x error(e)

int main(int argc, char *argv[])
{
  char *s, *p;
  char *err;
  int c;
  
  chk((((tmpf = tmpfile()) == NULL)),"Unable to open tempfile");
  
  *nfa = '\0';
  
  while ((s = fgets(ln,ln_len,stdin))) {
    ++lnnum;
    
    while (isspace(*s)) fputc(*s++,tmpf);
    
    if (strncmp(s,"RX_SWITCH",8) == 0) {
      if (*nfa) {
        fprintf(stdout,"static unsigned char RX_%d[]= {\n",swnum);
        rx_dump_num(stdout,nfa);
        fprintf(stdout,"\n};\n");
      }
      fprintf(tmpf,"switch(rx_matched((r = rx_exec(RX_%d,s)))) {\n", ++swnum);
      rxnum = 0;
      *nfa = '\0';
      *s = '\0';
    }
    else if (strncmp(s,"case",4) == 0) {
      fputs("case",tmpf);
      s += 4;
      while (isspace(*s)) fputc(*s++,tmpf);
      if (*s == '"') {
        p = ++s;
        while (*s && *s != '"')
          if (*s++ == '\\') 
             if (*s) s++;;
        if (*s != '"') {
          fprintf(stderr,"%d:ERR203: Unterminated string\n",lnnum);
          exit(1);
        }
        *s = '\0';
        err = rx_compile_add(p,nfa);
        if (err) {
          fprintf(stderr,"%d:ERR205: %s\n",lnnum,err);
          exit(1);
        }
        fprintf(tmpf," %d",++rxnum);
        s++;        
      }
    }
    if (s) {
      fprintf(tmpf,"%s",s);
    }
  }
  
  if (*nfa) {
    fprintf(stdout,"static unsigned char RX_%d[]= {\n",swnum);
    rx_dump_num(stdout,nfa);
    fprintf(stdout,"\n};\n");
  }
  fputs("#line 1\n",stdout);
  
  fseek(tmpf,SEEK_SET,0);
  
  while ((c = fgetc(tmpf)) != EOF) 
    fputc(c,stdout);
  
  fclose(tmpf);
  exit(0);
}
