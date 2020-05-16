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

	.section	".text"
	.global		__bcc_trap_flush_windows

/*
 * Handler for SPARC trap 0x83: trap_instruction, defined as "Flush windows" by
 * SPARC-ABI.
 *
 * This implementation uses the window overflow trap handler to perform the
 * actual window flush.
 *
 * On entry:
 * %l0: psr
 * %l1: pc
 * %l2: npc
 */
FUNC_BEGIN __bcc_trap_flush_windows
	/* Save globals */
	set	__bcc_winflush_spill_globals, %l4
	std	%g2, [%l4]
	B2BSTORE_FIX
	st	%l2, [%l4 + 0x10]
	std	%l0, [%l4 + 0x08]

	restore

	/* In window where we trapped from. This window will not be flushed. */

	sethi	%hi(__bcc_nwindows_min1), %g3
	ld	[%g3 + %lo(__bcc_nwindows_min1)], %g3

	/* Set highest processor interrupt level and enable traps */
	rd	%psr, %g2
	or	%g2, PSR_PIL, %g2
	wr	%g2, PSR_ET, %psr
	nop

	/* %g3: NWINDOWS-2 */
	sub	%g3, 1, %g3
	mov	%g3, %g2

	/* save NWINDOWS-1 times. */
1:
	save
	cmp	%g3, %g0
	bne	1b
	 sub	%g3, 1, %g3

	/* restore NWINDOWS-1 times. */
2:
	restore
	cmp	%g2, %g0
	bne	2b
	 sub	%g2, 1, %g2

	save

	/* Restore globals */
	set	__bcc_winflush_spill_globals, %l4
	ldd	[%l4], %g2
	ldd	[%l4 + 0x08], %l0
	ld	[%l4 + 0x10], %l2

	/* Restore %psr */
	wr	%l0, %psr
	nop
	nop
	nop

	/* GRLIB-TN-0018: assume trap was generated with "ta N; nop" */
	jmp	%l2
	 rett	%l2 + 4
FUNC_END __bcc_trap_flush_windows

	.section	.bss
	.align		8
__bcc_winflush_spill_globals:
	/* %g2, %g3, %l0, %l1, %l2 */
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0

