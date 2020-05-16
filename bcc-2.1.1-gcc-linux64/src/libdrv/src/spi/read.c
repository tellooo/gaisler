/*
 * Copyright (c) 2018, Cobham Gaisler AB
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

#include "priv.h"

int spi_read(struct spi_priv *priv, void *rxbuf, int count)
{
        volatile struct spictrl_regs *regs = priv->regs;
        SPIN_IRQFLAGS(plev);
        int i = 0;
        uint32_t rxword = 0;
        union {
                void *ptr;
                uint8_t* u8;
                uint16_t* u16;
                uint32_t* u32;
        } buf = {.ptr = rxbuf};

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        while (i < count) {
                if (regs->event & SPICTRL_EVENT_NE) {
                        rxword = regs->rx;
                } else {
                        break;
                }
                if (NULL != rxbuf) {
                        rxword >>= priv->rxshift;

                        if (priv->bytelen == 1) {
                                buf.u8[i] = (uint8_t)rxword;
                        } else if (priv->bytelen == 2) {
                                buf.u16[i] = (uint16_t)rxword;
                        } else {
                                buf.u32[i] = (uint32_t)rxword;
                        }
                }
                i++;
                priv->rxfree++;
        }

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return i;
}

