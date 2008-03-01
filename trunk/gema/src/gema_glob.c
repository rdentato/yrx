/* generalized macro processor */

/* $Id $ */

/*********************************************************************
  This file is part of "gema", the general-purpose macro translator,
  written by David N. Gray <dgray@acm.org> in 1994 and 1995.
  You may do whatever you like with this, so long as you retain
  an acknowledgment of the original source.
 *********************************************************************/

#include "gema_glob.h"

#ifndef GEMA_USAGE

#define GEMA_USAGE 	  "Syntax: gema [ -p <patterns> ] [ -f <pattern-file> ] [ <in-file> <out-file> ]\n"\
                      "Copies standard input to standard output, performing\n"\
	                  "text substitutions specified by a series of patterns\n"\
                      "specified in -p options or in files specified by "\
                      "-f options.\n"
#endif


COStream output_stream;
COStream stdout_stream;
CIStream stdin_stream;

boolean keep_going = FALSE;
boolean binary = FALSE;
Exit_States exit_status = EXS_OK;

void usage(void) {
  fprintf(stderr, GEMA_USAGE);
  pattern_help( stderr );
}

static struct switches {
  const char* name;
  boolean * var;
} switch_table[] =
  { { "line", &line_mode },
    { "b", &binary },
    { "k", &keep_going },
    { "match", &discard_unmatched },
    { "i", &case_insensitive },
    { "w", &ignore_whitespace },
    { "t", &token_mode },
    { "arglen", &MAX_ARG_LEN },
#ifndef NDEBUG
    { "debug", &debug_switch },
#endif
#ifdef TRACE
    { "trace", &trace_switch },
#endif
    { NULL, NULL } };

int*
find_switch(const char* arg) {
  /* given a switch name, return a pointer to the value. */
  struct switches *p;
  for ( p = &switch_table[0] ; p->name != NULL ; p++ )
    if ( stricmp(arg, p->name)==0 )
      return p->var;
  return NULL;
}

static struct parms {
  const char* name;
  char ** var;
} parm_table[] =
  { { "idchars", &idchars },
    { "filechars", &filechars },
    { "backup", &backup_suffix },
    { NULL, NULL } };

boolean
set_parm(const char* name, const char* value) {
  struct parms *p;
  for ( p = &parm_table[0] ; p->name != NULL ; p++ )
    if ( stricmp(name, p->name)==0 ) {
      char** vp = p->var;
#if 0 /* don't bother with this to avoid copying the initial value. */
      if ( *vp != NULL )
	free(*vp);
#endif
      *vp = str_dup(value);
      return TRUE;
    }
  return FALSE;
}

#ifdef MSDOS
#ifdef __TURBOC__
#include <dir.h>
#endif
#ifdef _WIN32
#include <io.h>
#else
#include <dos.h>
#endif
#endif /* MSDOS */

void
expand_wildcard ( const char* file_spec, COStream out ) {
/*  Expand wild card file name on MS-DOS.
    (On Unix, this is not needed because expansion is done by the shell.)
 */
#ifdef MSDOS						       
#if defined(_FIND_T_DEFINED)  /* Microsoft C on MS-DOS */
  struct _find_t fblk;
  if ( _dos_findfirst( file_spec, _A_NORMAL|_A_ARCH|_A_RDONLY, &fblk )
       == 0 ) {
    /* first match found */
    do {
      merge_pathnames( out, FALSE, file_spec, fblk.name, NULL );
      cos_putch( out, '\n' );
    }
    while ( _dos_findnext( &fblk ) == 0 );
  }
  else
#elif defined(_WIN32)  /* Microsoft C/C++ on Windows NT */
  struct _finddata_t fblk;
  long handle;
  handle = _findfirst( (char*)file_spec, &fblk );
  if ( handle != -1 ) {
    /* first match found */
    do {
      if ( !(fblk.attrib & _A_SUBDIR) ) {		   
	merge_pathnames( out, FALSE, file_spec, fblk.name, NULL );
	cos_putch( out, '\n' );
      }
    }
    while ( _findnext( handle, &fblk ) == 0 );
    _findclose( handle );
  }
  else
#elif defined(__TURBOC__)  /* Borland Turbo C */
  struct ffblk fblk;
  if ( findfirst( file_spec, &fblk, FA_ARCH|FA_RDONLY ) == 0 ) {
    /* first match found */
    do {
      merge_pathnames( out, FALSE, file_spec, fblk.ff_name, NULL );
      cos_putch( out, '\n' );
    }
    while ( findnext( &fblk ) == 0 );
  }
  else
#endif /* Borland */
  if ( strchr(file_spec,'*') != NULL )
    fprintf( stderr, "No match for \"%s\"\n", file_spec );
  else
#endif /* MSDOS */
  {
    cos_puts( out, file_spec );
    cos_putch( out, '\n' );
  } 
}
