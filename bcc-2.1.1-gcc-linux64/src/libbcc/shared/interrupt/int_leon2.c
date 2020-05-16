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

#include "int_leon2_priv.h"

int bcc_int_mask(int irq)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct leon2_irq_regs *regs;
        int plevel;

        regs = (struct leon2_irq_regs *) __bcc_int_handle;

        plevel = bcc_int_disable();
        regs->itmp &= ~(1<<irq & 0xFFFE);
        bcc_int_enable(plevel);

        return BCC_OK;
}

int bcc_int_unmask(int irq)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct leon2_irq_regs *regs;
        int plevel;

        regs = (struct leon2_irq_regs *) __bcc_int_handle;

        plevel = bcc_int_disable();
        regs->itmp |= (1<<irq & 0xFFFE);
        bcc_int_enable(plevel);

        return BCC_OK;
}

int bcc_int_clear(int source)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct leon2_irq_regs *regs;

        regs = (struct leon2_irq_regs *) __bcc_int_handle;
        regs->itc = (1 << source) & 0xfffe;

        return BCC_OK;
}

int bcc_int_force(int irq)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct leon2_irq_regs *regs;
        int plevel;

        regs = (struct leon2_irq_regs *) __bcc_int_handle;

        plevel = bcc_int_disable();
        regs->itf |= 1<<irq;
        bcc_int_enable(plevel);

        return BCC_OK;
}

int bcc_int_pend(int source)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }

        volatile struct leon2_irq_regs *regs;
        int plevel;

        regs = (struct leon2_irq_regs *) __bcc_int_handle;

        plevel = bcc_int_disable();
        regs->itp |= 1<<source;
        bcc_int_enable(plevel);

        return BCC_OK;
}

