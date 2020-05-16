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
#include "bcc/bcc.h"
#include "agga4.h"

struct agga4_uart_regs {
    volatile uint32_t Tx_SAP;
    volatile uint32_t Tx_EAP;
    volatile uint32_t Tx_CAP;
    volatile uint32_t Rx_SAP;
    volatile uint32_t Rx_EAP;
    volatile uint32_t Rx_CAP;
    volatile uint32_t Status;
    volatile uint32_t Ctrl;
    volatile uint32_t Scaler;
};

static inline void con_agga4_wait(volatile uint32_t *cap, uint32_t eap)
{
    while (__agga4_reg32(cap) <= eap) {
            __asm__ volatile ("nop;nop;nop;");
    }
}

int __attribute__((weak)) __bcc_con_init(void)
{
        return BCC_OK;
}

int __attribute__((weak)) __bcc_con_outbyte(char x)
{
        if (0 == __bcc_con_handle) { return 0; }

        /* SAP need to be 32-bit aligned */
        volatile char __attribute__((aligned(4))) buf[4];
        struct agga4_uart_regs *regs;

        buf[0] = x;
        regs = (void *) __bcc_con_handle;
        __agga4_wreg32(&regs->Tx_SAP, (uint32_t)&buf[0]);
        __agga4_wreg32(&regs->Tx_EAP, (uint32_t)&buf[0]);
        con_agga4_wait(&regs->Tx_CAP, (uint32_t)&buf[0]);

        return 0;
}

char __attribute__((weak)) __bcc_con_inbyte(void)
{
        if (0 == __bcc_con_handle) { return 0; }

        /* SAP need to be 32-bit aligned */
        volatile char __attribute__((aligned(4))) buf[4];
        struct agga4_uart_regs *regs;

        regs = (void *) __bcc_con_handle;
        __agga4_wreg32(&regs->Rx_SAP, (uint32_t)&buf[0]);
        __agga4_wreg32(&regs->Rx_EAP, (uint32_t)&buf[0]);
        con_agga4_wait(&regs->Rx_CAP, (uint32_t)&buf[0]);

        return buf[0];
}

