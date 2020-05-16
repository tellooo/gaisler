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

	
	.global	getpsr
	
getpsr:
	retl
	mov %psr, %o0

	
/* Number of register windows */
/*	.global _nwindows_min1, _nwindows*/
	
        ! Window overflow trap handler on save.
        ! Touches %g1
  	.global _window_overflow,_window_overflow_svt

_window_overflow_svt:		
_window_overflow:
#ifndef _FLAT
  	mov  %wim, %l3       		! Calculate next WIM
  	mov  %g1, %l7           
  	srl  %l3, 1, %g1        
	
	/*sethi %hi(_nwindows_min1), %l4	! NWINDOWS-1
	ld [%l4+%lo(_nwindows_min1)], %l4
	*/
	
	/* ================== */
	mov	%psr, %l4
	srl	%l4, 24, %l4
	and  	%l4, 3, %l4
	subcc	%l4, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	1f
	nop
	mov	%asr17, %l4		! leon3 has nwindows in %asr17
	ba	2f
1:
	set     0x80000000, %l4         ! leon2 on-chip register base
	ld	[%l4+0x24], %l4
	srl	%l4, 20, %l4
2:
	and	%l4, 0x1f, %l4
	/* ================== */


	
	
	sll  %l3, %l4 , %l4
  	or   %l4, %g1, %g1

  	save                              ! Get into window to be saved.
  	mov  %g1, %wim
  	nop; nop; nop
        std   %l0, [%sp + 0];
        std   %l2, [%sp + 8];
        std   %l4, [%sp + 16]; 
        std   %l6, [%sp + 24]; 
        std   %i0, [%sp + 32]; 
        std   %i2, [%sp + 40]; 
        std   %i4, [%sp + 48]; 
        std   %i6, [%sp + 56]; 
  	restore				! Go back to trap window.
  	mov  %l7, %g1
  	jmp  %l1			! Re-execute save.
  	rett %l2
#else	
	ta	0			! halt 
#endif
	
  /* Window underflow trap handler on restore.  */

        ! Touches %g1
  	.global  _window_underflow,_window_underflow_svt

_window_underflow_svt:
_window_underflow:
#ifndef _FLAT
	mov  %wim, %l3			! Calculate next WIM
	sll  %l3, 1, %l4

	/*sethi %hi(_nwindows_min1), %l5	! NWINDOWS-1
	ld [%l5+%lo(_nwindows_min1)], %l5
	*/
	/* ================== */
	mov	%psr, %l5
	srl	%l5, 24, %l5
	and  	%l5, 3, %l5
	subcc	%l5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	1f
	nop
	mov	%asr17, %l5		! leon3 has nwindows in %asr17
	ba	2f
1:
	set     0x80000000, %l5         ! leon2 on-chip register base
	ld	[%l5+0x24], %l5
	srl	%l5, 20, %l5
2:
	and	%l5, 0x1f, %l5
	/* ================== */
	
	srl  %l3, %l5, %l5
  	or   %l5, %l4, %l5
  	mov  %l5, %wim
  	nop; nop; nop
  	restore				! Two restores to get into the
  	restore				! window to restore
        ldd   [%sp + 0], %l0; 		! Restore window from the stack
        ldd   [%sp + 8], %l2; 
        ldd   [%sp + 16], %l4; 
        ldd   [%sp + 24], %l6; 
        ldd   [%sp + 32], %i0; 
        ldd   [%sp + 40], %i2; 
        ldd   [%sp + 48], %i4; 
        ldd   [%sp + 56], %i6; 
  	save				! Get back to the trap window.
  	save
  	jmp  %l1			! Re-execute restore.
  	rett  %l2
#else	
	ta	0			! halt 
#endif

        .global leonbare_irq_entry,_leonbare_irq_entry_svt
	.global _fpdis_enable,_fpdis_enable_svt
	.global	_flush_windows,_flush_windows_svt
        .global _irqcall,_irqcall_svt,_irqcall_disableirq,_irqcall_disableirq_svt
	
_leonbare_irq_entry_svt:	 /* irq from svt trap dispatcher */
leonbare_irq_entry:		
_fpdis_enable_svt:
_fpdis_enable:
_flush_windows_svt:		
_flush_windows:
_irqcall_svt:	
_irqcall:
_irqcall_disableirq_svt:		
_irqcall_disableirq:		
	jmpl	%l1,  %g0
	rett	%l2


	.global cpuinit
cpuinit:
	retl
	nop
