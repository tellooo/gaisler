/*
 * Copyright (c) 2017, Cobham Gaisler AB
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

#include "bcc/leon.h"
.include "macros.i"
.include "context.i"
.include "isr_node.i"
#include "tn0018.i"

#define CAT(a,b) a##b
#define XCAT(a,b) CAT(a,b)

/* NAME_SUFFIX will be added to symbol names exported by this file. */
#ifndef NAME_SUFFIX
 #define NAME_SUFFIX
#endif
#define INT_HANDLER_NAME     XCAT(__bcc_trap_interrupt,NAME_SUFFIX)
#define INT_HANDLER_NAME_SVT XCAT(__bcc_trap_interrupt_svt,NAME_SUFFIX)

/*
 * Interrupt trap handler
 *
 * - IU state is saved and restored
 * - FPU state is not saved or touched
 * - Interrupt nesting is supported.
 *
 * On entry:
 * %l0: psr
 * %l1: pc
 * %l2: npc
 * %l3: SPARC interrupt request level (bp_IRL)
 */

	.section	".text"
	.global		INT_HANDLER_NAME_SVT
	.global		INT_HANDLER_NAME

FUNC_BEGIN INT_HANDLER_NAME_SVT
	/* We came from an SVT trap dispatcher with trap type in %l6 */
	sub	%l6, 0x10, %l3
FUNC_BEGIN INT_HANDLER_NAME
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

	/* Manual window overflow*/
	/* %g2 := ror(%g2) = ror(%wim) */
	 sethi	%hi(__bcc_nwindows_min1), %g3
	ld	[%g3 + %lo(__bcc_nwindows_min1)], %g3
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
	rd	%y, %g1
	st	%g4, [%sp + isr_ctx_g4]
	st	%g1, [%sp + isr_ctx_y]
	/* NOTE: We do not touch %i registers here, so do not store them. */

	/* Interrupt nesting: set %psr.pil according to user configuration. */
	set	__bcc_int_nest, %l4
	andn	%l0, PSR_PIL, %o3
	ldub	[%l4 + %l3], %o1
	sll	%o1, PSR_PIL_BIT, %o2
	or	%o3, %o2, %o0

	/*
	 * Increment nestcount
	 * %l4: __bcc_int_nestcount
	 * %l5: old nestcount
	 * %l6: new nestcount
	 */
	ldub	[%l4], %l5
	add	%l5, 1, %l6
	stb	%l6, [%l4]

	/* Allocate a fresh C stack frame. */
	sub	%sp, 96, %sp

	/* Enable traps. xor ET in because we know it is zero. */
	wr	%o0, PSR_ET, %psr
	nop
	nop
	nop

/** ISR dispatch BEGIN **/
	/* Convert SPARC bp_IRL (%o0) to user interrupt source (%o0). */
	call	__bcc_int_get_source
	 mov	%l3, %o0

	/* Check if interrupt source (%o0) could be decoded. */
	cmp	%o0, %g0
	beq	.Lskipisr
	 mov	%o0, %l6
	set	__bcc_isr_list, %g1
	/* Address to ISR handler list. */
	sll	%o0, 2, %g2
	/* %l3: address of first node. */
	ld	[%g1 + %g2], %l3

	/* NOTE: Only rely on local registers when calling. */
.Lnodeloop:
	/* Reached tail? */
	cmp	%l3, %g0
	beq	.Lskipisr
	/* Reload second handler argument since %o1 is volatile across calls. */
	 mov	%l6, %o1
	/* Load handler */
	ld	[%l3 + isr_node_handler], %g4
	nop
	ld	[%l3 + isr_node_arg], %o0
	/* Assume handler is always there */
	call	%g4
	 nop
.Lnodenext:
	b	.Lnodeloop
	 ld	[%l3 + isr_node_next], %l3
.Lskipisr:
/** ISR dispatch END **/

	/* Disable traps. PIL is same as when interrupt trap was taken. */
	wr	%l0, %psr
	nop
	nop
	nop

	/* Deallocate C stack frame. */
	add	%sp, 96, %sp

	/*
	 * Decrement nestcount (using cached value/addr)
	 */
	stb	%l5, [%l4]

	/* Reverse of ISR context save */

	ld	[%sp + isr_ctx_y], %g1
	wr	%g1, %y
	ld	[%sp + isr_ctx_g4], %g4
	ldd	[%sp + isr_ctx_g2], %g2
	ld 	[%sp + isr_ctx_g1], %g1

	/* Determine if we must prepare the return window. */
	rd	%wim, %l4
	/* l6 := cwp+1 */
	add	%l0, 1, %l6
	and	%l6, PSR_CWP, %l6
	/* Handle wrap-around */
	sethi	%hi(__bcc_nwindows), %l5
	ld	[%l5 + %lo(__bcc_nwindows)], %l7
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
	/* %l7 := __bcc_nwindows - 1 */
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

.Lwudone:
	TN0018_WAIT_IFLUSH

	/* Restore %psr since we may have trashed condition codes. */
	wr	%l0, %psr
	nop
	nop
	nop

	TN0018_FIX
	jmp	%l1
	 rett	%l2

FUNC_END INT_HANDLER_NAME
FUNC_END INT_HANDLER_NAME_SVT

