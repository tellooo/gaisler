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

	.section ".text"
	.global __bcc_trap_window_overflow
	.global __bcc_trap_window_underflow

FUNC_BEGIN __bcc_trap_window_overflow
	/* Enter the window to be stored. */
	save
	/* Save local register set. */
	std	%l0, [%sp + 0x00]
	set	__bcc_nwindows_min1, %l0
	std	%l2, [%sp + 0x08]
	ld	[%l0], %l1
	std	%l4, [%sp + 0x10]
	rd	%wim, %l3
	std	%l6, [%sp + 0x18]
	/* l2 := WIM << (__bcc_nwindows_min1) */
	sll	%l3, %l1, %l2
	/* Save input register set. */
	std	%i0, [%sp + 0x20]
	/* l3 := WIM >> 1 */
	srl	%l3, 1, %l3
	std	%i2, [%sp + 0x28]
	/* WIM := (WIM >> 1) ^ (WIM << (__bcc_nwindows_min1)) */
	wr	%l3, %l2, %wim
	/* NOTE: 3 instruction before restore */
	std	%i4, [%sp + 0x30]
	nop
	std	%i6, [%sp + 0x38]
	/* Go back to trap window. */
	restore
	/* Re-execute save. */
	jmp	%l1
	 rett	%l2
FUNC_END __bcc_trap_window_overflow

FUNC_BEGIN __bcc_trap_window_underflow
	set	__bcc_nwindows_min1, %l6
	/* l7 := __bcc_nwindows_min1 */
	ld	[%l6], %l7
	rd	%wim, %l3
	/* l4 := WIM << 1 */
	sll	%l3, 1, %l4
	/* l5 := WIM >> (__bcc_nwindows_min1) */
	srl	%l3, %l7, %l5
	/* WIM := (WIM << 1) ^ (WIM >> (__bcc_nwindows_min1)) */
	wr	%l4, %l5, %wim
	/* WIM is implicitly read so nops are needed. */
	nop
	nop
	nop

	/* Enter the window to restore requires two restore instructions. */
	restore
	restore
	ldd	[%sp + 0x00], %l0
	ldd	[%sp + 0x08], %l2
	ldd	[%sp + 0x10], %l4
	ldd	[%sp + 0x18], %l6
	ldd	[%sp + 0x20], %i0
	ldd	[%sp + 0x28], %i2
	ldd	[%sp + 0x30], %i4
	ldd	[%sp + 0x38], %i6
	/* Go back to the trap window. */
	save
	save
	/* Re-execute restore. */
	jmp	%l1
	 rett	%l2
FUNC_END __bcc_trap_window_underflow

