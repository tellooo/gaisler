/* Definitions for LEON running on Cobham Gaisler BCC toolchain
   Copyright (C) 2016 Free Software Foundation, Inc.
   Contributed by Martin Aberg (maberg@gaisler.com).

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

/* Target OS builtins.  */
#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()		\
  do						\
    {						\
	if (sparc_fix_b2bst)			\
	  builtin_define ("__FIX_LEON3FT_B2BST"); \
    }						\
  while (0)

#undef CPP_SUBTARGET_SPEC
#define CPP_SUBTARGET_SPEC \
" \
%{qnano: -isystem %R/include/newlib-nano} \
"

/* Allow user to specify a BSP not in the default location. */
#undef DRIVER_SELF_SPECS
#define DRIVER_SELF_SPECS \
" \
%{!B: \
  %{qbsp=*:-B %R/bsp/%*} \
  %{!qbsp*:-B %R/bsp/leon3} \
} \
"

#define BCC_STARTFILE_SPEC \
 "%{qfix-tn0018|mfix-gr712rc|mfix-ut699|mfix-ut700: " \
  "%{!qsvt: trap_table_mvt_tn0018.S.o%s; :trap_table_svt_tn0018.S.o%s}" \
 "; :" \
  "%{!qsvt: trap_table_mvt.S.o%s; :trap_table_svt.S.o%s}" \
 "} "

#undef STARTFILE_SPEC
#define STARTFILE_SPEC \
BCC_STARTFILE_SPEC \
" \
crt0.S.o%s crti.o%s crtbegin.o%s \
"

#undef LIB_SPEC
#define LIB_SPEC \
" \
--start-group \
-lbcc \
-latomic \
%{!qnano: -lc} \
%{qnano: -lc_nano} \
--end-group \
"

/* Use the default */
#undef LINK_GCC_C_SEQUENCE_SPEC
#define LINK_GCC_C_SEQUENCE_SPEC \
" \
%{!T*: -T linkcmds%s} \
%G %L %G \
"

