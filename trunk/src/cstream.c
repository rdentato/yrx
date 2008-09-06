/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*
             _______ _______ _______ ______
            /  ____//  ____//_   __//  __  )
           /  /___ /___   /  /  /  /     _/ 
          (______//______/  /__/  /__/\__\
            
                                                                            */
/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>


typedef struct cstream {
  unsigned char *buf;
  unsigned char *max; 
  unsigned char *end;
  unsigned char *cur;
  unsigned long  size;
  unsigned char  mode;
  unsigned char  pad;
} CSTREAM;


int   cgetc  (CSTREAM*);
char* cgets  (char*, int, CSTREAM*);
int   cputc  (int, CSTREAM*);
int   cputs  (const char*, CSTREAM*);
int   ceof   (CSTREAM*);
int   cerror (CSTREAM*);

/* Check that the buf has at least n bytes available
*/
static unsigned char *chkavail(CSTREAM *s, unsigned long n)
{
  unsigned char *b = s->buf;
  unsigned long sz;
  
  if ((s->cur + n) > s->max) {
    sz = s->size;
    while((s->cur + n) > (s->buf + sz))
      sz <<= 1;
    b = realloc(b, sz);
    if (b) {
      s->buf = b;
      s->size = sz;
      s->max = s->buf + sz;
    }
  }
  return b;
}

CSTREAM *copen()
{
   CSTREAM *s;
   s = malloc(sizeof(CSTREAM));
   if (s) {
     s->buf = NULL; s->max = NULL;
     s->cur = NULL; s->end = NULL;
     s->size = 0; s->mode = 'w';
   }
   return s;
}

void cclose(CSTREAM *s)
{
  free (s->buf); free(s);
}

int cputc (int c, CSTREAM *s)
{
  if (chkavail(s,1)) {
   *(s->cur++) = (unsigned char)c;
    s->end = s->cur;     
    return c;
  }
  return -1;
}

int cputs (const char*p, CSTREAM*s)
{
  int k;
  
  while (chkavail(s,10)) {
    for (k=0; k<10 && *p; k++) {
      *(s->cur++) = *p++;
    }
    if (*p == '\0') {
      *(s->cur++) = '\0';
      s->end = s->cur;
      return 0;
    }      
  }
  return -1;
}

int cseek(CSTREAM *s, long offset, int origin)
{
  switch(origin) {
   case SEEK_SET: s->cur = s->buf + offset; break;
   case SEEK_END: s->cur = s->end - offset; break;
   case SEEK_CUR: s->cur = s->cur + offset; break;
  }
  return 0;
}

int ctell(CSTREAM *s)
{
  return (s->cur - s->buf);
}

size_t cwrite(void *buf, size_t size, size_t num, CSTREAM *s)
{

}

CSTREAM *creadfrom(CSTREAM *s)
{

}

int cgetc(CSTREAM *s)
{
  if (s->cur == s->max) return EOF;
  return *(s->cur++);
}

char* cgets (char* t, int n, CSTREAM *s)
{

}


size_t cread(void *buf, size_t size, size_t num, CSTREAM *s)
{

}

int main(int argc, char *argv[])
{
}

