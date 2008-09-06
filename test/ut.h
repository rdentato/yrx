/*  UT - ut.h
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

#ifndef UT_H
#define UT_H

void UTCheckFileEqual(char *fname, char *dname);
void UTCheckTextFileEqual(char *fname, char *dname);
void UTStringsEqual(char *stra, char *strb);
void UTPointersEqual(void *ptra, void *ptrb);
void UTUnsignedEqual(unsigned numa, unsigned numb);
void UTSignedEqual(signed numa, signed numb);

FILE *UTfopen(char *fname, char *mode);

typedef struct {
  char *(*func)(int);
  char *desc;
} ut_test;

#define TESTCASE(_n,_m) char *ut_##_n(int testnum) {
#define _TESTCASE(_n,_m) TESTCASE(_n,_m)
#define TESTRESULT(_e)  return _e;}

#endif

