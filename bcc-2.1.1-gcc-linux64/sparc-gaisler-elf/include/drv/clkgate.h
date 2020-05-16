/*
 * Copyright (c) 2019, Cobham Gaisler AB
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

#ifndef DRV_CLKGATE_H
#define DRV_CLKGATE_H

#include <stdint.h>
#include <drv/regs/clkgate.h>
#include <drv/auto.h>
#include <drv/osal.h>

struct clkgate_devcfg;
struct clkgate_priv;

/* return: number of devices registered */
int clkgate_autoinit(void);
/* Register one device */
int clkgate_register(struct clkgate_devcfg *devcfg);
/* Register an array of devices */
int clkgate_init(struct clkgate_devcfg *devcfgs[]);

int clkgate_dev_count(void);

const struct drv_devreg *clkgate_get_devreg(int dev_no);

struct clkgate_priv *clkgate_open(int dev_no);
int clkgate_close(struct clkgate_priv *priv);

/* Gate the clock for cores in coremask. */
int clkgate_gate(
        struct clkgate_priv *priv,
        uint32_t coremask
);

/* Enable the clock and reset cores in coremask. */
int clkgate_enable(
        struct clkgate_priv *priv,
        uint32_t coremask
);

/*
 * Get enable status of cores
 *
 * enabled:  Output mask of cores which are enabled.
 * disabled: Output mask of cores which are disabled.
 */
int clkgate_status(
        struct clkgate_priv *priv,
        uint32_t *enabled,
        uint32_t *disabled
);

/*
 * Control CPU/FPU override register
 *
 * set: 0 - do not write register, others - write register
 * newval: Value to write to register
 * return: Register content before newval is written
 */
uint32_t clkgate_override(
        struct clkgate_priv *priv,
        int set,
        uint32_t newval
);


/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct clkgate_priv {
        volatile struct clkgate_regs *regs;
        uint8_t open;
};

struct clkgate_devcfg {
        struct drv_devreg regs;
        struct clkgate_priv priv;
};

#endif

