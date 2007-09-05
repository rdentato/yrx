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


#ifndef HUL_H
#define HUL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <stdarg.h>
#include <setjmp.h>

#ifndef HUL_MAIN
#define HUL_EXTERN extern
#else
#define HUL_EXTERN
#endif


#define DBG_OFF  3
#define DBG_ERR  2
#define DBG_WARN 1
#define DBG_MSG  0

#ifdef DEBUG

HUL_EXTERN uint8_t dbg_lvl;

#define dbgprintf(n,fmt,...) \
               (dbg_lvl <= n? fprintf(stderr,fmt,__VA_ARGS__),fflush(stderr): 0)
#define dbglvl(n) (dbg_lvl = n)

#else

#define dbgprintf(n,fmt,...)
#define dbglvl(n)

#endif /* DEBUG */

HUL_EXTERN jmp_buf *errjmpptr;

/* Print a message and exit */
#define err(...) \
               (fprintf(stderr,__VA_ARGS__),\
                errjmpptr != NULL? longjmp(*errjmpptr,1) :\
                exit(1))

#define errjmp(j) (errjmpptr = &j)
               
#endif /* HUL_H */

