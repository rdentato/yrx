/* RX - rx.h
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
** (|rx.h|) and link against the rx library. See the <makefile|makefile>
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

/*   You don't really need to know how {rx_result} internals. You will
** receive it from the {rx_exec} function if a match is successful
** and you will pass it to the other functions to get more information.
*/
typedef void *rx_result;

/*   Should you need a readable dump of the nfa, use these to functions.
*/
void       rx_hexdump (FILE *f,unsigned char * nfa);
void       rx_symdump (FILE *f,unsigned char * nfa);

/*   Create a NFA from a regular expression. Note that nfa must be at 
** least {RX_MAXNFA} characters long.
**   Compiling is an expensive operations, consider caching nfa's rather
** then recompiling the regular expression every time. 
**   If anything goes wrong, it returns an error message.
*/
char      *rx_compile (const unsigned char *pat, unsigned char * nfa);

/* Use |rx_exec| to match a string against a compiled regular
** expression (a NFA).
*/
rx_result rx_exec    (const unsigned char * nfa, unsigned char *str);

/* To get the length of the nth capture use |rx_len()|. Remember, 
** there are 8 captures at maximum.
*/
int       rx_len     (rx_result rx, unsigned char n);

/* To get the beginning of the nth capture use |rx_start()|, it will return
** a pointer inside the string, not a zero terminated string!
*/
char     *rx_start   (rx_result rx, unsigned char n);

/* To get the end of the nth capture use |rx_start()|, it will return
** a pointer inside the string!
*/
char     *rx_end     (rx_result rx, unsigned char n);

/* Use |rx_copy()| to fill a buffer (that must be at least of rx_len()+1
** bytes) with a zero terminated copy of the nth capture. 
*/
char     *rx_cpy     (rx_result rx, unsigned char n, unsigned char *s);

/* Execute a function for every match over a string. The matching results
** are passed as arguments to the string. If the string returns a non zero
** value, the iteration ends.
*/
int       rx_iterate (const unsigned char * nfa, unsigned char *str,
                                                         int (* f)(rx_result));

unsigned char rx_matched(rx_result rx);

char *rx_compile_add(const unsigned char *pat, unsigned char *nfa);

#define RX_CASE(n,s) case n 

/* == Finite State Machines
**
**   These macros provide a simple mechanism for defining
** finite state machines (FSM).
**
**   Each state containse a block of instructions:
**
** | STATE(state_name) {
** |   ... C instructions ...
** | }
**
**   To move from a state to another you use the GOTO macro:
**
** | if (c == '*') GOTO(stars);
**
** or, in case of an error) the FAIL(x) macro:
**
** | if (c == '?') FAIL(404);  ... 404 is an error code
**
**   There must be two special states states: ON_FAIL and ON_END
**
*/

#define FSM       
#define ENDFSM    
#define STATE(x)  x##_: if (0) goto x##_;
#define NEXT(x)   goto x##_



#endif /* RX_H */

