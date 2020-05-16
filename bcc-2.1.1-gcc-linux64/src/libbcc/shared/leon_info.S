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

#include "include/bcc/leon.h"
.include "macros.i"

	.section	".text"
	.global		__bcc_get_leon_info
	.global		bcc_get_cpuid

/* Probe LEON characteristics: LEON version and number of windows. */
FUNC_BEGIN __bcc_get_leon_info
	rd	%psr, %o0
	srl	%o0, PSR_VER_BIT, %o1
	and	%o1, 0xf, %o0
	cmp	%o0, 3
	beq	.Lleon34
	 nop

	cmp	%o0, 5
	beq	.Lleon5
	 nop

	bne	.Lleon2
	 nop

.Lleon34:
.Lleon5:
	/* LEON3 (and LEON4) */
	/* nwindows_min1 in %o1 */
	rd	%asr17, %o1
	srl	%o1, ASR17_NWIN_BIT, %o1
	rd	%asr17, %o2
	set	ASR17_INDEX, %o3
	and	%o2, %o3, %o2
	srl	%o2, ASR17_INDEX_BIT, %o2
	ba	.Lwriteinfo
	 nop

.Lleon2:
	set	2, %o0
	sethi	%hi(LEON2_PCR), %o2
	ld	[%o2 + %lo(LEON2_PCR)], %o3
	srl	%o3, LEON2_PCR_NWIN_BIT, %o1
	set	0, %o2

.Lwriteinfo:
	/* %o0: LEON version, %o1: nwindows_min1, %o2: cpuid */
	and	%o1, 0x1f, %o1
	add	%o1, 1, %o4
	set	__bcc_leon_version, %o3
	st	%o0, [%o3 + 0x0]
	st	%o1, [%o3 + 0x4]
	st	%o4, [%o3 + 0x8]
	st	%o2, [%o3 + 0xc]

	retl
	 nop
FUNC_END __bcc_get_leon_info


FUNC_BEGIN bcc_get_cpuid
	sethi	%hi(__bcc_cpuid), %g1
	ld	[%g1 + %lo(__bcc_cpuid)], %o0
	retl
	 nop
FUNC_END bcc_get_cpuid


	.section	".data"
	.align 4
	.global		__bcc_leon_version
	.global		__bcc_nwindows
	.global		__bcc_nwindows_min1
	.global		__bcc_cpuid

__bcc_leon_version:
	.word 3
__bcc_nwindows_min1:
	.word 7
__bcc_nwindows:
	.word 8
__bcc_cpuid:
	.word 0

