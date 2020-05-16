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

	
		
	.text
	.global _start, main, _nwindows, start_ecos, _end
  .global _nwindows, _leon_version, _nwindows_min1

_start:
#ifndef _FLAT
	save	%sp, -64, %sp
#else
	add	%sp, -72, %sp
	st	%o7, [%sp+64]
#endif
        /* move data */

	! -------------------------------
	! only cpu 0 initializes
	mov	%psr, %g5
	srl	%g5, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	copydata
	nop
	
	rd     %asr17,%g5
	srl    %g5,28,%g5
	cmp %g5,%g0
	 bne slavego
	nop
	! -------------------------------
	
copydata:	
	set   _edata, %g2
	set   _sdata, %g3
	sub   %g2, %g3, %g4
	set   _endtext, %g2
mvdata:
	subcc %g4, 8, %g4
	ldd   [%g2+%g4], %g6
	bge,a mvdata
	std    %g6,[%g3+%g4]

        /* clear bss */
 
        set   bss_start, %g2
        set   _end, %g3
        mov   %g0, %g1
	sub   %g3, %g2, %g3
zerobss:
	subcc %g3, 8, %g3
	bge,a zerobss
	std   %g0, [%g2+%g3]

	set    _end, %o0
	st     %g0,[%o0]
	
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
! get nwindows and leon version
			
	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	1f
	nop
	set     _leon_version,%l0
	set     3,%l1
	st      %l1,[%l0]
	mov	%asr17, %g5		! leon3 has nwindows in %asr17
	ba	2f
1:
	set     _leon_version,%l0
	set     2,%l1
	st      %l1,[%l0]
	set     0x80000000, %l0         ! leon2 on-chip register base
	ld	[%l0+0x24], %l3
	srl	%l3, 20, %g5
2:
	and	%g5, 0x1f, %g5
	set	_nwindows_min1, %l3
	st	%g5, [%l3]
	add     %g5,1,%g5
	set	_nwindows, %l3
	st	%g5, [%l3]

!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

/*	
	set	_nwindows, %g1
	st	%g5, [%g1]
*/
	
        call  cpuinit
        nop
	call	bdinit2
	nop
	
slavego:		
        call  start_ecos
        nop
	
#ifndef _FLAT
	ret
	restore
#else
	ld	[%sp+64], %o7
	retl
	add	%sp, 72, %sp
#endif

        .seg    "data"
        .global .bdata
.bdata:
        .align  8
        .global _environ
_environ:
        .word   1


irqtbl:
        .word   1
	.skip 60
        .section   .bss 
	.align 4
        .global _errno                  ! not defined elsewhere ..?
_errno:
        .skip 4

