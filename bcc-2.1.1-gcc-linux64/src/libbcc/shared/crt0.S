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

	.section	".text"
	.global		__bcc_crt0

/*
 * At entry to __crt0_entry, the following is assumed:
 * - %sp points to top of stack
 * - %wim and %psr.cwp are valid
 * - %psr.et=1
 */
FUNC_BEGIN __bcc_crt0
	call    __bcc_init50
	 nop

	/*
	 * According to SPARC ABI, Chapter 3: The system marks the deepest
	 * stack frame by setting the frame pointer to zero. No other frame's
	 * %fp has a zero value.
	 */
	/* NOTE: Assuming %sp is already set by boot loader. */
	and	%sp, 0xfffffff0, %l0
	sub	%l0, 96, %sp
	clr	%fp
	clr	%i7

	/*
	 * If the symbol __bcc_cfg_skip_clear_bss has non-zero value, then
	 * clear .bss. Otherwise do not clear .bss.
	 */
	set	__bcc_cfg_skip_clear_bss, %o4
	cmp	%o4, %g0
	bne	.Lskip_clear_bss
	/* Clear bss */
	 set	__bss_start, %o0
	set	__bss_end, %o2
	/* Number of double words. */
	sub	%o2, %o0, %o3
	srl	%o3, 3, %o1
	call	bcc_dwzero
	 nop
.Lskip_clear_bss:

	sethi	%hi(__bcc_sp_at_entry), %l1
	st	%l0, [%l1 + %lo(__bcc_sp_at_entry)]

	/*
	 * Copy .data from Load Memory Address (LMA) to Virtual Memory Address
	 * (VMA) if needed.
	 */
	call	__bcc_copy_data
	 nop

	/* .data can be referenced only after return from __bcc_copy_data(). */

	call	__bcc_init60
	 nop

	call	__bcc_get_leon_info
	 nop

	call	__bcc_con_init
	 nop

	call	__bcc_timer_init
	 nop

	call	__bcc_int_init
	 nop

	set	_fini, %o0
	call	atexit
	 nop

	call	_init
	 nop

	call    __bcc_init70
	 nop

	sethi	%hi(__bcc_argc), %l0
	ld	[%l0 + %lo(__bcc_argc)], %o0
	sethi	%hi(__bcc_argvp), %l1
	ld	[%l1 + %lo(__bcc_argvp)], %o1
	call    main
	/* In case someone tries to reach environment. */
	 clr	%o2

	call	exit
	 nop

	mov	2, %g1
	ta	0x01
FUNC_END __bcc_crt0

	.section	.bss
	.global		__bcc_sp_at_entry
	.align		4
__bcc_sp_at_entry:
	.word	0

