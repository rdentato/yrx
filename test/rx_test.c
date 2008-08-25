#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include "rx.h"

unsigned char nfa[RX_MAXNFA];

#define MAX_LINELEN 256
unsigned char buf[MAX_LINELEN];

int main(int argc, char **argv)
{
  FILE *in=stdin;
  int j;
  unsigned char *t,*e;
  rx_result r;

  
  FILE *out = stdout;
  int repeat = 1;
  
  *nfa = 0;
  j=1;
  while (j < argc) {
    if (argv[j][0] == '-') {
      switch (argv[j][1]) {
        case 'q' : out = NULL; break;
        case 'r' : repeat = atoi(argv[j]+2); break;
      }
    }
    j++;
  }
  if (repeat == 0) repeat = 1;
  
  while (fgets(buf,MAX_LINELEN,in)) {
    t=buf+strlen(buf);
    while (t>buf && isspace(t[-1])) *--t='\0';
    if (buf[0] == '-' && buf[1] == '-') {
      return 0;
    } else if (buf[0] != '#' && buf[0]!='\0') {
      t=buf;
      if (isspace(*t)) {
        if (*nfa != '\0') {
          while (isspace(*t)) t++;
          for (j=0; j<repeat; j++)
             r = rx_exec(t,nfa);
          if (out) {
            fprintf(out,"\t'%s' -> ",t);
            for (j=0; rx_len(r,j) >= 0; j++) { 
              fputs("  (",out);
              rx_fwrite(r,j,out);
              fputs(")",out);
            }
            fputs("\n",out);
          }
        }
      }
      else {
        for (j=0; j<repeat; j++)
          e=rx_compile(t,nfa);
        if (out) {
          fprintf(out,"\n'%s' ",t);
          fputs("\"",out);
          rx_hexdump(out,nfa);
          fputs("\"\n",out);

          if (e != NULL) {
            fprintf(out,"ERROR: %s\n",e);
            /**nfa=0;*/
          }
        }
      }
    }
  }
  return 0;
}
