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

int bcc_int_map_set(int busintline, int irqmpintline)
{
        if (0 == __bcc_int_handle) {
                return BCC_NOT_AVAILABLE;
        }
        if (0 == __bcc_int_irqmp_map) {
                return BCC_NOT_AVAILABLE;
        }
        DBG(
                "Remapping bus interrupt line %d to interrupt controller "
                "interrupt line %d\n",
                busintline,
                irqmpintline
        );

        volatile struct irqmp_regs *regs;

        regs = (struct irqmp_regs *) __bcc_int_handle;

        int offset;
        int index;
        uint32_t map;

        offset = busintline >> 2;
        index = ((~busintline) & 0x3) * 8;
        map = regs->map[offset];
        map &= ~(0xff << index);
        map |= (irqmpintline & 0xff) << index;
        regs->map[offset] = map;

        return BCC_OK;
}

int bcc_int_map_get(int busintline)
{
        if (0 == __bcc_int_handle) {
                return -1;
        }
        if (0 == __bcc_int_irqmp_map) {
                return -1;
        }

        volatile struct irqmp_regs *regs;

        regs = (struct irqmp_regs *) __bcc_int_handle;

        int irqmpintline;
        int offset;
        int index;
        uint32_t map;

        offset = busintline >> 2;
        index = ((~busintline) & 0x3) * 8;
        map = regs->map[offset];
        irqmpintline = (map >> index) & 0xff;

        return irqmpintline;
}

