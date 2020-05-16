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
#define INT_HANDLER_NAME     XCAT(__bcc_trap_interrupt_flat,NAME_SUFFIX)
#define INT_HANDLER_NAME_SVT XCAT(__bcc_trap_interrupt_flat_svt,NAME_SUFFIX)

/*
 * Interrupt trap handler for the single register window model
 *
 * The idea is to allocate a stack frame where we put interrupted scratch
 * registers which must be stored (%g, %o) and also some %l, %i registers used
 * by the dispatcher itself.
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

	/*
	 * ISR context save
	 * Allocate space on stack for IU registers and ISR context
	 * %l4 is the future %sp
	 */
	/* in trap window (0) */
	sub	%fp, 96 + sizeof_isr_ctx_flat, %l4

	/* Store ISR context and export values from trap to "user" window */
	st 	%g1, [%l4 + 96 + isr_ctx_flat_g1]
	std	%g2, [%l4 + 96 + isr_ctx_flat_g2]
	mov	%l2, %g2
	std	%g4, [%l4 + 96 + isr_ctx_flat_g4]
	mov	%l3, %g3
	rd	%y, %g1
	std	%i0, [%l4 + 96 + isr_ctx_flat_o0]
	mov	%l0, %g5
	std	%i2, [%l4 + 96 + isr_ctx_flat_o2]
	mov	%l4, %g4
	st	%g1, [%l4 + 96 + isr_ctx_flat_y]
	std	%i4, [%l4 + 96 + isr_ctx_flat_o4]
	mov	%l1, %g1
	std	%i6, [%l4 + 96 + isr_ctx_flat_o6]

	restore

	/* in user window (1) */
	std	%fp, [%g4 + 96 + isr_ctx_flat_i6]
	mov	%sp, %fp
	mov	%g4, %sp

	/*
	 * We now have a stack frame etc.
	 *
	 * Put registers that we use on register store area: need only save
	 * %l0..%l7, %i0..%i5 if used by dispatch.
	 * NOTE: Some registers are stored after interrupts are reenabled.
	 */
	std	%l4, [%sp + store_area_l4]

	/* Interrupt nesting: set %psr.pil according to user configuration. */
	set	__bcc_int_nest, %l4
	andn	%g5, PSR_PIL, %o3
	ldub	[%l4 + %g3], %o1
	sll	%o1, PSR_PIL_BIT, %o2
	or	%o3, %o2, %o0

	/*
	 * Increment nestcount
	 * %l4: __bcc_int_nestcount
	 * %l5: old nestcount
	 * %o1: new nestcount
	 */
	ldub	[%l4], %l5
	add	%l5, 1, %o1
	stb	%o1, [%l4]

	/* Enable traps. xor ET in because we know it is zero. */
	/* Toggle CWP bit 0 to stay in window. */
	wr	%o0, (PSR_ET | 1), %psr
	nop
	nop
	nop
	/* in user window (1) */

	/* Store some more register we use in this trap handler. */
	std	%l0, [%sp + store_area_l0]
	mov	%g5, %l0
	mov	%g1, %l1
	std	%l2, [%sp + store_area_l2]
	mov	%g2, %l2
	mov	%g3, %l3
	st	%l6, [%sp + store_area_l6]

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

	mov	%l0, %g5
	mov	%l1, %g1
	mov	%l2, %g2

	/* Loading back some ISR context before disabling traps. */
	ldd	[%sp + store_area_l0], %l0
	ldd	[%sp + store_area_l2], %l2
	ld	[%sp + store_area_l6], %l6

	/* Disable traps. PIL is same as when interrupt trap was taken. */
	/* Toggle CWP bit 0 to stay in window. */
	/* NOTE: Do not modify condition codes after this wrpsr. */
	wr	%g5, 1, %psr
	nop
	nop
	nop
	/* in user window (1) */

	/*
	 * Decrement nestcount (using cached value/addr)
	 */
	stb	%l5, [%l4]
	ldd	[%sp + store_area_l4], %l4
	ldd	[%sp + 96 + isr_ctx_flat_i6], %fp

	/* Enter trap window for the last time so we can return from trap. */
	save
	/* in trap window (0) */

	/* Reverse of ISR context save */
	mov	%g1, %l1
	mov	%g2, %l2

	TN0018_WAIT_IFLUSH
	TN0018_WRITE_PSR	%g5

	ld	[%i6 + 96 + isr_ctx_flat_y], %g1
	ldd	[%i6 + 96 + isr_ctx_flat_g2], %g2
	ldd	[%i6 + 96 + isr_ctx_flat_g4], %g4
	wr	%g1, %y
	ld 	[%i6 + 96 + isr_ctx_flat_g1], %g1

	ldd	[%i6 + 96 + isr_ctx_flat_o0], %i0
	ldd	[%i6 + 96 + isr_ctx_flat_o2], %i2
	ldd	[%i6 + 96 + isr_ctx_flat_o4], %i4
	ldd	[%i6 + 96 + isr_ctx_flat_o6], %i6

	TN0018_FIX
	jmp	%l1
	 rett	%l2

FUNC_END INT_HANDLER_NAME
FUNC_END INT_HANDLER_NAME_SVT

