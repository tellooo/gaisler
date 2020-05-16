/************************************************************************/
/*   This file is a part of the mkprom3 boot-prom utility               */
/*   Copyright (C) 2004 Gaisler Research                                */
/*                                                                      */
/*   This library is free software; you can redistribute it and/or      */
/*   modify it under the terms of the GNU General Public                */
/*   License as published by the Free Software Foundation; either       */
/*   version 2 of the License, or (at your option) any later version.   */
/*                                                                      */
/*   See the file COPYING.GPL for the full details of the license.      */
/************************************************************************/

	
/*
 * this will override the weak hardreset_custom in libgloss's locore.s
 */

/* Entry for traps which jump to a programmer-specified trap handler.  */
#ifndef _SOFT_FLOAT
#define PSR_INIT 0x10e0
#else
#define PSR_INIT 0x00e0
#endif
#define TRAPR(H)  wr %g0, PSR_INIT, %psr; mov %g0, %tbr; ba (H); mov %g0, %wim;
#define TRAP(H)  mov %psr, %l0; sethi %hi(H), %l4; jmp %l4+%lo(H); nop;

/* Unexcpected trap will halt the processor by forcing it to error state */
#define BAD_TRAP ta 0; nop; nop; nop;

/* Number of register windows */
#define NWINDOWS 	8

#define WIM_INIT   2

	
  .seg    "text"
  .global _hardreset_custom_weak

_hardreset_custom_weak:
	flush
	mov      %o7,%g7
	
	clr	%g1
	clr	%g2
	clr	%g3
	clr	%g4
	clr	%g5
	clr	%g6


	! -------------------------------
	! only cpu 0 initializes
	mov	%psr, %g5
	srl	%g5, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	1f
	nop
	
	rd     %asr17,%g5
	srl    %g5,28,%g5
	cmp %g5,%g0
	 bne sysini0
	nop
	
1:
       ! -------------------------------
	
	set	noinit, %g2		! Load memory config register 1
	ld	[%g2], %g2
	subcc   %g2, %g0, %g0
	bne	sysini0

	set	0x81000f, %g2
	mov	%psr, %g1
	srl	%g1, 24, %g1
	and  	%g1, 0x0f, %g1
	subcc	%g1, 3, %g1
	be	1f
	set     0x80000000, %g1
  	st	%g2, [%g1+0x14]
	set	0xAA00, %g3
	ba 2f
  	st	%g3, [%g1+0xA4]
1:
	set	_memcaddr, %g1
	ld	[%g1], %g1		
  	sta	%g2, [%g0] 2	        ! enable LEON3 cache
        st      %g0, [%g1 + 0x08]     
2:
	set	_memcfg1, %g2		! Load memory config register 1
	ld	[%g2], %g2
  	ld	[%g1], %g3		! 
	and	%g3, 0x300, %g3
	or	%g2, %g3, %g2
  	st	%g2, [%g1 + 0x00]
	set	_memcfg2, %g2		! Load memory config register 2
	ld	[%g2], %g2
  	st	%g2, [%g1 + 0x04]
	set	_memcfg3, %g2		! Load memory config register 3
	ld	[%g2], %g2
  	st	%g2, [%g1 + 0x08]

	set	freq, %g2		! Load timer scaler register
	ld	[%g2], %g2
	be	1f
	sub	%g2, 1, %g2

	st	%g2, [%g1 + 0x64]
	st	%g2, [%g1 + 0x60]
	set	-1, %g3
	st	%g3, [%g1 + 0x40]
	st	%g3, [%g1 + 0x44]
	set	7, %g3
	st	%g3, [%g1 + 0x48]
	set	0x80000070, %g1
	ba	2f
	nop
1:
	set	_gptaddr, %g1
	ld	[%g1], %g1	
  	st	%g2, [%g1 + 0x4]
  	st	%g2, [%g1 + 0x0]
	set	-1, %g3
	st	%g3, [%g1 + 0x10]
	st	%g3, [%g1 + 0x14]
	set	7, %g3
	st	%g3, [%g1 + 0x18]

	set	_uaddr, %g1
	ld	[%g1], %g1	
2:
	set	_uart, %g2		! Load UART scaler register
	ld	[%g2], %g2
  	st	%g2, [%g1 + 0xc]
	set	3, %g2			! Enable UART
  	st	%g2, [%g1 + 0x8]
	st	%g0, [%g1 + 0x4]

	mov	%g0, %y 		! Init Y-register

sysini0:

	
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
! get nwindows and leon version
			
	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	1f
	nop
	mov	%asr17, %g5		! leon3 has nwindows in %asr17
	ba	2f
1:
	set     0x80000000, %l0         ! leon2 on-chip register base
	ld	[%l0+0x24], %l3
	srl	%l3, 20, %g5
2:
	and	%g5, 0x1f, %g5
	add     %g5,1,%g5
	
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''


	
	mov	%g5, %g2
wl0:
	clr	%i0
	clr	%i1
	clr	%i2
	clr	%i3
	clr	%i4
	clr	%i5
	clr	%i6 
	clr	%i7
	clr	%l0
	clr	%l1
	clr	%l2
	clr	%l3
	clr	%l4
	clr	%l5
	clr	%l6
	clr	%l7
	save
	subcc	%g2, 1, %g2
	bne	wl0
	nop

	wr	%g0, PSR_INIT, %psr
	set	WIM_INIT, %g3
	mov	%g3, %wim

#ifndef _SOFT_FLOAT

! Wash FPU register file

	set	_fsr, %g1
	ldd	[%g1], %f0
	ldd	[%g1], %f2
	ldd	[%g1], %f4
	ldd	[%g1], %f6
	ldd	[%g1], %f8
	ldd	[%g1], %f10
	ldd	[%g1], %f12
	ldd	[%g1], %f14
	ldd	[%g1], %f16
	ldd	[%g1], %f18
	ldd	[%g1], %f20
	ldd	[%g1], %f22
	ldd	[%g1], %f24
	ldd	[%g1], %f26
	ldd	[%g1], %f28
	ldd	[%g1], %f30
	ld	[%g1], %fsr

#endif

! System init

sysini:
	
	! -------------------------------
	! only cpu 0 initializes
	mov	%psr, %g5
	srl	%g5, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	callbedinit1
	nop
	
	rd     %asr17,%g5
	srl    %g5,28,%g5
	cmp %g5,%g0
	 beq mastergo
	nop
	
	set _entry,%g7
	ba slavego
	 ld  [%g7],%g7
	
        ! -------------------------------
		
mastergo:	
	call	bdinit1
	 nop
slavego:	
	set	_stack, %g2
	ld	[%g2], %fp
	andn	%fp, 0x0f, %fp
	sub	%fp, 64, %sp

	mov %g7,%o7
	retl
	nop

/*		
        sub     %sp, 0x40, %sp  	! room for main to save args
	call	crt2
        nop
*/
	
	.align 8
_fsr:
	.word 0
	.word 0



