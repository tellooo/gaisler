/*
 * Copyright (c) 2018, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <bcc/leon.h>
.include "macros.i"
.include "context.i"

/*
 * 1: switch stack during trap handling to space reserved by safe environment
 * 0: do not switch stack
 */
#define CUSTOM_STACK 1

/*
 * Soft trap handler
 *
 * - IU state is saved and restored
 * - FPU state is not saved or touched
 *
 * On entry:
 * %l0: psr
 * %l1: pc
 * %l2: npc
 * %g1: function to call
 *
 * NOTE: This trap handler saves more state than needed.
 */

	.section	".text"

FUNC_BEGIN __safe_trap_soft_svt
FUNC_BEGIN __safe_trap_soft
	/* %g2/%g3 used during manual window overflow. */
	mov	%g2, %l4
	mov	%g3, %l5
	/*
	 * We are in our own register window. It may be the "invalid" window.
	 * If so, we must save the next.
	 */
	rd	%wim, %g2
	srl	%g2, %l0, %g3
	cmp	%g3, 1
	bne	.Lwodone
	 nop

	/* Manual window overflow*/
	/* %g2 := ror(%g2) = ror(%wim) */
	sethi	%hi(__safe_nwindows_min1), %g3
	ld	[%g3 + %lo(__safe_nwindows_min1)], %g3
	sll	%g2, %g3, %g3
	srl	%g2, 1, %g2
	or	%g2, %g3, %g2

	/* Enter window to save, bringing %g2 with us. */
	save

	/* Put on stacks register store area. */
	std	%l0, [%sp + store_area_l0]
	/* Install new %wim. (It marks current window as invalid.) */
	wr	%g2, %wim
	std	%l2, [%sp + store_area_l2]
	B2BSTORE_FIX
	std	%l4, [%sp + store_area_l4]
	B2BSTORE_FIX
	std	%l6, [%sp + store_area_l6]
	B2BSTORE_FIX
	std	%i0, [%sp + store_area_i0]
	B2BSTORE_FIX
	std	%i2, [%sp + store_area_i2]
	B2BSTORE_FIX
	std	%i4, [%sp + store_area_i4]
	B2BSTORE_FIX
	std	%i6, [%sp + store_area_i6]

	restore

	/* Manual window overflow completed */

.Lwodone:
	/* ISR context save */
	/* Allocate space on stack for IU registers */
	sub	%fp, sizeof_isr_ctx, %sp
	st 	%g1, [%sp + isr_ctx_g1]
	std	%l4, [%sp + isr_ctx_g2]
	rd	%y, %l4
	st	%g4, [%sp + isr_ctx_g4]
	st	%l4, [%sp + isr_ctx_y]
	/* NOTE: We do not touch %i registers here, so do not store them. */

	/* Highest interrupt priority level */
	or	%l0, PSR_PIL, %o0

#if CUSTOM_STACK
	/* Install our private stack */
	set	__safe_trap_sp, %l3
	st	%sp, [%l3]
	set	__safe_stack, %sp
#endif
	/* Allocate a fresh C stack frame. */
	sub	%sp, 96, %sp

	/* Install our private tbr */
	set	__safe_trap_tbr, %l3
	rd	%tbr, %l4
	st	%l4, [%l3]
	set	__safe_trap_table, %l4
	wr	%l4, %tbr
	nop
	nop
	nop

	/* Enable traps. xor ET in because we know it is zero. */
	wr	%o0, PSR_ET, %psr
	nop
	nop
	nop

/** SAFE dispatch BEGIN **/
	/* Copy arguments set by user before trap */
	mov	%i0, %o0
	mov	%i1, %o1
	mov	%i2, %o2
	mov	%i3, %o3
	mov	%i4, %o4
	mov	%i5, %o5

	call	%g1
	 nop
	mov	%o0, %i0
/** SAFE dispatch END **/

	/* Disable traps. PIL is same as when interrupt trap was taken. */
	wr	%l0, %psr
	nop
	nop
	nop

	/* Restore private tbr */
	set	__safe_trap_tbr, %l3
	ld	[%l3], %l4
	wr	%l4, %tbr

#if CUSTOM_STACK
	set	__safe_trap_sp, %l3
	ld	[%l3], %sp
#endif
	/* Deallocate C stack frame. */
	add	%sp, 96, %sp

	/* Reverse of ISR context save */

	ld	[%sp + isr_ctx_y], %g1
	wr	%g1, %y
	ld	[%sp + isr_ctx_g4], %g4
	ldd	[%sp + isr_ctx_g2], %g2
	ld 	[%sp + isr_ctx_g1], %g1

	/* NOTE: no need to change windows at return in this example */
#if 0
	/* Determine if we must prepare the return window. */
	rd	%wim, %l4
	/* l6 := cwp+1 */
	add	%l0, 1, %l6
	and	%l6, PSR_CWP, %l6
	/* Handle wrap-around */
	sethi	%hi(__safe_nwindows), %l5
	ld	[%l5 + %lo(__safe_nwindows)], %l7
	cmp	%l6, %l7
	bge,a	.Lwrapok
	 mov	0, %l6
.Lwrapok:
	/* %l5 := %wim >> (cwp+1) */
	srl	%l4, %l6, %l5
	/* %l5 is 1 if (cwp+1) is an invalid window */
	cmp	%l5, 1
	bne	.Lwudone
	 nop

	/* Manual window underflow */
	/* %wim = rol(%wim) */
	/* %l7 := __safe_nwindows - 1 */
	sub	%l7, 1, %l7
	srl	%l4, %l7, %l5
	sll	%l4, 1, %l4
	wr	%l4, %l5, %wim
	nop
	nop
	nop

	restore
	ldd	[%sp + store_area_l0], %l0
	ldd	[%sp + store_area_l2], %l2
	ldd	[%sp + store_area_l4], %l4
	ldd	[%sp + store_area_l6], %l6
	ldd	[%sp + store_area_i0], %i0
	ldd	[%sp + store_area_i2], %i2
	ldd	[%sp + store_area_i4], %i4
	ldd	[%sp + store_area_i6], %i6
	save

	/* Manual window underflow completed */
#endif

.Lwudone:
	/* Restore %psr since we may have trashed condition codes. */
	wr	%l0, %psr
	nop
	nop
	nop

	jmp	%l2
	 rett	%l2 + 4

FUNC_END __safe_trap_soft
FUNC_END __safe_trap_soft_svt

	.section        ".data"
	.align		4
	.global         __safe_nwindows
	.global         __safe_nwindows_min1

__safe_nwindows_min1:
	.word		7
__safe_nwindows:
	.word		8


#if CUSTOM_STACK
	.section        ".bss"
	.align		0x10
	.space		1024
__safe_stack:

	.align		4
__safe_trap_sp:
	.word		0
__safe_trap_tbr:
	.word		0
#endif

