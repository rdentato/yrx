/* $Id: pattern.h,v 1.6 2003/11/02 00:03:37 gray Exp $ */

#ifndef PATTERN_H
#define PATTERN_H

#include "cstream.h"

typedef struct patterns_struct * Patterns;

typedef struct domain_struct * Domain;

Domain get_domain( const char* name );

int read_patterns ( CIStream s, const char* default_domain, boolean undef );

typedef struct goal_state* Goal;

/* Read from `in', translate according to `p', and write the result
   to `out', until the next characters to be read matches `goal'.
   Returns TRUE on success, or FALSE if the translation fails or if
   end-of-file is reached before the goal string is found.  */
boolean translate ( CIStream in, Domain d, COStream out, Goal goal_info );

extern boolean line_mode;
extern boolean token_mode;

extern boolean discard_unmatched;

extern boolean case_insensitive;

extern char* idchars;
extern char* filechars;

/* maximum length of a "*" argument: */
extern int MAX_ARG_LEN;

extern boolean debug_switch;
extern boolean trace_switch;

extern boolean keep_going;

extern boolean ignore_whitespace;

extern COStream output_stream;

void pattern_help( FILE* f ); /* write help info to f */

void initialize_syntax(void);

boolean set_syntax( int type, const char* char_set );

#ifdef TRACE
/* Compile with -DTRACE to enable use of the -trace option */
extern boolean trace_switch;
extern int trace_indent;
struct trace_enter_struct
{ int level; long line; int column; int ch; int domain; };
extern struct trace_enter_struct trace_enter;
#else
#define trace_switch FALSE
#endif

#if 0
#define Arg_Delim '\0'
#endif

#endif
