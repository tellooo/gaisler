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

#include <stdint.h>
#include <stdlib.h>

#include <drv/clkgate.h>
#include <drv/osal.h>
#include <drv/drvret.h>
#include <drv/nelem.h>

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int clkgate_register(struct clkgate_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int clkgate_init(struct clkgate_devcfg *devcfgs[])
{
        struct clkgate_devcfg **dev = &devcfgs[0];

        while (*dev) {
                clkgate_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int clkgate_dev_count(void)
{
        return dev_count;
}

const struct drv_devreg *clkgate_get_devreg(int dev_no)
{
        const struct
            clkgate_devcfg
            *dev =
            (const struct clkgate_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}

struct clkgate_priv *clkgate_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }

        struct clkgate_devcfg *dev =
            (struct clkgate_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct clkgate_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        priv->regs = (struct clkgate_regs *) dev->regs.addr;

        return priv;
}

int clkgate_close(struct clkgate_priv *priv)
{
        priv->open = 0;
        return DRV_OK;
}

int clkgate_gate(
        struct clkgate_priv *priv,
        uint32_t coremask
)
{
        volatile struct clkgate_regs *regs = priv->regs;

        regs->unlock = coremask;
        regs->reset = coremask;
        regs->enable = 0;
        regs->unlock = 0;

        return DRV_OK;
}

int clkgate_enable(
        struct clkgate_priv *priv,
        uint32_t coremask
)
{
        volatile struct clkgate_regs *regs = priv->regs;

        coremask &= ~(regs->enable);
        regs->unlock = coremask;
        regs->reset = coremask;
        regs->enable = coremask;
        regs->reset = 0;
        regs->unlock = 0;

        return DRV_OK;
}

int clkgate_status(
        struct clkgate_priv *priv,
        uint32_t *enabled,
        uint32_t *disabled
)
{
        volatile struct clkgate_regs *regs = priv->regs;

        if (enabled) {
                *enabled = regs->enable;
        }
        if (disabled) {
                *disabled = regs->reset;
        }

        return DRV_OK;
}

uint32_t clkgate_override(
        struct clkgate_priv *priv,
        int set,
        uint32_t newval
)
{
        volatile struct clkgate_regs *regs = priv->regs;
        uint32_t oldval = regs->override;

        if (set) {
                regs->override = newval;
        }

        return oldval;
}

