
/* generalized macro processor */

/* $Id: gema.c,v 1.14 2001/12/15 20:22:13 gray Exp $ */

/*********************************************************************
  This file is part of "gema", the general-purpose macro translator,
  written by David N. Gray <dgray@acm.org> in 1994 and 1995.
  You may do whatever you like with this, so long as you retain
  an acknowledgment of the original source.
 *********************************************************************/

/*
 * $Log: gema.c,v $
 * Revision 1.14  2001/12/15 20:22:13  gray
 * Add new convenience option "-ml" for using "[...]" instead of "<...>"
 * with HTML, XML, etc.
 *
 * Revision 1.13  1995/10/01  23:29:44  gray
 * Fix to support MS-DOS wild card expansion with a full pathname.
 *
 * Revision 1.12  1995/09/29  05:42:18  gray
 * Fix MS-DOS version for input file specified as full pathname.
 *
 * Revision 1.11  1995/08/07  03:21:25  gray
 * Remove support for "/" options in MS-DOS because doesn't work right.
 *
 * Revision 1.10  1995/07/04  23:41:52  gray
 * Split `argv_rules' into 5 pieces to placate the MPW compiler.
 *
 * Revision 1.9 1995/06/12 02:59:42 gray
 * Add "-trace" option.  Fix "-f" to not undo binary mode set in file.
 *
 * Revision 1.8 1995/05/22 02:50:14 gray
 * Update expand_wildcard to work on Windows/NT.
 *
 * Revision 1.7 1995/05/08 03:15:15 gray
 * Add wild card expansion for MS-DOS
 */

#if defined(_QC) || defined(_MSC_VER)
#pragma check_stack(off)
#endif

#include "gema_glob.h"


static char argv_domain_name[] = "ARGV";

static void
do_args(char** argv) {
  /* process the arguments according to the ARGV pattern domain */
  char** ap;
  CIStream argsbuf;
  COStream outbuf;
  const char* remaining;
  boolean ok;

  outbuf = make_buffer_output_stream();
  cos_putch(outbuf, '\n');
  for ( ap = argv ; *ap != NULL ; ap++ ) {
    cos_puts(outbuf, *ap);
    cos_putch(outbuf, '\n');
  }
  argsbuf = convert_output_to_input( outbuf );
  outbuf = make_buffer_output_stream();
  ok = translate ( argsbuf, get_domain(argv_domain_name), outbuf, NULL );
  cis_close(argsbuf);
  argsbuf = convert_output_to_input( outbuf );
  remaining = cis_whole_string(argsbuf);
  while ( isspace(*remaining) )
    remaining++;
  if ( remaining[0] != '\0' ) {
    fprintf(stderr, "Unrecognized arguments:\n%s", remaining);
    ok = FALSE;
  }
  cis_close(argsbuf);
  if ( !ok )
    exit_status = EXS_ARG;
}

#ifdef MSDOS
/* for MS-DOS conventions, use case-insensitive comparison for options */
#define CI "\\C"
#else
#define CI
#endif

/* The following rules define how the command-line arguments will
   be processed. (They are split into several strings to accommodate the
   Macintosh MPW compiler, which does not allow strings longer than 512.) */
static char argv_rules1[] =
CI "\\N-h*\\n=@show-help@end\n"
   "\\A\\n\\Z=@err{@version\\N@show-help}@end\n"
CI "\\N-version\\n=@err{@version\\N}\n"
CI "\\N-f\\n*\\n=@set{.BINARY;@get-switch{b}}@set-switch{b;0}"
		"@define{@read{*}}@cmpn{${.BINARY;0};0;;;@set-switch{b;1}}\n"
CI "\\N-p\\n*\\n=@define{*}\n"
   "\\N-<L1>\\n=@set-switch{$1;1}\n"
CI "\\N-w\\n=@set-switch{w;1}@set-syntax{S;\\s\\t}\n"
CI "\\N-t\\n=@set-switch{w;1}@set-switch{t;1}\n";
static char argv_rules2[] =
CI "\\N-arglen\\n<D>\\n=@set-switch{arglen;$1}\n"
CI "\\N-idchars\\n*\\n=@set-parm{idchars;$1}\n"
CI "\\N-filechars\\n*\\n=@set-parm{filechars;$1}\n"
CI "\\N-literal\\n*\\n=@set-syntax{L;$1}\n"
#ifndef NDEBUG
CI "\\N-debug\\n=@set-switch{debug;1}\n"
#endif
#ifdef TRACE
CI "\\N-trace\\n=@set-switch{trace;1}\n"
#endif
CI "\\N-line\\n=@set-switch{line;1}\n"
CI "\\N-match\\n=@set-switch{match;1}\n"
CI "\\N-ml\\n=@set-syntax{</>LLL;[|]</>}\n" /* for Markup Language (XML,HTML)*/
#ifdef MSDOS
#if 0 /* doesn't work because causes "\Z" action to be invoked. */
   "\\N\\/<L1>\\n=@ARGV{-$1\\n}\n"	/* allow "/" instead of "-" */
#endif
#else
CI "\\N-n\\n=@set-switch{match;1}\n"	/* like for sed */
CI "\\N-e\\n*\\n=@define{*}\n"		/* like for sed */
#endif
;
static char argv_rules3[] =
CI "\\N-nobackup\\n=@set-parm{backup;}\n"
CI "\\N-backup\\n<G>\\n=@set-parm{backup;$1}\n"
CI "\\N-out\\n*\\n=@set{.OUT;$1}\n"
CI "\\N-in\\n*\\n=@set{.IN;$1}\n"
   "\\N\\L*\\=*\\n=@define{$0}\n"
   "\\N\\L\\@*\\n=@define{$0}\n"
