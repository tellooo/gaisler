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
 * FTMCTRL PROM 32+8 bit access
 *
 * This is an implementation of the mbdrv interface for the FTMCTRL PROM bus
 * with support for 32-bit data bus and an 8-bit checkbit bus.
 *
 * The implementation is designed to execute in an isolated "safe" environment
 * where access to external RAM is eliminated. This is important because the
 * checkbit bypass interface of FTMCTRL applies to all memory banks.
 */

#include <bcc/bcc.h>
#include <mbdrv.h>
#include <ftmctrl-regs.h>

/*
 * NOTES
 *   - TCB are generated and written automatically by memory controller when wb=0
 *     (and promedac=1). So data can be written as normal.
 *
 *   - Read bypass does not work when promedac=0. This means that we have to
 *     take the trap.
 */

int safe_write32plus8(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t data,
        uint32_t tcb
)
{
        volatile struct ftmctrl_regs *regs = ctx->mctrl;
        volatile uint32_t *dest = (uint32_t *) addr;
        uint32_t mcfg3;

        mcfg3 = regs->mcfg3;
        regs->mcfg3 = (mcfg3 & ~FTMCTRL_MCFG3_TCB) | FTMCTRL_MCFG3_WB | tcb;
        *dest = data;
        regs->mcfg3 = mcfg3 & ~FTMCTRL_MCFG3_WB;

        return MBDRV_RET_OK;
}

/* Number of data access exception traps */
volatile int safe_lasttrap;
/*
 * Read from bus and report trap was generated
 *
 * trap: 0 - no trap, else trap number encountered during the read
 */
int safe_read32plus8(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t *data,
        uint32_t *tcb,
        int *trap
)
{
        volatile struct ftmctrl_regs *regs = ctx->mctrl;
        uint32_t mcfg3;
        uint32_t d, t;

        safe_lasttrap = 0;
        mcfg3 = regs->mcfg3;
        regs->mcfg3 = mcfg3 | FTMCTRL_MCFG3_RB;
        d = bcc_loadnocache((uint32_t *) addr);
        mcfg3 = regs->mcfg3;
        regs->mcfg3 = mcfg3 & ~FTMCTRL_MCFG3_RB;
        t = mcfg3 & FTMCTRL_MCFG3_TCB;
        if (data) {
                *data = d;
        }
        if (tcb) {
                *tcb = t;
        }
        if (trap) {
                *trap = safe_lasttrap;
        }

        return MBDRV_RET_OK;
}

int safe_write32data(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t data
)
{
        (void) sizeof ctx;
        volatile uint32_t *dest = (uint32_t *) addr;

        *dest = data;

        return MBDRV_RET_OK;
}

int safe_read32(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t *data
)
{
        volatile struct ftmctrl_regs *regs = ctx->mctrl;
        uint32_t mcfg3;
        uint32_t d;

        mcfg3 = regs->mcfg3;
        regs->mcfg3 = mcfg3 & ~FTMCTRL_MCFG3_PE;
        d = bcc_loadnocache((uint32_t *) addr);
        regs->mcfg3 = mcfg3;
        if (data) {
                *data = d;
        }

        return MBDRV_RET_OK;
}

extern int trap_write32plus8(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t data,
        uint32_t tcb
);
extern int trap_read32plus8(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t *data,
        uint32_t *tcb,
        int *trap
);
extern int trap_write32data(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t data
);
extern int trap_read32(
        struct mbdrv_ctx *ctx,
        uint32_t addr,
        uint32_t *data
);

/* This shall be published to the application. */
struct mbdrv_ops __safe_user = {
        .write32plus8   = trap_write32plus8,
        .read32plus8    = trap_read32plus8,
        .write32data    = trap_write32data,
        .read32         = trap_read32,
};

