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

/*
 * Memory bus bus access interface
 */

#ifndef MBDRV_H
#define MBDRV_H

#include <stdint.h>

enum {
        MBDRV_RET_OK,
        MBDRV_RET_FAIL,
};

struct mbdrv_ctx;
struct mbdrv_ops {
        /* addr: bus address */
        int (*write32plus8)(
                struct mbdrv_ctx *ctx,
                uint32_t addr,
                uint32_t data,
                uint32_t tcb
        );

        /*
         * Read from bus and report trap if generated
         *
         * - tcb is always valid at return.
         * - data is valid only if trap is zero.
         * trap:
         *   - 0:       no trap
         *   - nonzero: some trap occured during the read. data is not valid
         *     but tcb is.
         */
        int (*read32plus8) (
                struct mbdrv_ctx *ctx,
                uint32_t addr,
                uint32_t *data,
                uint32_t *tcb,
                int *trap
        );

        /*
         * Write data and appropriate TCB to memory. TCB could be calculated by
         * hardware or software.
         */
        int (*write32data)(
                struct mbdrv_ctx *ctx,
                uint32_t addr,
                uint32_t data
        );

        /*
         * Read raw data (typically with EDAC disabled)
         */
        int (*read32) (
                struct mbdrv_ctx *ctx,
                uint32_t addr,
                uint32_t *data
        );
};

struct mbdrv_ctx {
        struct mbdrv_ops *ops;
        /* Base register address of FTMCTRL */
        void *mctrl;
};

static inline int mbdrv_write32plus8(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t data,
        uint32_t tcb
)
{
        return ctx->ops->write32plus8(ctx, addr, data, tcb);
}

static inline int mbdrv_read32plus8(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t *data,
        uint32_t *tcb,
        int *trap
)
{
        return ctx->ops->read32plus8(ctx, addr, data, tcb, trap);
}

static inline int mbdrv_write32data(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t data
)
{
        return ctx->ops->write32data(ctx, addr, data);
}

static inline int mbdrv_read32(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t *data
)
{
        return ctx->ops->read32(ctx, addr, data);
}

#endif

