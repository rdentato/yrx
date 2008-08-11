/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include <stdio.h>
#include "rx.h"

unsigned char *buf = NULL;
size_t buf_sz = 0;

unsigned char *mp_read(char *fname)
{
  FILE *f;
  size_t len,k;
  
  f = fopen(fname,"rb");
  if (f == NULL) return NULL; 

  /* try guessing needed buffer size */
  buf_sz = fseek(f,0,SEEK_END);
  buf_sz = ftell(f); 
  
  fseek(f,0,SEEK_SET); /* back to the file start */
  
  free(buf);
  buf = malloc(buf_sz+16);/* will need to add '\0' */
  
  if (buf) {
    len = fread(buf, 1,buf_sz+1, f);
    if (len != buf_sz) {
      /* TODO: Instead of failing, let's continue reading if needed */
      fprintf(stderr,"Cannot trust ftell() %d |= %d!\n",len,buf_sz);
      free(buf);
      buf=NULL;
    }
    else {
      buf[buf_sz] = '\0';
    }
  }
  fclose(f);
  return buf;
}

int usage()
{
  fprintf(stderr,"Usage: mpad file");
  return (1);
}

int main(int argc, char *argv[])
{
  unsigned char *mptext;
  
  if (argc < 2) return usage();
  
  mptext = mp_read(argv[1]);
  
  
  
  free(mptext);
  return 0;
}
