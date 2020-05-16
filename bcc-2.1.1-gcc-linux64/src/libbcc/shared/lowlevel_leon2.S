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

/* Low-level functions specific for LEON2 systems. */

#include "bcc/leon.h"
.include "macros.i"

	.section	".text"
	.global		bcc_set_ccr
	.global		bcc_get_ccr
	.global		bcc_flush_cache
	.global		bcc_flush_icache
	.global		bcc_flush_dcache
	.global		bcc_power_down

FUNC_BEGIN bcc_set_ccr
	sethi	%hi(LEON2_CCR), %g1
	st	%o0, [%g1 + %lo(LEON2_CCR)]
	retl
	 nop
FUNC_END bcc_set_ccr

FUNC_BEGIN bcc_get_ccr
	sethi	%hi(LEON2_CCR), %g1
	ld	[%g1 + %lo(LEON2_CCR)], %o0
	retl
	 nop
FUNC_END bcc_get_ccr

FUNC_BEGIN bcc_flush_cache
	sta	%g0, [%g0] ASI_LEON2_IFLUSH
	retl
	 sta	%g0, [%g0] ASI_LEON2_DFLUSH
FUNC_END bcc_flush_cache

FUNC_BEGIN bcc_flush_icache
	retl
	 sta	%g0, [%g0] ASI_LEON2_IFLUSH
FUNC_END bcc_flush_icache

FUNC_BEGIN bcc_flush_dcache
	retl
	 sta	%g0, [%g0] ASI_LEON2_DFLUSH
FUNC_END bcc_flush_dcache

FUNC_BEGIN bcc_power_down
	/* Enter idle mode. */
	sethi	%hi(LEON2_IDLE), %o0
	st	%g0, [%o0 + %lo(LEON2_IDLE)]
	ld	[%o0 + %lo(LEON2_IDLE)], %g0
	retl
	 clr	%o0
FUNC_END bcc_power_down

