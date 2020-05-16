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

#ifndef DRV_MEMPROT_H
#define DRV_MEMPROT_H

#include <stdint.h>
#include <drv/regs/grmemprot.h>
#include <drv/auto.h>
#include <drv/osal.h>

struct memprot_devcfg;
struct memprot_priv;

/* return: number of devices registered */
int memprot_autoinit(void);
/* Register one device */
int memprot_register(struct memprot_devcfg *devcfg);
/* Register an array of devices */
int memprot_init(struct memprot_devcfg *devcfgs[]);

int memprot_dev_count(void);

const struct drv_devreg *memprot_get_devreg(int dev_no);

struct memprot_priv *memprot_open(int dev_no);
int memprot_close(struct memprot_priv *priv);

/* Disables all segments and the global sets global enable bit to 0 */
int memprot_reset(
        struct memprot_priv *priv
);

int memprot_start(
        struct memprot_priv *priv
);

int memprot_stop(
        struct memprot_priv *priv
);

/*
 * return: 1: started, 0: not started
 */
int memprot_isstarted(
        struct memprot_priv *priv
);

/* user representation of one memprot segment */
struct memprot_seginfo {
        uintptr_t start;
        uintptr_t end;
        uint32_t g;
        int en;
};

/* Return number of segments */
int memprot_nseg(
        struct memprot_priv *priv
);

int memprot_set(
        struct memprot_priv *priv,
        int segment,
        const struct memprot_seginfo *seginfo
);

int memprot_get(
        struct memprot_priv *priv,
        int segment,
        struct memprot_seginfo *seginfo
);

#if 0
/* Enable protection for segment */
int memprot_enable(
        struct memprot_priv *priv,
        int segment
);

/* Disable protection for segment */
int memprot_disable(
        struct memprot_priv *priv,
        int segment
);
#endif


/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct memprot_priv {
        volatile struct grmemprot_regs *regs;
        uint8_t open;
};

struct memprot_devcfg {
        struct drv_devreg regs;
        struct memprot_priv priv;
};

#endif

