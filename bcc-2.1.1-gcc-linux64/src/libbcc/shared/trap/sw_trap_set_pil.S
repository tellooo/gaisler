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
	.global		__bcc_trap_sw_set_pil

/*
 * Set processor interrupt level
 *
 * Handler for SPARC trap 0x89: trap_instruction, defined as "Reserved for the
 * operating system" by SPARC-ABI.
 *
 * entry:
 * - %l0: psr
 * - %l1: pc
 * - %l2: npc
 * - %i0: New processor interrupt level
 *
 * return:
 * - %i0: Old processor interrupt level
 */
FUNC_BEGIN __bcc_trap_sw_set_pil
	/* %l5: new %psr */
	sll	%i0, PSR_PIL_BIT, %i0
	andn	%l0, PSR_PIL, %l5
	or	%l5, %i0, %l5

	/* Update %psr */
	wr	%l5, %psr
	nop
	nop
	nop

	/* %i0: old %psr.pil */
	and	%l0, PSR_PIL, %l3
	srl	%l3, PSR_PIL_BIT, %i0

	/* GRLIB-TN-0018: assume trap was generated with "ta N; nop" */
	jmp	%l2
	 rett	%l2 + 4
FUNC_END __bcc_trap_sw_set_pil

