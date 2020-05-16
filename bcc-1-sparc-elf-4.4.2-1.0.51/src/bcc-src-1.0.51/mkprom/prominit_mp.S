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
	mov      %o7,%g7
	
	set     PSR_INIT,%g1
	wr      %g1, %g0, %psr
	mov     %g0, %tbr 
	mov     %g0, %wim;
	
	flush
	
	clr	%g1
	clr	%g2
	clr	%g3
	clr	%g4
	clr	%g5
	clr	%g6

	! -------------------------------
	! only cpu 0 initializes
	!mov	%psr, %g5
	!srl	%g5, 24, %g5
	!and  	%g5, 3, %g5
	!subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	!be	1f
	!nop	
	

       ! -------------------------------
	
	set	noinit, %g2		
	ld	[%g2], %g2
	subcc   %g2, %g0, %g0
	bne	sysini0
	
	set	0x81000f, %g2	
  	sta	%g2, [%g0] 2	        ! enable LEON3 cache

	rd     %asr17,%g5
	srl    %g5,28,%g5
	cmp    %g5,%g0
	bne    sysini0
	nop			
	
	! initialize SDRAM ctrl
	set	0x8a200348, %g2         ! FIXME
	set	0xfff80100, %g3
	st	%g2, [%g3]	
	
	set	freq, %g2		! Load timer scaler register
	ld	[%g2], %g2
	!be	1f
	sub	%g2, 1, %g2

	set     _gptaddr, %g1
	ld	[%g1], %g1
	st	%g2, [%g1 + 0x4]        ! scaler register
	ld	[%g1+0x8], %g6		! timer conf reg
	and	%g6, 7, %g6		! timer nr
	sll	%g6, 4, %g6
	add	%g1, %g6, %g6
1:	cmp	%g1, %g6
	be	2f
	add	%g1, 0x10, %g1	
	set	-1, %g3
	st	%g3, [%g1]		! timer value reg
	st	%g3, [%g1 + 0x4]	! timer reload reg
	set	7, %g3
	ba	1b	
	st	%g3, [%g1 + 0x8]	! timer ctrl reg
	
2:		
	set	_uartnr, %g3
	ld	[%g3], %g3		! uart nr
	set	_uaddr, %g1
	!ld	[%g1], %g4				
	sll	%g3, 2, %g3
	add	%g1, %g3, %g3
	set	_uart, %g2		
	ld	[%g2], %g2
2:	ld	[%g1], %g4			
	cmp	%g1, %g3
	be	1f
	nop
  	st	%g2, [%g4 + 0xc]        ! UART scaler register
	set	3, %g5			! 
  	st	%g5, [%g4 + 0x8]	! Enable UART
	st	%g0, [%g4 + 0x4]	! Clear status reg
	ba	2b					
	add	%g1, 4, %g1			

1:		
	mov	%g0, %y 		! Init Y-register

sysini0:

!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
! get nwindows and leon version
			
	mov	%asr17, %g5		! leon3 has nwindows in %asr17
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
	
	set	_stack, %g1
	rd      %asr17, %g5
	srl     %g5, 28, %g5
	sll	%g5, 2, %g5		
	ld	[%g1+%g5], %g1

	st	%g0, [%g1]
	ld	[%g1], %f0
	ld	[%g1], %f1
	ld	[%g1], %f2
	ld	[%g1], %f3
	ld	[%g1], %f4
	ld	[%g1], %f5
	ld	[%g1], %f6
	ld	[%g1], %f7
	ld	[%g1], %f8
	ld	[%g1], %f9
	ld	[%g1], %f10
	ld	[%g1], %f11
	ld	[%g1], %f12
	ld	[%g1], %f13
	ld	[%g1], %f14
	ld	[%g1], %f15
	ld	[%g1], %f16
	ld	[%g1], %f17
	ld	[%g1], %f18
	ld	[%g1], %f19
	ld	[%g1], %f20
	ld	[%g1], %f21
	ld	[%g1], %f22
	ld	[%g1], %f23
	ld	[%g1], %f24 
	ld	[%g1], %f25
	ld	[%g1], %f26
	ld	[%g1], %f27
	ld	[%g1], %f28
	ld	[%g1], %f29
	ld	[%g1], %f30
	ld	[%g1], %f31
	ld	[%g1], %fsr

#endif

! System init

sysini:

	! -------------------------------
	! only cpu 0 initializes	
	rd     %asr17, %g5
	srl    %g5, 28, %g5
	cmp    %g5, %g0
	beq    mastergo
	nop

	! note:	 on a .text in rom mkprom image _entry is 0
	!set _entry,%g6                 
	!ld  [%g6],%g6
	!cmp %g0,%g6
	!bne,a slavego
	!mov  %g6,%g7                   ! execute only if branch taken (g6 != 0)
	ba,a slavego    
	
        ! -------------------------------
mastergo:		
	call	bdinit1
	nop
slavego:	
	set	_stack, %g2
	sll	%g5, 2, %g5		
	ld	[%g2+%g5], %fp
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
	

