/*

    LEON2/3 LIBIO low-level routines 
    Written by Jiri Gaisler.
    Copyright (C) 2004  Gaisler Research AB

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * locore.s for LEON
 */
 
/* The traptable has to be the first code in a boot PROM. */

#include <asm-leon/head.h>
                
  .seg    "text"

  .global _trap_table, _start_svt_weak
  .global start
  
  /* Hardware traps */
	/* svt code asumes that %g6 is never used in the code */

start:
_trap_table:

	sethi %hi(_start_svt_weak), %g6
	jmp %g6+%lo(_start_svt_weak)
	 nop	

