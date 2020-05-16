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

/*
 * Reset trap handler for multi vector trapping
 *
 * This file assumes that a boot loader has performed basic initialization.
 *
 * __bcc_reset_trap_svt can be overriden by a custom boot loader.
 */

	.section	".text"
	.weak		__bcc_trap_reset_svt
	.set		__bcc_trap_reset_svt, __bcc_trap_reset_svt_default
	.global		__bcc_trap_reset_svt

FUNC_BEGIN __bcc_trap_reset_svt_default
	call    __bcc_init40
	 nop

	/* On LEON3, enable single vector trapping by setting ASR17.SV. */
	rd	%asr17, %g1
	set	ASR17_SV, %g2
	or	%g1, %g2, %g1
	/* NOTE: Delayed-write */
	wr	%g1, %asr17
	nop
	nop
	nop

	/* Initialize %tbr */
	set	__bcc_trap_table, %g1
	wr	%g1, %tbr

#ifndef _FLAT
	/* %wim := 2 */
	wr	%g0, 2, %wim
#else
	/* All windows invalid except 0 and 1 */
	wr	%g0, 0xfffffffc, %wim
#endif

	/* Remember initial %sp in case we switch windows */
	mov	%o6, %g4
	/* %psr := cwp=0, et=1, pil=0 */
	rd	%psr, %g1
	set	(PSR_PIL | PSR_ET | PSR_CWP), %g3
	andn	%g1, %g3, %g2
#ifndef _FLAT
	wr	%g2, PSR_ET, %psr
#else
	/* Start out in window 1 for flat */
	wr	%g2, (PSR_ET | 1), %psr
#endif
	nop
	nop
	nop

	mov	%g4, %o6

	sethi	%hi(__bcc_crt0), %g1
	jmp	%g1 + %lo(__bcc_crt0)
	 nop
FUNC_END __bcc_trap_reset_svt_default

