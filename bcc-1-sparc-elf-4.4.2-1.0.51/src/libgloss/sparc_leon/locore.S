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
                
#define TRAP(H)  mov %psr, %l0; sethi %hi(H), %l4; jmp %l4+%lo(H); nop;
#define TRAPL(H)  nop; sethi %hi(H), %g1; jmp %g1+%lo(H); nop;
#define TRAP_ENTRY(H) rd %psr, %l0; b H; rd %wim, %l3; nop;

#define WIM_INIT 2
#ifdef _SOFT_FLOAT
#define PSR_INIT 0x0e0
#else
#define PSR_INIT 0x10e0
#endif

#define NUMREGWINDOWS 8

/* Unexcpected trap will halt the processor by forcing it to error state */
#define BAD_TRAP ta 0; nop; nop; nop;

/* Software trap. Treat as BAD_TRAP */
#define SOFT_TRAP BAD_TRAP

  .seg    "text"
  .global _trap_table, start, _start, cpuinit, leonbare_irq_entry, _hardreset
  .global _window_overflow, _window_underflow, _flush_windows, _fpdis_enable
  /*.global _nwindows, _leon_version, _nwindows_min1*/

	
!
! Startup code for standalone system. Wash IU and FPU (if present) registers.
! The registers have to be written to initiate the parity bits.
!
	.section .text
	/* ------- */
	.weak	_hardreset
	.set	_hardreset,_hardreset_libgloss
	/* ------- */
	
	.global _hardreset_custom_weak, _hardreset_real, _cleanregs_custom_weak, _hardreset_custom_svt_weak

_hardreset_real:
	nop
_hardreset_libgloss:
	set     _hardreset_custom_weak, %g1  ! possible mkprom init code here, default links to dummy _hardreset_custom_dummy
	call     %g1
	 nop
	
	set     _trap_table, %g1       	! Initialize TBR
	mov     %g1, %tbr
	
	set     _hardreset_custom_svt_weak, %g1 ! give mkprom svt chance to reset tbr
	call     %g1
	 nop
	
	set     _cleanregs_custom_weak, %g1
	call     %g1
	 nop

#ifdef _FLAT
	mov	%g0, %wim
#else
/*	! assume that %sp is correct use cwp of psr to set the next window as invalid
	mov	%psr, %g2    ! extract cwp
	and     0x1f, %g2,%g2
	set     0x1, %g3
	sll     %g3,%g2,%g3  ! the bit mask for cwp

	sll     %g3, 1, %g4  ! rotate one to left
	sethi %hi(_nwindows_min1), %g5	! NWINDOWS-1
	ld [%g5+%lo(_nwindows_min1)], %g5
	srl  %g3, %g5, %g5
  	or   %g5, %g4, %g5
  	mov  %g5, %wim
  	nop; nop; nop */
#endif
	
/*
	mov	%psr, %g2
	set	0x202, %g3
	sll	%g3, %g2, %g2
	mov	%g2, %wim
	nop; nop; nop
1:
*/
	! -------------------------------
	! only cpu 0 initializes
/*	mov	%psr, %g5
	srl	%g5, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	callcpuinit
	nop
*/
	
	rd     %asr17,%g5
	srl    %g5,28,%g5
	cmp %g5,%g0
	 bne slavego
	nop
	
callcpuinit:      
	call	cpuinit
	 nop
	
	call	pnpinit
	 nop
	
slavego:	
        ! -------------------------------
	
        sub     %sp, 0x40, %sp  	! room for main to save args
	call	_start
        nop

	mov	1, %g1
	ta 	0			! Halt if _main would return ...
	nop

/* what is this supposed to do?!? */
	.global	_fpdis,_fpdis_svt
_fpdis_svt:
_fpdis:
	set	0x1000, %l4
	andcc	%l0, %l4, %l3
	bne,a	4f
	andn	%l0, %l4, %l0
	ta	0
4:
	mov      %l0, %psr		! restore %psr
	nop; nop; nop
  	jmp  %l2			! Jump to nPC
  	rett  %l2 + 4

	
/*
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	.section .bss
	.global _nwindows, _leon_version, _nwindows_min1
_nwindows:
	.word 8
_nwindows_min1:	
	.word 7
_leon_version:
	.word 3
*/
