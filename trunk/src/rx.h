/* 
**
** = Licence
**
** (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
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

/*  = Programmer's manual
    ~~~~~~~~~~~~~~~~~~~~~
**    To us rx in your applications, you have to include this file
** (|rx.h|) and link against the rx library. See the <makefile>
** for an example on how to do it.
*/

#ifndef RX_H
#define RX_H

#include <stdio.h>
/*  If you need very long NFA you can define {RX_MAXNFA} to the desired
**  value *before* including |rx.h|.
*/
#ifndef RX_MAXNFA
#define RX_MAXNFA 1024
#endif

#define RX_MAXCAPT 8

/*   You don't really need to know {rx_result} internals. You will
** receive it from the {rx_exec} function if a match is successful
** and you will pass it to the other functions to get more information.
*/
typedef void *rx_result;

/*   Should you need a readable dump of the nfa, use these to functions.
*/
void  rx_dump_str (FILE *f,unsigned char * nfa);
void  rx_dump_asm (FILE *f,unsigned char * nfa);
void  rx_dump_num (FILE *f,unsigned char * nfa);
unsigned char *rx_step_asm (FILE *f, unsigned int cnt, unsigned char *nfa);


/*   Create a NFA from a regular expression. Note that nfa must be at 
** maximum {RX_MAXNFA} characters long.
**   Compiling is an expensive operations, consider caching nfa's rather
** then recompiling the regular expression every time. 
**   If anything goes wrong, it returns an error message.
*/
char *rx_compile     (const unsigned char *pat, unsigned char *nfa);

/*   Add a new expression to an existing NFA. */
char *rx_compile_add (const unsigned char *pat, unsigned char *nfa);

/* Use |rx_exec| to match a string against a compiled regular
** expression (a NFA).
*/
rx_result rx_exec    (const unsigned char * nfa, unsigned char *str);

/* To get the length of the nth capture use |rx_len()|. 
** Capture 0 is the entire matching text. Captures 1-8 are the ones 
** specified in the expression.
*/
int       rx_len     (rx_result rx, unsigned char n);

/* To get the beginning of the nth capture use |rx_start()|, it will return
** a pointer inside the input buffer, not a zero terminated string!
*/
char     *rx_start   (rx_result rx, unsigned char n);

/* To get the end of the nth capture use |rx_start()|, it will return
** a pointer inside the input buffer.
*/
char     *rx_end     (rx_result rx, unsigned char n);

/* Use |rx_copy()| to fill a buffer (that must be at least of rx_len()+1
** bytes) with a zero terminated copy of the nth capture. 
*/
char     *rx_ncpy    (rx_result rx, unsigned char n, unsigned char *s, 
                                                           unsigned short len);
#define rx_cpy(r,n,s) rx_ncpy(r,n,s,0xFFFF)

/* Execute a function for every match over a string. The matching results
** are passed as arguments to the string. If the string returns a non zero
** value, the iteration ends.
*/
int       rx_iterate (const unsigned char * nfa, unsigned char *str,
                                                         int (* f)(rx_result));
                                                         
/* returns which expression matched */
unsigned char rx_matched(rx_result rx);

/* writes a capture on file f */
#define rx_write(r,n,f) fwrite(rx_start(r,n),rx_len(r,n),1,f)

#ifdef TRACE
extern int rx_trace;
#define rx_trace_on()  (rx_trace=1)
#define rx_trace_off() (rx_trace=0)
#endif

#endif /* RX_H */

