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

	.section	".text"
	.global		bcc_fpu_save
	.global		bcc_fpu_restore

FUNC_BEGIN bcc_fpu_save
	/* Allocate space on stack for stdfq. Aligned to 8 byte by ABI. */
	sub	%sp, 8, %sp
	mov	%o0, %o1
	set	0xf, %o0
	ta	BCC_SW_TRAP_SET_PIL
	/* Old %psr.pil in %o0 */

	set	FSR_QNE, %o3
	/* Empty FPU deferred trap queue by reading %fsr until qne = 0. */
1:
	st	%fsr, [%o1 + fpu_state_fsr]
	ld	[%o1 + fpu_state_fsr], %o2
	/* If qne = 0, the queue is empty; if qne = 1, the queue is not empty. */
	andcc	%o2, %o3, %g0
	beq	2f
	 nop
	/* Advance queue to next entry. */
	std	%fq, [%sp + sizeof_store_area]
	ba	1b
	 nop
2:
	/* Restore %psr.pil from %o0 */
	ta	BCC_SW_TRAP_SET_PIL

	std	%f0, [%o1 + fpu_state_f0]
	B2BSTORE_FIX
	std	%f2, [%o1 + fpu_state_f2]
	B2BSTORE_FIX
	std	%f4, [%o1 + fpu_state_f4]
	B2BSTORE_FIX
	std	%f6, [%o1 + fpu_state_f6]
	B2BSTORE_FIX
	std	%f8, [%o1 + fpu_state_f8]
	B2BSTORE_FIX
	std	%f10, [%o1 + fpu_state_f10]
	B2BSTORE_FIX
	std	%f12, [%o1 + fpu_state_f12]
	B2BSTORE_FIX
	std	%f14, [%o1 + fpu_state_f14]
	B2BSTORE_FIX
	std	%f16, [%o1 + fpu_state_f16]
	B2BSTORE_FIX
	std	%f18, [%o1 + fpu_state_f18]
	B2BSTORE_FIX
	std	%f20, [%o1 + fpu_state_f20]
	B2BSTORE_FIX
	std	%f22, [%o1 + fpu_state_f22]
	B2BSTORE_FIX
	std	%f24, [%o1 + fpu_state_f24]
	B2BSTORE_FIX
	std	%f26, [%o1 + fpu_state_f26]
	B2BSTORE_FIX
	std	%f28, [%o1 + fpu_state_f28]
	B2BSTORE_FIX
	std	%f30, [%o1 + fpu_state_f30]

	add	%sp, 8, %sp
	retl
	 mov	%g0, %o0
FUNC_END bcc_fpu_save

FUNC_BEGIN bcc_fpu_restore
	/* Allocate space on stack for stdfq. Aligned to 8 byte by ABI. */
	sub	%sp, 8, %sp
	mov	%o0, %o1
	set	0xf, %o0
	ta	BCC_SW_TRAP_SET_PIL
	/* Old %psr.pil in %o0 */

	set	FSR_QNE, %o3
	/* Empty FPU deferred trap queue by reading %fsr until qne = 0. */
1:
	st	%fsr, [%sp + sizeof_store_area]
	ld	[%sp + sizeof_store_area], %o2
	/* If qne = 0, the queue is empty; if qne = 1, the queue is not empty. */
	andcc	%o2, %o3, %g0
	beq	2f
	 nop
	/* Advance queue to next entry. */
	std	%fq, [%sp + sizeof_store_area]
	ba	1b
	 nop
2:
	/* Restore %psr.pil from %o0 */
	ta	BCC_SW_TRAP_SET_PIL

	ldd	[%o1 + fpu_state_f0], %f0
	ldd	[%o1 + fpu_state_f2], %f2
	ldd	[%o1 + fpu_state_f4], %f4
	ldd	[%o1 + fpu_state_f6], %f6
	ldd	[%o1 + fpu_state_f8], %f8
	ldd	[%o1 + fpu_state_f10], %f10
	ldd	[%o1 + fpu_state_f12], %f12
	ldd	[%o1 + fpu_state_f14], %f14
	ldd	[%o1 + fpu_state_f16], %f16
	ldd	[%o1 + fpu_state_f18], %f18
	ldd	[%o1 + fpu_state_f20], %f20
	ldd	[%o1 + fpu_state_f22], %f22
	ldd	[%o1 + fpu_state_f24], %f24
	ldd	[%o1 + fpu_state_f26], %f26
	ldd	[%o1 + fpu_state_f28], %f28
	ldd	[%o1 + fpu_state_f30], %f30
	ld	[%o1 + fpu_state_fsr], %fsr

	add	%sp, 8, %sp
	retl
	 mov	%g0, %o0
FUNC_END bcc_fpu_restore

