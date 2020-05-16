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

#include <stdint.h>
#include "bcc/bcc_param.h"
#include "bcc/regs/apbuart.h"

extern const unsigned int __bsp_sysfreq;
static const unsigned int BSP_BAUD = 38400;

int __bcc_con_init(void)
{
        if (0 == __bcc_con_handle)      { return BCC_NOT_AVAILABLE; }
        if (0 == __bsp_sysfreq)         { return BCC_NOT_AVAILABLE; }

        volatile struct apbuart_regs *regs = (void *) __bcc_con_handle;
        const uint32_t APBUART_DEBUG_MASK = APBUART_CTRL_DB | APBUART_CTRL_FL;
        uint32_t dm;
        uint32_t scaler;

        /* NOTE: CTRL_FL has reset value 0. CTRL_DB has no reset value. */
        dm = regs->ctrl & APBUART_DEBUG_MASK;
        if (dm == APBUART_DEBUG_MASK) {
                /* Debug mode enabled so assume APBUART already initialized. */
                return BCC_OK;
        }

        scaler = (((__bsp_sysfreq * 10) / (BSP_BAUD * 8)) - 5) / 10;
        regs->scaler = scaler;
        regs->ctrl = (
                APBUART_CTRL_TE |
                APBUART_CTRL_RE
        );
        regs->status = 0;

        return BCC_OK;
}

