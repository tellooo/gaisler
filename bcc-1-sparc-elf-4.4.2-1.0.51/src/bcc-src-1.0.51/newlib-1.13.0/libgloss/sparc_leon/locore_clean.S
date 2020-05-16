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
 
#include <asm-leon/head.h>
                
	.section .text
	/* ------- */
	.weak	_cleanregs_custom_weak
	.set	_cleanregs_custom_weak,_cleanregs_donothing
	/* ------- */
	.global _cleanregs_libgloss
	

_cleanregs_donothing:	
_cleanregs_libgloss:	
	retl
	 nop
	
	
#define NUMREGWINDOWS 8
	
//_cleanregs_libgloss:	
	 
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
! initialize regs with values	
//#define REGINIT	
#ifdef REGINIT
	
	mov      %psr, %g3
	mov      %wim, %g4
	mov      %sp, %g5
	mov      %fp, %g6
	mov      %o7, %g7
	mov      %g0, %wim
	set      0, %g1
	andn     %g3,0x1f,%l0
	mov      %l0,%psr
	nop;  nop; nop
	
3:	set     0x0001,%i0
	set     0x0101,%i1
	set     0x0201,%i2
	set     0x0301,%i3
	set     0x0401,%i4
	set     0x0501,%i5
	set     0x0601,%i6
	set     0x0701,%i7
	set     0x0801,%l0
	set     0x0901,%l1
	set     0x1001,%l2
	set     0x1101,%l3
	set     0x1201,%l4
	set     0x1301,%l5
	set     0x1401,%l6
	set     0x1501,%l7
	or      %g1,%i0,%i0
	or      %g1,%i1,%i1
	or      %g1,%i2,%i2
	or      %g1,%i3,%i3
	or      %g1,%i4,%i4
	or      %g1,%i5,%i5
	or      %g1,%i6,%i6
	or      %g1,%i7,%i7
	or      %g1,%l0,%l0
	or      %g1,%l1,%l1
	or      %g1,%l2,%l2
	or      %g1,%l3,%l3
	or      %g1,%l4,%l4
	or      %g1,%l5,%l5
	or      %g1,%l6,%l6
	or      %g1,%l7,%l7
	restore
	set 0x10000,%g2
	add %g1,%g2,%g1
	set NUMREGWINDOWS*0x10000,%g2
	cmp %g1,%g2
	bne 3b
	 nop
	
	mov      %g4,%wim
	nop; nop; nop;
	mov      %g3,%psr 
	nop; nop; nop;
	mov      %g5, %sp 
	mov      %g6, %fp 
	
#endif
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

	jmpl %g7+8,%g0
	nop

