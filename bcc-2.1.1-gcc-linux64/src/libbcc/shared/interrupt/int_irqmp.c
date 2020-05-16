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

#include "int_irqmp_priv.h"

int bcc_int_mask(int source)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct irqmp_regs *regs =
            (struct irqmp_regs *) __bcc_int_handle;
        volatile uint32_t *maskreg = &regs->pimask[__bcc_cpuid];
        uint32_t keepbits = ~(1 << source);
        int plevel;

        plevel = bcc_int_disable();
        *maskreg &= keepbits;
        bcc_int_enable(plevel);

        return BCC_OK;
}

int bcc_int_unmask(int source)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct irqmp_regs *regs =
            (struct irqmp_regs *) __bcc_int_handle;
        volatile uint32_t *maskreg = &regs->pimask[__bcc_cpuid];
        uint32_t setbit = (1 << source);
        int plevel;

        plevel = bcc_int_disable();
        *maskreg |= setbit;
        bcc_int_enable(plevel);

        return BCC_OK;
}

int bcc_int_clear(int source)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct irqmp_regs *regs;

        regs = (struct irqmp_regs *) __bcc_int_handle;
        regs->iclear = (1 << source) & (IRQMP_ICLEAR_EIC | IRQMP_ICLEAR_IC);

        return BCC_OK;
}

int bcc_int_force(int level)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct irqmp_regs *regs;

        regs = (struct irqmp_regs *) __bcc_int_handle;
        if (0 == (regs->mpstat & IRQMP_MPSTAT_NCPU) >> IRQMP_MPSTAT_NCPU_BIT) {
                /* NCPU = 0 so piforce is not available. */
                /* NOTE: No read-modify-write */
                regs->iforce0 = (1 << level) & IRQMP_IFORCE0_IF;
        } else {
                /* NCPU != 0 so piforce is available. */
                regs->piforce[__bcc_cpuid] = (1 << level) & IRQMP_PIFORCE_IF;
        }

        return BCC_OK;
}

int bcc_int_pend(int source)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct irqmp_regs *regs =
            (struct irqmp_regs *) __bcc_int_handle;
        volatile uint32_t *ipendreg = &regs->ipend;
        uint32_t setbit = (1 << source);
        int plevel;

        plevel = bcc_int_disable();
        *ipendreg |= setbit;
        bcc_int_enable(plevel);

        return BCC_OK;
}

