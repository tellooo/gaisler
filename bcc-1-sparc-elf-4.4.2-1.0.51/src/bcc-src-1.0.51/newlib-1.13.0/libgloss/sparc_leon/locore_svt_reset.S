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

#include <asm-leon/head.h>
                
	.section .text
	/* ------- */
	.weak	_hardreset_svt
	.set	_hardreset_svt, __hardreset_svt
	/* ------- */
	.global	_hardreset, _hardreset_svt_real
	
! reset entry point for single vector trapping. Try enable svt 
_hardreset_svt_real:
	nop
__hardreset_svt:
	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	.L2
	nop
	
	mov	%asr17, %g5		! set svt
	set     1<<13,%g1
	or      %g5,%g1,%g5
	mov     %g5, %asr17
	nop;  nop; nop
	mov	%asr17,%g5 		! check svt
	andcc	%g5, %g1, %g0
	beq	.L2
	 nop

	set _hardreset,%l3
	jmp %l3
         nop
	
.L2:	ta    0x0			! no svt implemented (ether leon2 or svt != 1)
        nop

