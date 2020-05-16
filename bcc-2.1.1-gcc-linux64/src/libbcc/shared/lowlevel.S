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

/* Low-level functions common for all LEON systems. */

#include "bcc/leon.h"
.include "macros.i"

	.section	".text"
	.global		bcc_loadnocache
	.global		bcc_loadnocache8
	.global		bcc_loadnocache16
	.global		bcc_ldstub
	.global		bcc_set_psr
	.global		bcc_get_psr
	.global		bcc_get_pil
	.global		bcc_set_tbr
	.global		bcc_get_tbr

FUNC_BEGIN bcc_loadnocache
	lda	[%o0] ASI_MISS, %o0
	retl
	 nop
FUNC_END bcc_loadnocache

FUNC_BEGIN bcc_loadnocache16
	lduha	[%o0] ASI_MISS, %o0
	retl
	 nop
FUNC_END bcc_loadnocache16

FUNC_BEGIN bcc_loadnocache8
	lduba	[%o0] ASI_MISS, %o0
	retl
	 nop
FUNC_END bcc_loadnocache8

	.align	16
FUNC_BEGIN bcc_ldstub
	ldstub	[%o0], %o0
	retl
	 nop
FUNC_END bcc_ldstub

FUNC_BEGIN bcc_set_psr
	wr	%o0, %psr
	nop
	nop
	nop
	retl
	 nop
FUNC_END bcc_set_psr

FUNC_BEGIN bcc_get_psr
	retl
	 rd	%psr, %o0
FUNC_END bcc_get_psr

FUNC_BEGIN bcc_get_pil
	rd	%psr, %o2
	and	%o2, PSR_PIL, %o1
	retl
	 srl	%o1, PSR_PIL_BIT, %o0
FUNC_END bcc_get_pil

FUNC_BEGIN bcc_set_tbr
	wr	%o0, %tbr
	nop
	nop
	nop
	retl
	 nop
FUNC_END bcc_set_tbr

FUNC_BEGIN bcc_get_tbr
	retl
	 rd	%tbr, %o0
FUNC_END bcc_get_tbr

