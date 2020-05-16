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

#include <bcc/leon.h>

#ifndef _FLAT
 #error example requires -mflat!
#endif

.macro FUNC_BEGIN name
        .global \name
        .align 4
        .type \name, #function
        \name:
.endm

.macro FUNC_END name
        .size \name, .-\name
.endm

FUNC_BEGIN __bcc_init50
        set     fl_isr_vectors, %g7
        wr      %g0, (1<<2), %wim
        retl
         nop
FUNC_END __bcc_init50

/* Interrupt trap handler for interrupt 1..15 */
FUNC_BEGIN __bcc_trap_interrupt_flat_svt
        /*
         * We came from an SVT trap dispatcher with trap type in %l6 so
         * calculate irl.
         */
        sub     %l6, 0x10, %l3
FUNC_BEGIN __bcc_trap_interrupt_flat
        /*
         * Register usage
         *
         * %l0: psr on entry (set in trap table)
         * %l1: pc (trapped)
         * %l2: npc (trapped)
         * %l3: irl (SPARC interrupt request level, set in trap table)
         * %l4: irl << PIL_BIT
         * %o5: IRL * 8 (indexes into vectors (%g7) for user interrupt handler)
         * %o0: psr on entry with int enable
         *
         * %l3, %l4, %l5, %l6, %l7: Backup of %g1..%g4 and %y
         */

        /* Calculate new PIL and PSR */
        andn    %l0, PSR_PIL, %o0
        sll     %l3, PSR_PIL_BIT, %l4
        /* For indexing into interrupt vectors */
        or      %l4, %o0, %o0

        /* Enable traps and higher interrupts. xor ET in: we know it is zero. */
        wr      %o0, PSR_ET, %psr

        /*
         * Get user's handler by indexing into "vectors". %g7 (not used by ABI)
         * has been setup to point to it in __bcc_init50().
         */
        sll     %l3, 3, %o5
        rd      %y, %l3
        mov     %g4, %l4
        /* %o0: arg, %o1: user handler entry point */
        ldd     [%g7 + %o5], %o0

        /*
         * Protect global registers from nested interrupts. %l and %i regs are
         * preserved by (flat) ABI.
         */
        mov     %g1, %l5
        mov     %g2, %l6
        mov     %g3, %l7

        call    %o1
         add    %fp, -96, %sp

        /*
         * "ET and PIL appear to be written immediately with respect to
         * interrupts." So there should be no more interrupt after the wrpsr
         * instruction. Furthermore, CWP do not change. We do not use ICC after
         * wrpsr.
         *
         * The PSR value written is the "psr on trap entry".
         */
        wr      %l0, %psr
        wr      %l3, %y
        mov     %l4, %g4
        mov     %l5, %g1
        mov     %l6, %g2
        mov     %l7, %g3
        jmp     %l1
         rett   %l2

FUNC_END __bcc_trap_interrupt_flat
FUNC_END __bcc_trap_interrupt_flat_svt

