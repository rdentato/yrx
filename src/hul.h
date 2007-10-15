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

/* My Hopefully Useful Library (requires a ANSI C99 compiler) */

#ifndef HUL_H
#define HUL_H

#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
/* MSVC does not have stdint!!! Paul Hsie graciously created one */
#include "pstdint.h"
#else
#include <stdint.h>
#endif

#include <string.h>
#include <stdarg.h>

/* Using HUL as unit test, implies |DEBUG| */
#ifdef HUL_UT
#ifndef DEBUG
#define DEBUG
#endif
#endif

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

#define DBG_OFF    -1
#define DBG_ERR     2
#define DBG_WRN     1
#define DBG_MSG     0

/* To be used in |dbglvl()| */
#define DBG_NOMSG   3
#define DBG_ANYMSG  0

#ifdef DEBUG

HUL_EXTERN int8_t dbg_lvl;

/* To ensure that dbg messages are correctly interleaved with other
   messages that might have been printed, |stdout| is flushed before
   printing and |stderr| is flushed right after.
*/

#define dbgprintf(n,...) ( (dbg_lvl <= n)\
                              ? (fflush(stdout), \
                                fprintf(stderr,"# "),\
                                fprintf(stderr,__VA_ARGS__),\
                                fflush(stderr)) \
                              : 0)
#define dbglvl(n) (dbg_lvl = n)



#else

#define dbgprintf(n,...)
#define dbglvl(n)

#endif /* DEBUG */

#define dbgmsg(...) dbgprintf(DBG_MSG,__VA_ARGS__)
#define dbgwrn(...) dbgprintf(DBG_WRN,__VA_ARGS__)
#define dbgerr(...) dbgprintf(DBG_ERR,__VA_ARGS__)
#define dbgoff(...) dbgprintf(DBG_OFF,__VA_ARGS__)
#define dbgwrt(...) dbgprintf(DBG_NOMSG,__VA_ARGS__)


/* Error Handling */
/* The |err()| macros does a |fprintf()| on |stderr| and then |exit()|.

   To properly release resources you should register a cleanup
   function with |atexit()|
*/

/* Print a message and exit */
#define err(errnum,...)  (fflush(stdout),\
                          fprintf(stderr,"[%d] ",errnum),\
                          fprintf(stderr,__VA_ARGS__),\
                          exit(errnum))

/* Unit Test */
/*
   These macros are intended to be used when writing unit tests. They
   are available only if the symbol |HUL_UT| is defined before including
   the |hul.h| header.
   The log created is TAP compatible (see: <http://testanything.org>).
*/

#ifdef HUL_UT

#define TSTWRITE(...) (fprintf(stderr,__VA_ARGS__),fflush(stderr))


/* Tests are divided in sections introduced by the |TSTHDR(s)| macro.
   The macro reset the appropriate counters and print the header |s|
*/

#define TSTSECTION(s) (TSTSTAT(), TSTGRP = 0, TSTSEC++, TSTPASS=0, \
                       TSTWRITE("#\n# %d. %s\n",TSTSEC, s))

/* In each session, tests can be logically grouped so that different aspects
   of related functions can be tested.
*/

#define TSTGROUP(s) (TSTNUM=0, \
                     TSTWRITE("#\n#   %d.%d %s\n", TSTSEC, ++TSTGRP, s),\
                     TSTGRP)

/* The single test is defined  with the |TST(s,x)| macro.
     |s| is a string that defines the test
     |x| a boolean expression that is to be true for the test to succeed.
*/

#define TST(s,x)    (TST_DO(s,(TSTSKP != NULL? 1 : x)),\
                     (TSTSKP != NULL? TSTWRITE(" # SKIP %s",TSTSKP):0),\
                     TSTWRITE("\n"),TSTRES)
/*
#define TST_DO(s,x) (TSTRES = (x), TSTGTT++, TSTTOT++, TSTNUM++,\
                     TSTWRITE("%s %4d - $%02d%02d%03d %s",\
                              (TSTRES? (TSTGPAS++,TSTPASS++,TSTOK) : TSTKO),\
                              TSTGTT, TSTSEC, TSTGRP, TSTNUM, s))

*/
#define TST_DO(s,x) (TSTRES = (x), TSTGTT++, TSTTOT++, TSTNUM++,\
                     TSTWRITE("%s %4d - %s",\
                              (TSTRES? (TSTGPAS++,TSTPASS++,TSTOK) : TSTKO),\
                              TSTGTT, s))

#define TSTT(s,x,r) (TST_DO(s,x), TSTWRITE(" # TODO %s\n",r), TSTRES)

#define TSTSKIP(x,r)(TSTSKP = ((x)? r : NULL))
#define TSTENDSKIP  (TSTSKP = NULL)


#define TSTNOTE(...) (TSTWRITE("#     "), TSTWRITE(__VA_ARGS__), TSTWRITE("\n"))

#define TSTONFAIL(...) (TSTRES? 0 : (TSTNOTE(__VA_ARGS__)))

/* At the end of a section, the accumulated stats can be printed out
*/
#define TSTSTAT() (TSTTOT == 0 ? 0 : (\
                   TSTWRITE("#\n# SECTION %d PASSED: %d/%d\n",TSTSEC,TSTPASS,TSTTOT),\
                   TSTTOT = 0))

#define TSTDONE() (TSTSTAT(), \
                   TSTWRITE("#\n# TOTAL PASSED: %d/%d\n",TSTGPAS,TSTGTT),\
                   TSTWRITE("#\n# END OF TESTS\n1..%d\n",TSTGTT),fflush(stderr))


static int TSTRES  = 0;  /* Result of the last performed |TST()| */
static int TSTNUM  = 0;  /* Last test number */
static int TSTGRP  = 0;  /* Current test group */
static int TSTSEC  = 0;  /* Current test SECTION*/
static int TSTTOT  = 0;  /* Number of tests executed */
static int TSTGTT  = 0;  /* Number of tests executed (Grand Total) */
static int TSTGPAS = 0;  /* Number of tests passed (Grand Total) */
static int TSTPASS = 0;  /* Number of passed tests */

static char *TSTSKP = NULL;
static const char *TSTOK = "ok    ";
static const char *TSTKO = "not ok";

#endif

#endif /* HUL_H */

