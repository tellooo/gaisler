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

#ifndef DRV_AHBSTAT_H
#define DRV_AHBSTAT_H

/*
 * Driver for AHBSTAT. Allows the user to register an interrupt handler which
 * is called on new errors.
 *
 * Recommended reading when using this driver is the application note
 *   GRLIB-AN-0004 "Handling of External Memory EDAC Errors in LEON/GRLIB
 *   Systems".
 */

#include <stdint.h>
#include <drv/regs/ahbstat.h>
#include <drv/auto.h>
#include <drv/osal.h>

struct ahbstat_devcfg;
struct ahbstat_priv;

/* return: number of devices registered */
int ahbstat_autoinit(void);
/* Register one device */
int ahbstat_register(struct ahbstat_devcfg *devcfg);
/* Register an array of devices */
int ahbstat_init(struct ahbstat_devcfg *devcfgs[]);

int ahbstat_dev_count(void);

const struct drv_devreg *ahbstat_get_devreg(int dev_no);

struct ahbstat_priv *ahbstat_open(int dev_no);
int ahbstat_close(struct ahbstat_priv *priv);

static inline volatile struct ahbstat_regs *ahbstat_get_regs(
        struct ahbstat_priv *priv
);

/* User callback from ISR */
int ahbstat_set_user(
        struct ahbstat_priv *,
        int (*userhandler)(
                volatile struct ahbstat_regs *regs,
                uint32_t status,
                uint32_t failing_address,
                void *userarg
        ),
        void *userarg
);


/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct ahbstat_priv {
        volatile struct ahbstat_regs *regs;
        int (*userhandler)(
                volatile struct ahbstat_regs *regs,
                uint32_t status,
                uint32_t failing_address,
                void *userarg
        );
        void *userarg;
        int interrupt;
        struct osal_isr_ctx isr_ctx;
        uint8_t open;
};

struct ahbstat_devcfg {
        struct drv_devreg regs;
        struct ahbstat_priv priv;
};

static inline volatile struct ahbstat_regs *ahbstat_get_regs(
        struct ahbstat_priv *priv
)
{
        return priv->regs;
}

#endif

