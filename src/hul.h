/*  
**  (C) 2007 by Remo Dentato (rdentato@users.sourceforge.net)
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
*/

/* My Hopefully Useful Library */

#ifndef HUL_H
#define HUL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <stdarg.h>
#include <setjmp.h>
#include <errno.h>

/* Using HUL */

#ifndef HUL_MAIN
#define HUL_EXTERN extern
#else
#define HUL_EXTERN
#endif


/* Debugging trace 

   Despite the availability of debuggers, sometimes it's useful to insert
 a couple of |printf()| to trace what's going on in a program. Those 
 statements are normally guarded with an |#ifdef DEBUG| so that they disappear
 in the release version of the software.
   The downside is that they visually clutter the source code not to mention
 that you might want to have only some of them printing their message and 
 disable the others.
   The macros here help inserting debug messages that can be turned on/off
 and that will be completely removed in the release code. If the source is
 compiled without having the |DEBUG| defined, the macros will expand to nothing.
 
 [|dbglvl(n)|] Messages are grouped in /levels/ so that you may select the ones
               that are more important for you at a given point in time. This
               macros sets the current level of debugging wanted:
               
               [|DBG_NONE|] No message will be printed
               [|DBG_ERR|]  Only messages that will reveal errors
               [|DBG_WARN|] Print also warning messages
               [|DBG_ANY|]  Print any message
 
 [|dbgprintf(n,fmt,...)|] Works as a |fprintf(stderr,fmt, ...)|.
               The first parameter |n| defines the type of the message:
               
               [|DBG_ERR|]  A critical message revealing a serious error
               [|DBG_WARN|] A warning messages, probably an unexpected
                            situation that may be recovered.
               [|DBG_MSG|]  A generic message informing on the state of the
                            program.
               [|DBG_OFF|]  A message that has been disabled. This is useful
                            when you want to switch off a given trace message
                            regardless the current debug level.
*/

#define DBG_OFF   -1
#define DBG_NONE  3
#define DBG_ERR   2
#define DBG_WARN  1
#define DBG_MSG   0
#define DBG_ANY   0

#ifdef DEBUG

HUL_EXTERN int8_t dbg_lvl;

/* To ensure that dbg messages are correctly interleaved with other 
   messages that might have been printed, |stdout| is flushed before
   printing and |stderr| is flushed right after.
*/
#define dbgprintf(n,...) ( (dbg_lvl <= n)\
                              ? fflush(stdout),\
                                fprintf(stderr,__VA_ARGS__),\
                                fflush(stderr)\
                              : 0)
#define dbglvl(n) (dbg_lvl = n)

#else

#define dbgprintf(n,fmt,...)
#define dbglvl(n)

#endif /* DEBUG */

/* Error Handling */
/* The |err()| macros works as a |fprintf()| on |stderr| and then
   does a longjump() (if a jmp target has been set) or exit().
   
   To properly release resources you should register a cleanup
   function with |atexit()|

*/

HUL_EXTERN jmp_buf *errjmpptr;

/* Print a message and exit */
#define err(errnum,...) \
               (errno = errnum,\
                fprintf(stderr,"[%d] ",errno),\
                fprintf(stderr,__VA_ARGS__),\
                errjmpptr != NULL\
                  ? longjmp(*errjmpptr,1) \
                  : exit(1))

#define errjmp(j) (errjmpptr = &j)
               

/* Unit Test */
/*
   These macros are intended to be used when writing unit tests. They
   are available only if the symbol |HUL_UT| is defined before including
   the |hul.h| header.
*/
#ifdef HUL_UT

/* Test are divided in sections introduced by the |TSTHDR(s)| macro. 
   The macro reset the appropriate counters and print the header |s|
*/

#define TSTHDR(s) (TSTGRP = 0,\
                   TSTTOT \
                     ? TSTSTAT() \
                     : 0, TSTTOT = 0, TSTPASS=0, \
                       fprintf(stderr,"\n:: %s\n",s),fflush(stderr))

/* In each session, tests can be logically grouped so that different aspects
   of related functions can be tested.
*/

#define TSTGROUP(s) (TSTNUM=0, \
                     fprintf(stderr, "\n#%d %s\n",\
                                     ++TSTGRP, s),\
                                     fflush(stderr), TSTGRP)

/* The single test is defined  with the |TST(s,x)| macro.
     |s| is a string that defines the test
     |x| a boolean expression that is to be true for the test to succeed.
*/

#define TST(s,x) (TST_DO(s,x), TST_END)

/* With |TSTW(s,x,...)| you can setup a warning to be printed if the test fails
   so to explain better why the test failed.
     TSTW("Zero(x)", (x==0), "x is %d instead of 0",x)
*/                  
#define TSTW(s,x,...) (TST_DO(s,x),(TSTRES ? 0 : fprintf(stderr,__VA_ARGS__)) , TST_END)


#define TST_DO(s,x) (TSTRES = (x), TSTTOT++,\
                  fprintf(stderr, ".%03d %s - %s",\
                          ++TSTNUM, TSTRES?(TSTPASS++,TSTOK):TSTKO, s))

#define TST_END fputc('\n',stderr), fflush(stderr),TSTRES



/* At the end of a section, the accumulated stats can be printed out
*/
#define TSTSTAT() (fprintf(stderr,"\nPASSED: %d/%d\n",TSTPASS,TSTTOT),fflush(stderr))

#define TSTDESC(x) 

static int TSTRES  = 0;  /* Result of the last performed |TST()| */
static int TSTNUM  = 0;  /* Last test number */
static int TSTGRP  = 0;  /* Current test group */
static int TSTTOT  = 0;  /* Number of tests executed */
static int TSTPASS = 0;  /* Number of passed tests */

static const char *TSTOK = " PASS";
static const char *TSTKO = "*FAIL";

#endif

#endif /* HUL_H */

