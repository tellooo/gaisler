       /* Number of register windows */
	
        ! Window overflow trap handler on save.
        ! Touches %g1
	/* ------- */
	.global	_window_overflow
	.global	_window_overflow_svt
	/* ------- */

calc_nwindows_min1:	
	mov	%psr, %l4
	srl	%l4, 24, %l5
	and  	%l5, 3, %l5
	subcc	%l5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	1f
	 nop
	mov	%asr17, %l4		! leon3 has nwindows in %asr17
	ba	2f
1:       set     0x80000000, %l5         ! leon2 on-chip register base
	ld	[%l5+0x24], %l4
	srl	%l4, 20, %l4
2:
	jmpl    %l7+8,%g0
	 and	%l4, 0x1f, %l4


	
_window_overflow_svt:		
_window_overflow:
#ifndef _FLAT
  	mov  %wim, %l3       		! Calculate next WIM
  	mov  %g1, %l6           
	jmpl calc_nwindows_min1,%l7
         srl  %l3, 1, %g1        
	
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
  	mov  %l6, %g1
  	jmp  %l1			! Re-execute save.
  	rett %l2
#else	
	ta	0			! halt 
#endif
	
  /* Window underflow trap handler on restore.  */

	/* ------- */
  	.global	_window_underflow
	.global	_window_underflow_svt
	/* ------- */

_window_underflow_svt:
_window_underflow:
#ifndef _FLAT
	mov  %wim, %l3			! Calculate next WIM
	jmpl calc_nwindows_min1,%l7
	 sll  %l3, 1, %l6
	 
	srl  %l3, %l4, %l4
  	or   %l4, %l6, %l4
  	mov  %l4, %wim
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


	
