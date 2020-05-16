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
 * locore_var.S for LEON
 */

	
	.section .text
	/* ------- */
	.weak	_hardreset_custom_weak
	.set	_hardreset_custom_weak,_hardreset_custom_weak_dummy
	/* ------- */
	.global _nwindows, _leon_version, _nwindows_min1
	
_hardreset_custom_weak_dummy:

!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
! get nwindows and leon version
			
	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon3:	3
	bne	1f
	nop
	set     _leon_version,%l0
	set     3,%l1
	st      %l1,[%l0]
	mov	%asr17, %g5		! leon3 has nwindows in %asr17
	ba	2f
1:
	/* other version */
2:
	and	%g5, 0x1f, %g5
	set	_nwindows_min1, %l3
	st	%g5, [%l3]
	add     %g5,1,%g5
	set	_nwindows, %l3
	st	%g5, [%l3]
	set	_nwindows_min2, %l3
	sub     %g5,2,%g5
	st	%g5, [%l3]
	
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	
	retl
	nop

!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	.section .data
	.global _nwindows, _leon_version, _nwindows_min1, _nwindows_min2
_nwindows:
	.word 8
_nwindows_min1:	
	.word 7
_nwindows_min2:	
	.word 6
_leon_version:
	.word 3
