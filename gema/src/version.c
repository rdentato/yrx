/* $Id: version.c,v 1.1.1.1 2003/11/07 13:16:03 rdentato Exp $ */

/*      Version identification string

	The prefix "@(#)" is searched for by the Unix "what" command.
	The characters following that will also be displayed by the 
	"-version" option or the "@version" function.

	If you make any changes to this program at all, please be
	sure to update this string also, so that your modified version
	can be distinguished from the original version.
 */

#ifndef LUA
const char what_string [] = "@(#)gema 1.4.1RC Mar 31, 2005";

const char author [] = "@(#) David N. Gray <DGray@acm.org>";
#else
const char what_string [] = "@(#)gel 1.0.1RC Mar 31, 2005";

const char author [] = "@(#) David N. Gray <DGray@acm.org>\n"\
                       "     Remo Dentato <rdentato@users.sourceforge.net>";
#endif
