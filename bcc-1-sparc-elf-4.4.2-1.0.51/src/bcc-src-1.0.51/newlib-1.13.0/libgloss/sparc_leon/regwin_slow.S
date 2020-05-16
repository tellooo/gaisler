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
 * regwin.s for LEON
 */

#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/asmmacro.h>
        
  .seg    "text"

	
/* Number of register windows */
	.global _nwindows_min1, _nwindows
	
        ! Window overflow trap handler on save.
        ! Touches %g1
	/* ------- */
	.weak	_window_overflow
	.set	_window_overflow,__window_overflow
	.weak	_window_overflow_svt
	.set	_window_overflow_svt,__window_overflow_svt
	/* ------- */
  	!.global _window_overflow,_window_overflow_svt
  	.global __window_overflow_rettseq,__window_overflow_rettseq_ret,__window_overflow_slow1

__window_overflow_svt:		
__window_overflow:
#ifndef _FLAT
__window_overflow_rettseq:		
  	mov  %wim, %l3       		! Calculate next WIM
  	mov  %g1, %l7           
  	srl  %l3, 1, %g1        
	
__window_overflow_rettseq_ret:		
	sethi %hi(_nwindows_min1), %l4	! NWINDOWS-1
	ld [%l4+%lo(_nwindows_min1)], %l4
	
	sll  %l3, %l4 , %l4
  	or   %l4, %g1, %g1

  	save                              ! Get into window to be saved.
  	mov  %g1, %wim
  	nop; nop; nop
        std   %l0, [%sp + 0];
	B2BSTORE_FIX
        std   %l2, [%sp + 8];
	B2BSTORE_FIX
        std   %l4, [%sp + 16]; 
	B2BSTORE_FIX
        std   %l6, [%sp + 24]; 
	B2BSTORE_FIX
        std   %i0, [%sp + 32]; 
	B2BSTORE_FIX
        std   %i2, [%sp + 40]; 
	B2BSTORE_FIX
        std   %i4, [%sp + 48]; 
	B2BSTORE_FIX
        std   %i6, [%sp + 56]; 
  	restore				! Go back to trap window.
  	mov  %l7, %g1
	
  	jmp  %l1			! Re-execute save.
  	rett %l2
	nop
	
__window_overflow_slow1:		! space for possible stackcheck patch
	nop
	nop
#else	
	ta	0			! halt 
__window_overflow_rettseq:		
__window_overflow_rettseq_ret:		
__window_overflow_slow1:		
	nop
	nop
	nop
#endif
	
  /* Window underflow trap handler on restore.  */

        ! Touches %g1
	/* ------- */
  	.weak	_window_underflow
	.set	_window_underflow,__window_underflow
	.weak	_window_underflow_svt
	.set	_window_underflow_svt,__window_underflow_svt
	/* ------- */
  	!.global  _window_underflow,_window_underflow_svt

__window_underflow_svt:
__window_underflow:
#ifndef _FLAT
	mov  %wim, %l3			! Calculate next WIM
	sll  %l3, 1, %l4

	sethi %hi(_nwindows_min1), %l5	! NWINDOWS-1
	ld [%l5+%lo(_nwindows_min1)], %l5
	
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

	
