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

#include <stdint.h>
#include <stdlib.h>

#include <drv/ahbstat.h>
#include <drv/osal.h>
#include <drv/drvret.h>
#include <drv/nelem.h>

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int ahbstat_register(struct ahbstat_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int ahbstat_init(struct ahbstat_devcfg *devcfgs[])
{
        struct ahbstat_devcfg **dev = &devcfgs[0];

        while (*dev) {
                ahbstat_register(*dev);
                dev++;
        }
        return DRV_OK;
}

static void ahbstat_isr(void *arg);

int ahbstat_dev_count(void)
{
        return dev_count;
}

const struct drv_devreg *ahbstat_get_devreg(int dev_no)
{
        const struct
            ahbstat_devcfg
            *dev =
            (const struct ahbstat_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}

struct ahbstat_priv *ahbstat_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }

        struct ahbstat_devcfg *dev =
            (struct ahbstat_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct ahbstat_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        priv->regs = (struct ahbstat_regs *) dev->regs.addr;
        priv->interrupt = dev->regs.interrupt;
        /* Initialize registers. */
        priv->regs->status = 0;

        priv->userhandler = NULL;
        priv->userarg = NULL;

        return priv;
}

int ahbstat_close(struct ahbstat_priv *priv)
{
        int ret = DRV_OK;
        if (priv->userhandler) {
                ret = osal_isr_unregister(
                        &priv->isr_ctx,
                        priv->interrupt,
                        ahbstat_isr,
                        priv
                );
        }
        priv->open = 0;
        return ret;
}

int ahbstat_set_user(
        struct ahbstat_priv *priv,
        int (*userhandler)(
                volatile struct ahbstat_regs *regs,
                uint32_t status,
                uint32_t failing_address,
                void *userarg
        ),
        void *userarg
)
{
        int ret = DRV_OK;

        /* Uninstall ISR if already installed. */
        if (priv->userhandler) {
                osal_isr_unregister(
                        &priv->isr_ctx,
                        priv->interrupt,
                        ahbstat_isr,
                        priv
                );
        }
        priv->userhandler = userhandler;
        priv->userarg = userarg;
        if (userhandler) {
                ret = osal_isr_register(
                        &priv->isr_ctx,
                        priv->interrupt,
                        ahbstat_isr,
                        priv
                );
        }
        return ret;
}

static void ahbstat_isr(void *arg)
{
        struct ahbstat_priv *priv = arg;
        volatile struct ahbstat_regs *regs = priv->regs;
        uint32_t status;
        uint32_t failing_address;
        int uret;

        status = regs->status;
        if (0 == (status & AHBSTAT_STS_NE)) {
                return;
        }

        failing_address = regs->failing;

        uret = priv->userhandler(regs, status, failing_address, priv->userarg);
        if (0 == uret) {
                /* Re-enable error monitoring iff user returns 0. */
                regs->status = 0;
        }
}

