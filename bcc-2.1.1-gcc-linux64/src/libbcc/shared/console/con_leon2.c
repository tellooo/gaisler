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

#include "bcc/bcc_param.h"
#include "bcc/bcc.h"

#include "bcc/regs/apbuart.h"

/* LEON2 UART is a subset of APBUART without FIFO, it has holding registers
 * instead.
 */

int __attribute__((weak)) __bcc_con_init(void)
{
        return BCC_OK;
}

int __attribute__((weak)) __bcc_con_outbyte(char x)
{
        if (0 == __bcc_con_handle) { return 0; }

        volatile struct apbuart_regs *regs = (void *) __bcc_con_handle;

        while (0 == (regs->status & APBUART_STATUS_TE));
        regs->data = x & 0xff;

        return 0;
}

char __attribute__((weak)) __bcc_con_inbyte(void)
{
        if (0 == __bcc_con_handle) { return 0; }

        volatile struct apbuart_regs *regs = (void *) __bcc_con_handle;

        while (0 == (regs->status & APBUART_STATUS_DR));

        return regs->data & 0xff;
}