CI "\\N-odir\\n*\\n=@set{.ODIR;*}\n"
CI "\\N-otyp\\n*\\n=@set{.OTYP;*}\n"
   "\\N-*\\n=@err{Unrecognized option:\\ \"-*\"\\n}@exit-status{3}\n"
   "\\n=\n"
#ifdef MSDOS
   /* On MS-DOS, file name wild cards have to be expanded by the
      program, unlike Unix where expansion is done by the shell. */
   "\\N*\\n=@ARGV-F{@expand-wild{*}}\n"
   "ARGV-F:*\\n=@ARGV-FILE{${.ODIR;}\\n${.OUT;}\\n${.IN;}\\n*}\n"
#else
   "\\N*\\n=@ARGV-FILE{${.ODIR;}\\n${.OUT;}\\n${.IN;}\\n*}\n"
#endif
   "\\Z=@ARGV-END{${.OUT;}\\n${.IN;}\\n${.ODIR;}\\n}\n";
static char argv_rules4[] =
"ARGV-FILE:\\n\\n\\n<U>=@set{.IN;$1};"
 "\\n\\n<U>\\n<U>=@set{.OUT;$2};"
 "\\n<U>\\n<U>\\n<U>=@err{More than two files specified.\\n}@exit-status{3};"
 "\\n<U>\\n*\\n<U>=@write{$1;@{@read{$3}}};"
 "<U>\\n\\n*\\n<U>=@bind{.OUT;@makepath{$1;@relpath{$3;$3};${.OTYP;}}}"
  "@write{${.OUT};@{@read{$3}}}@close{${.OUT}}@unbind{.OUT};"
 "<U>\\n<U>\\n=@err{Not meaningful\\:\\ both\\ -out\\ and\\ -odir\\n}"
	"@exit-status{3}@end\n";
static char argv_rules5[] =
"ARGV-END:\\n\\n<U>\\n=@end;" /* -odir was specified */
 "<U>\\n\\n\\n=@end;"
 "<U>\\n<U>\\n=@write{$1;@{@read{$2}}}@end;" /* output and input files */
 "\\n\\n\\n=@write{-;@{@read{-}}};" /* no files specified, use stdin/stdout */
 "\\n<U>\\n=@write{-;@{@read{$1}}}@end\n"
#ifndef NDEBUG
  "ARGV-FILE:*=@err{Internal failure in ARGV-FILE \"*\"\\n}\n"
  "ARGV-END:*=@err{Internal failure in ARGV-END \"*\"\\n}\n"
#endif
;

#ifdef LUA
static char argv_rules6[] =
CI "\\N-l\\n*\\n=@lua{arg=\\{\\}\\;dofile(\"@LESC{$1}\")}\n"
CI "\\N-lua\\n<ARGV-LS>\\n<ARGV-LA>=@lua{arg[0]=\"$1\"\\;dofile(\"$1\")}@end\n"
   "ARGV-LS:<G>=@LESC{$1}@lua{arg=\\{\\}\\;arg.n=0}@end\n"
   "ARGV-LA:\\A=@set{k;0}\n"
   "ARGV-LA:<LESC>\\n=@incr{k}@lua{arg[$k]=\"$1\"\\;arg.n=$k}\n"
   "LESC:\\\"=\\\\\\\"\n"
   "LESC:\\\\=\\\\\\\\\n"
;
#endif

#if 0
   "ARGV-LA:=@lua{arg.n=$k}@end\n"
   "ARGV-LA:\\Z=@lua{arg.n=$k}\n"
   "ARGV-LA:<S>=\n"
   "ARGV-LA:<G>=@incr{k}@lua{arg[$k]=\"$1\"}\n"
#endif

static void
load_rules( const char* argv_rules, size_t length ) {
  CIStream str;
  assert( length < 512 );
  str = make_string_input_stream(argv_rules, length, FALSE);
  read_patterns ( str, argv_domain_name, FALSE );
  cis_close(str);
}

#define LOAD_RULES(argv_rules) load_rules(argv_rules, sizeof(argv_rules)-1)

static void
initialize_argv_domain(void) {
  assert( (int)EXS_ARG == 3 ); /* to match "@exit-status" above */
  LOAD_RULES(argv_rules1);
#ifdef LUA
  LOAD_RULES(argv_rules6);
#endif
  LOAD_RULES(argv_rules2);
  LOAD_RULES(argv_rules3);
  LOAD_RULES(argv_rules4);
  LOAD_RULES(argv_rules5);
}

int main(int argc, char* argv[]){
  char** ap;
  initialize_syntax();
  ap = argv+1;
  stdout_stream = make_file_output_stream(stdout,"");
  stdin_stream = make_file_input_stream(stdin,NULL);
  output_stream = stdout_stream;
  if ( argc >= 3 && stricmp(*ap,"-prim") == 0 ) {
    CIStream ps;
    ap++;
    ps = open_input_file( *ap, FALSE );
    if ( ps != NULL ) {
      read_patterns(ps, "", FALSE);
      cis_close(ps);
    }
    ap++;
  }
  else initialize_argv_domain();
  do_args(ap);
  return (int)exit_status;
}

