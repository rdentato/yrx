/* Creative Commons Right (BY:) 2006 by Remo Dentato <rdentato@users.sourceforge.net>
** This code is in the licensed accord (BY:)

http://creativecommons.org/licenses/by/2.5/
*/

#ifndef UT_H
#define UT_H

#define TST(s,x) (TSTRES = (x), TSTTOT++,\
                  fprintf(stderr, ".%03d %s - %s\n",\
                          ++TSTNUM, TSTRES?(TSTPASS++,TSTOK):TSTKO, s), \
                  fflush(stderr),TSTRES)

#define TSTGROUP(s) (TSTNUM=0, \
                     fprintf(stderr,"\n#%d %s\n", ++TSTGRP, s),fflush(stderr),TSTGRP)

#define TSTSTAT() (fprintf(stderr,"\nPASSED: %d/%d\n",TSTPASS,TSTTOT),fflush(stderr))

#define TSTHDR(s) (TSTGRP = 0, TSTTOT?TSTSTAT():0, TSTTOT = 0, TSTPASS=0, \
                    fprintf(stderr,"\n:: %s\n",s),fflush(stderr))


static int TSTNUM  = 0;
static int TSTGRP  = 0;
static int TSTRES  = 0;
static int TSTTOT  = 0;
static int TSTPASS = 0;

static const char *TSTOK = " PASS";
static const char *TSTKO = "*FAIL";

#endif

