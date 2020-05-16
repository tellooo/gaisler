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

.include "macros.i"

#define TBR_TT_MASK     0xFF0   /* Trap type mask from tbr */
#define TBR_TT_SHIFT    4       /* Shift to get trap type */

	.section	".text.traptable"
	.global		__bcc_trap_table
	.global		__bcc_trap_table_svt_bad
	.global		__bcc_entry_point

FUNC_BEGIN __bcc_trap_table
	rd	%tbr, %l6
	and	%l6, TBR_TT_MASK, %l6
	srl	%l6, TBR_TT_SHIFT, %l6

#ifndef __FIX_LEON3FT_TN0018
	sethi	%hi(__bcc_trap_table_svt_level0), %l3
	or	%l3, %lo(__bcc_trap_table_svt_level0), %l3
#else
	sethi	%hi(__bcc_trap_table_svt_level0_tn0018), %l3
	or	%l3, %lo(__bcc_trap_table_svt_level0_tn0018), %l3
#endif

	and	%l6, 0xF0, %l5
	srl	%l5, 2, %l5                     ! Table offset
	ld	[%l3+%l5], %l3                  ! Fetch subtable

	and	%l6, 0x0F, %l5
	sll	%l5, 2, %l5                     ! Table offset
	ld	[%l3+%l5], %l3                  ! Fetch trap handler

	/*
	 * %l0 = psr
	 * %l1 = pc (except at cold restart)
	 * %l2 = npc (except at cold restart)
	 * %l3 = Trap handler
	 * %l6 = Trap type
	 */
	jmp	%l3                             ! Dispatch
	 rd	%psr, %l0
FUNC_END __bcc_trap_table

	/* Alignment allows reset trap as IRQMP.BADDRn entry point. */
	.align	8
FUNC_BEGIN __bcc_entry_point
	mov	%g0, %g4
	sethi	%hi(__bcc_trap_reset_svt), %g4
	jmp	%g4+%lo(__bcc_trap_reset_svt)
	 nop
FUNC_END __bcc_entry_point

	.global		__bcc_trap_table_type
	.set		__bcc_trap_table_type, 1

