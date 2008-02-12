
#include "yrxc.h"

#include "x.c"

int main(int argc, char **argv)
{
  yrxmatch *y;
  int i,j;
  char *buf=NULL;
  char buflen=0;
  
  if (argc > 1) {
    printf("String: %s\n",argv[1]);
    y = yrxlex(argv[1]);
    printf("Match: %d\n",y->rx);
    for (i=0;i<10;i++) {
      printf("  %d: %p %4d \"",i,y->capt[i],y->len[i]); 
      for (j=0; j < y->len[i]; j++) {
        putchar(y->capt[i][j]);
      }
      printf("\"\n"); 
    }
  }
  
  if (buf != NULL) free(buf);
  exit(0);  
}

