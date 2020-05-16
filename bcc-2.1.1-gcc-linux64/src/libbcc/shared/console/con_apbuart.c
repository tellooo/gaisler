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

enum {
        FIFO_UNKNOWN,
        FIFO_YES,
        FIFO_NO,
};

static int fifoinfo = FIFO_UNKNOWN;

int __attribute__((weak)) __bcc_con_outbyte(char x)
{
        if (0 == __bcc_con_handle) { return 0; }

        volatile struct apbuart_regs *regs = (void *) __bcc_con_handle;
        int fi;

        /* Use transmitter FIFO if available */
again:
        fi = fifoinfo;
        if (FIFO_YES == fi) {
                /* Transmitter FIFO full flag is available */
                while (regs->status & APBUART_STATUS_TF);
        } else if (FIFO_NO == fi) {
                /*
                 * Transmitter "hold register empty" AKA "FIFO empty" flag is
                 * available
                 */
                while (!(regs->status & APBUART_STATUS_HOLD_REGISTER_EMPTY));
        } else {
                /* First time: probe */
                if (regs->ctrl & APBUART_CTRL_FA) {
                        fifoinfo = FIFO_YES;
                } else {
                        fifoinfo = FIFO_NO;
                }
                goto again;
        }

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

