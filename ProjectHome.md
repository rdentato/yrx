# News #

  * **09 Sep 2008**: New source tarball and Windows pre-compiled executables. Checked with valgrind for memory leak. Tests are still to be fixed!

  * **25 Aug 2008**: Back to the NFA approach! Also, now every modifier is greedy. The expressions like "`.+z`" or "`\d+3`" will never succeed as the first term of the expression will eat all the available characters. Current version under SVN supports:
    * Captures (up to 8)
    * Back reference to captures (e.g. "`{a+)b\1`" matches _aba_, _aabaa_, etc )
    * Negated expressions that match 0 characters of the input.

  * **07 Feb 2008**: Stuck on an issue on the determinization step. The algorithm described in the PDF and implemented in the code succeedn on (a`*`)(a)a but fails on (`[`ab`]*`)(a)(`[`ab`]*`). I devised a different algorithm, closely following the standard subset construction, that succeeds on (`[`ab`]*`)(a)(`[`ab`]*`) but, alas, fails on (a`*`)(a)a !  I guess I have to take a step back an rethink the whole thing.  For those interested, the problem is to determine when a set of states needs to be merged again, even if they've been already, because of a different tags assignment.  Any suggestion is greatly welcome!

  * 04 Feb 2008: New tarball and Windows binary. This version produces C code! Use "sh mktest 'expr1' 'expr2' ..."  to create a test program named x (or x.exe). Requires gcc.

  * 31 Jan 2008: New tarball and Windows binary. Able to generate ASM-like code. Also performs some optimization.

  * 25 Jan 2008: New tarball and Windows binary.

  * 21 Jan 2008: Discussion group and SVN commits group created

  * 20 Jan 2008: Posted a request for comment about the YRX approach on comp.compilers http://compilers.iecc.com/comparch/article/08-01-051

# About YRX #
YRX is a tool to ease the creation of lexical scanners similar to re2C.
It's in its early stage, any comment or feedback is appreciated.
