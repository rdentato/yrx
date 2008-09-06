:                        /\   __ ____ __   __ 
:                        \ \ / // __ \\ \ / / 
:                         \ / // /_/ / \ / /  
:                          \ // _  _/   \ \   
:                         / // / \ \__ / / \  
:                        /_// /   \__//_/ \ \ 
:                           \/             \/  
:   (C) 2006 by Remo Dentato (rdentato@users.sourceforge.net)
                 
= License
=========

  Permission to use, copy, modify and distribute this code and
its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice, or equivalent attribution
acknowledgement, appears in all copies and supporting documentation.
 
  Copyright holder makes no representations about the suitability
of this software for any purpose. It is provided "as is" without
express or implied warranty.

= Building yrx
==============

  The |src| directory contains a makefile to be used with GNU
make. It has been tested under Linux and Mingw+Msys (for Windows).

  If you are using a different compiler, it should be rather easy for
you to rebuild the libraries according your needs. See the |makefile|
in the src directories for details on what to inlcude/link.

= INSTALL
=========

  - Uncompress the tar file in a directory (say |~|):
      :  ~> tar xvzf yrx_060219.tgz
            
  - Change directory to the newly created |yrx| directory:
      :  ~> cd yrx
      :  ~/yrx>
      
  - Configure for your system (use |auto| to make a guess):
      :  ~/yrx> . configure linux
  
  - Compile in the |src| directory:
      :  ~/yrx> cd src
      :  ~/yrx/src> make
      
= Contacts
==========

  The homepage for yrx is on <SourceForge|http://yrx.sourceforge.net>.
  
  You can contact me via <email|rdentato@users.sourceforge.net>.
  
