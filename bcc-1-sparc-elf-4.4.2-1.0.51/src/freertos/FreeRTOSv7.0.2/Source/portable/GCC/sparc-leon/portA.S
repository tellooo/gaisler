#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/contextswitch.h>
#include <asm-leon/winmacros.h>
#include "portmacro.h"
	
	.global FreeRtosSwitchTo, vPortYieldFromISR, pxCurrentTCB, freeos_getsp, freeos_getpsr, freeos_error
	
	.text

vPortYieldFromISR:
#ifndef _FLAT
	ta 	0x3;
#endif
	sub 	%sp, SF_REGS_SZ+FREERTOS_STACK_SIZE, %sp
	add 	%sp, SF_REGS_SZ, %o2
	
	RW_STORE(o2)
	
	mov 	%psr, %o3
	st	%o3, [%o2 + FREERTOS_STACK_PSR_OFF]
	mov 	%wim, %o3
	st	%o3, [%o2 + FREERTOS_STACK_WIM_OFF]
	st	%o7, [%o2 + FREERTOS_STACK_PC_OFF]

	set	pxCurrentTCB, %o0
	ld	[%o0], %o0
	st	%sp, [%o0]
	
	call	vTaskSwitchContext
	 nop
	
	set	pxCurrentTCB, %o0
	ld	[%o0], %o0
	
FreeRtosSwitchTo:
	ld	[%o0], %g2					/* top word is new stack pointer */

	ld	[%g2 + SF_REGS_SZ + FREERTOS_STACK_PSR_OFF], %g1
	wr	%g0,%wim
	nop; nop; nop
	wr	%g1, %psr
	nop; nop; nop
	
	add 	%g2, SF_REGS_SZ, %o2
	RW_LOAD(o2)
	
	ld	[%o2 + FREERTOS_STACK_PSR_OFF], %g1
	ld	[%o2 + FREERTOS_STACK_PC_OFF], %o7
	ld	[%o2 + FREERTOS_STACK_O0_OFF], %o0
	add 	%g2,  SF_REGS_SZ + FREERTOS_STACK_SIZE, %sp

	mov	%psr, %g1
	SET_WIM_CWPMIN1(g1,g2,g3,g4,g5) 
	
	retl
	 nop
	
freeos_getsp:
	retl
	 mov	%sp, %o0

freeos_getpsr:
	retl
	 mov	%psr, %o0

freeos_error:
	ta	0x0
	 nop