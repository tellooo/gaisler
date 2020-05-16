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
	.weak	_hardreset_mvt
	.set	_hardreset_mvt,_hardreset_mvt_ram
	/* ------- */
	.global	_hardreset
	
_hardreset_mvt_ram:	
	mov	%psr, %g1
	srl	%g1, 24, %g1
	and  	%g1, 3, %g1
	subcc	%g1, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	.L2
	 nop
	
	mov	%asr17, %g1		! set svt
	set     1<<13,%g2
	andn    %g1,%g2,%g1
	mov     %g1, %asr17
	
.L2:	mov	%g0, %g4
	sethi	%hi(_hardreset), %g4
	jmp	%g4+%lo(_hardreset);
         nop

