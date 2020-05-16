/* Definitions for rtems targeting a SPARC using a.out.
   Copyright (C) 1996, 1997, 2000, 2002 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (joel@OARcorp.com).

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef LIB_SPEC_LEONMORE
#define LIB_SPEC_LEONMORE ""
#endif

/* bundle libraries */
#ifndef LIB_SPEC_KEEP
# undef LIB_SPEC
# define LIB_SPEC \
"%{!nostdlib: --start-group -lc -lgcc " LIB_SPEC_LEONMORE  " -lleonbare --end-group} "
#endif

/*
  -qsvt flag   : switch to single vector trapping dispatcher traptable
  -qsmall flag : link in locore_atexit.o that defines a simple atexit (mo malloc) to
                 avoid linking in clib's malloc referencing stuff
*/		

#ifndef STARTFILE_SPEC_KEEP
# undef STARTFILE_SPEC
# define STARTFILE_SPEC \
"%{!qsvt: locore_mvt.o%s} %{qsvt: locore_svt.o%s} %{!qprom: crt0.o%s} %{qprom: %{qprom2: %{qprom2ecos: crt_cpdataecos.o%s} %{!qprom2ecos: crt_cpdata.o%s}  } %{!qprom2: crt0_noatexit.o%s } }   crti.o%s %{!qprom: %{!qnocrtbegin: crtbegin.o%s } }  %{qprom: %{qprom2: %{!qprom2ecos: %{!qnocrtbegin: crtbegin.o%s } } } prominit.o%s %{qsvt: prominit_svt.o%s} } %{qprommp: prominit_mp.o%s} %{!qnoambapp: pnpinit.o%s} %{qnoambapp: pnpinit_simple.o%s} %{qfix-tn0018: tn0018yes.o%s}"
#endif

/*
  "%{!qprom: %{!qsvt: locore_mvt.o%s} %{qsvt: locore_svt.o%s} crt0.o%s crti.o%s crtbegin.o%s } \
 %{qprom: %{qprom2: %{qw: promcore2_qw.o%s} %{!qw: promcore2.o%s} crti.o%s crtbegin.o%s } \
 %{!qprom2: %{qw: promcore3_qw.o%s} %{!qw: promcore3.o%s}}}"
*/
