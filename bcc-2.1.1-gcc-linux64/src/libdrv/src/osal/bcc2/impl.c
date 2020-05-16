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

/* Operating System Abstraction Layer for BCC 2.0.x */

#include <stdint.h>
#include <stdlib.h>

#include <drv/osal.h>
#include <drv/nelem.h>

#include <bcc/bcc.h>
#include <bcc/bcc_param.h>

static int osal_isr_translate(int source)
{
        int ret;

        ret = bcc_int_map_get(source);
        if (ret < 0) {
                ret = source;
        }
        return ret;
}

int osal_isr_register(
        struct osal_isr_ctx *ctx,
        int source,
        osal_isr_t handler,
        void *arg
)
{
        int irqmpsrc;

        irqmpsrc = osal_isr_translate(source);
        if (irqmpsrc < 0) {
                return DRV_FAIL;
        }

        ctx->node.handler = (void (*)(void *, int)) handler;
        ctx->node.source = irqmpsrc;
        ctx->node.arg = arg;

        if (bcc_isr_register_node(&ctx->node) != BCC_OK) {
                return DRV_FAIL;
        }

        /* clear pending interrupts */
        bcc_int_clear(irqmpsrc);
        bcc_int_unmask(irqmpsrc);

        return DRV_OK;
}

int osal_isr_unregister(
        struct osal_isr_ctx *ctx,
        int source,
        osal_isr_t handler,
        void *arg
)
{
        UNUSED(arg);
        UNUSED(handler);
        int irqmpsrc;

        irqmpsrc = osal_isr_translate(source);
        if (irqmpsrc < 0) {
                return DRV_FAIL;
        }

        bcc_int_mask(irqmpsrc);

        if (bcc_isr_unregister_node(&ctx->node) != BCC_OK) {
                return DRV_FAIL;
        }

        return DRV_OK;
}

