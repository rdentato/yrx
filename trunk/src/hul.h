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

#define dbgprintf(n,fmt,...) ( (dbg_lvl <= n)\
                                  ? fprintf(stderr,fmt,__VA_ARGS__),fflush(stderr)\
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
#define err(...) \
               (fprintf(stderr,__VA_ARGS__),\
                errjmpptr != NULL\
                  ? longjmp(*errjmpptr,1) \
                  : exit(1))

#define errjmp(j) (errjmpptr = &j)
               
#endif /* HUL_H */

