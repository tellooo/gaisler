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

int bcc_send_interrupt(int level, int cpuid)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct irqmp_regs *regs;

        regs = (struct irqmp_regs *) __bcc_int_handle;
        regs->piforce[cpuid] = (1 << level) & IRQMP_PIFORCE_IF;

        return BCC_OK;
}

int bcc_get_cpu_count(void)
{
        if (0 == __bcc_int_handle) {
                return -1;
        }

        volatile struct irqmp_regs *regs;
        int count;

        regs = (struct irqmp_regs *) __bcc_int_handle;
        count = (
                (regs->mpstat & IRQMP_MPSTAT_NCPU) >>
                IRQMP_MPSTAT_NCPU_BIT
        ) + 1;

        return count;
}

int bcc_start_processor(int cpuid)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct irqmp_regs *regs;

        DBG("Waking CPU%d\n", cpuid);
        regs = (struct irqmp_regs *) __bcc_int_handle;
        regs->mpstat = (1 << cpuid) & IRQMP_MPSTAT_STATUS;

        return BCC_OK;
}

