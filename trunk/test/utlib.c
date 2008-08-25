/*  UT - utlib.c
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
#include <ctype.h>
#include <setjmp.h>
#include <string.h>

#include "ut.h"

static char buf[256];

jmp_buf test_fail;

void UTStringsEqual(char *stra, char *strb)
{
  if (stra != NULL && strb != NULL && strcmp(stra,strb) == 0) return;
  fprintf(stderr,"FAIL: String differs (\"%s\" != \"%s\") \n",(stra?stra:"(null)"),(strb?strb:"(null)"));
  longjmp(test_fail,1);
}

void UTPointersEqual(void *ptra, void *ptrb)
{
  if (ptra == ptrb) return;
  fprintf(stderr,"FAIL: Pointers differs (0x%p != 0x%p)\n",ptra,ptrb);
  longjmp(test_fail,1);
}

void UTUnsignedEqual(unsigned numa, unsigned numb)
{
  if (numa == numb) return;
  fprintf(stderr,"FAIL: Numbers differs (%d != %d)\n",numa,numb);
  longjmp(test_fail,1);
}

void UTSignedEqual(signed numa, signed numb)
{
  if (numa == numb) return;
  fprintf(stderr,"FAIL: Numbers differs (%d != %d)\n",numa,numb);
  longjmp(test_fail,1);
}

static void chkfile(char *fname, char *dname,unsigned char txt)
{
  FILE *f1=NULL, *f2=NULL;
  int c1=EOF, c2=EOF, res = 1, off=0, neq;
  
  f1 = fopen(fname,"rb");
  if (f1 == NULL) {
    fprintf(stderr,"FAIL: Unable to open input file '%s'\n",fname);
  }
  else {
    sprintf(buf,"%s/%s",dname,fname);
    f2 = fopen(buf,"rb");
    if (f2 == NULL) { 
      fprintf(stderr,"FAIL: Unable to open check file in '%s'\n",dname);
    }
  }
  
  if (f1 != NULL && f2 != NULL) {
    do {     
      if (c1 != c2) {
        fprintf(stderr,"FAIL: File \"%s/%s\" differs at offset %d ('%c' != '%c')\n",dname,fname,off,c1,c2);
        break;
      }
      c1 = fgetc(f1); c2 = fgetc(f2); off++;
      if (txt) {
        if (c1 == '\r') {
          c1=fgetc(f1);
          if (c1 != '\n') 
            ungetc(c1,f1);
          c1='\n';
        }
        if (c2 == '\r') {
          c2=fgetc(f2);
          if (c2 != '\n') 
            ungetc(c2,f2);
          c2='\n';
        }
      }
    } while (c1 != EOF && c2 != EOF);
    
    if (c1 == c2) res = 0;
  }
  
  if (f1) fclose(f1);
  if (f2) fclose(f2);
  
  if (res) longjmp(test_fail,1);
  
  return;
}

void UTCheckFileEqual(char *fname, char *dname)
{
  chkfile(fname,dname,0);
  return; 
}

void UTCheckTextFileEqual(char *fname, char *dname)
{
  chkfile(fname,dname,1);
  return; 
}


FILE *UTfopen(char *fname, char *mode)
{
  FILE *f=NULL;
  
  f=fopen(fname,mode);
  
  if (NULL == f) {
    fprintf(stderr, "INTERNAL ERROR: can't open '%s' as '%s'\n",fname,mode);
    longjmp(test_fail,1);
  }
  return f;
}

#include "ut_tests.h"
int main(int argc, char **argv)
{
  volatile int from=0;
  volatile int to=UT_NUMTESTS;
  volatile int tests=0, failures=0;
  volatile char *err;

  if (argc > 1) {
    if (argv[1][0] == '-') {
      switch(argv[1][1]) {
        
        case 'l' : fprintf(stderr,"Tests list:\n");
                   while (from < UT_NUMTESTS) {
                     fprintf(stderr,"%4d %s\n",from+1,ut_tests[from].desc);
                     from++;
                   }
                   return 0;
                   
        default  : fprintf(stderr,"Usage: %s [-l] | [from to]\n",argv[0]);
                   return 1;
      }
    }
  }
#if 0      
  if ( 1 > from || from > UT_NUMTESTS) {
    fprintf(stderr, "INTERNAL ERROR:  ");
    return 1;
  }
#endif 
  do_test: 
  if (from < to) {
    if (ut_tests[from].func) {
      fprintf(stderr,"\nTEST #%d %s ...\n",from+1, ut_tests[from].desc);
      tests++;
      if (setjmp(test_fail) != 0) {
        failures++; 
      }
      else if ((err = ut_tests[from].func(from)) != NULL ) {
        failures++;
        fprintf(stderr,"FAIL: %s\n",err);
      }
      else {
        fprintf(stderr,"PASS!\n");
      }
    }
    from++;
    goto do_test;
  }
  
  fprintf(stderr,"\nTests executed/failed %d/%d\n",tests,failures);
  
  return 0;
}

