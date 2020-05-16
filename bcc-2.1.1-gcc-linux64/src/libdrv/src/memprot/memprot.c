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

#include <drv/memprot.h>
#include <drv/osal.h>
#include <drv/drvret.h>
#include <drv/nelem.h>

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int memprot_register(struct memprot_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int memprot_init(struct memprot_devcfg *devcfgs[])
{
        struct memprot_devcfg **dev = &devcfgs[0];

        while (*dev) {
                memprot_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int memprot_dev_count(void)
{
        return dev_count;
}

const struct drv_devreg *memprot_get_devreg(int dev_no)
{
        const struct
            memprot_devcfg
            *dev =
            (const struct memprot_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}

static int lockit(struct memprot_priv *priv)
{
        volatile struct grmemprot_regs *regs = priv->regs;
        uint32_t pcr;

        pcr = regs->pcr;
        pcr &= ~GRMEMPROT_PCR_PROT;
        regs->pcr = pcr;
        return DRV_OK;
}

static int unlockit(struct memprot_priv *priv)
{
        volatile struct grmemprot_regs *regs = priv->regs;
        uint32_t pcr;

        pcr = regs->pcr;
        pcr &= ~GRMEMPROT_PCR_PROT;
        pcr |= GRMEMPROT_PCR_PROT_MAGIC;
        regs->pcr = pcr;
        return DRV_OK;
}

struct memprot_priv *memprot_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }

        struct memprot_devcfg *dev =
            (struct memprot_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct memprot_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        priv->regs = (struct grmemprot_regs *) dev->regs.addr;

        lockit(priv);

        return priv;
}

int memprot_close(struct memprot_priv *priv)
{
        priv->open = 0;
        return DRV_OK;
}

int memprot_reset(
        struct memprot_priv *priv
)
{
        memprot_stop(priv);

        const int nseg = memprot_nseg(priv);
        for (int i = 0; i < nseg; i++) {
                static const struct memprot_seginfo SEGRESET = { 0 };
                memprot_set(priv, i, &SEGRESET);
        }

        return DRV_OK;
}

int memprot_start(
        struct memprot_priv *priv
)
{
        volatile struct grmemprot_regs *regs = priv->regs;
        unlockit(priv);
        regs->pcr |= GRMEMPROT_PCR_EN;
        lockit(priv);
        return DRV_OK;
}

int memprot_stop(
        struct memprot_priv *priv
)
{
        volatile struct grmemprot_regs *regs = priv->regs;
        unlockit(priv);
        regs->pcr &= ~GRMEMPROT_PCR_EN;
        lockit(priv);
        return DRV_OK;
}

int memprot_isstarted(
        struct memprot_priv *priv
)
{
        volatile struct grmemprot_regs *regs = priv->regs;
        return !!(regs->pcr & GRMEMPROT_PCR_EN);
}

int memprot_nseg(
        struct memprot_priv *priv
)
{
        volatile struct grmemprot_regs *regs = priv->regs;
        return (regs->pcr & GRMEMPROT_PCR_NSEG) >> GRMEMPROT_PCR_NSEG_BIT;
}

int memprot_set(
        struct memprot_priv *priv,
        int segment,
        const struct memprot_seginfo *seginfo
)
{
        volatile struct grmemprot_segment *seg;

        seg = &priv->regs->seg[segment];
        unlockit(priv);
        seg->psa = seginfo->start;
        seg->pea = seginfo->end;
        {
                uint32_t psc = seg->psc;
                psc &= ~GRMEMPROT_PSC_G;
                psc |= seginfo->g << GRMEMPROT_PSC_G_BIT;
                if (seginfo->en) {
                       psc |= GRMEMPROT_PSC_EN;
                } else {
                       psc &= ~GRMEMPROT_PSC_EN;
                }
                seg->psc = psc;
        }
        lockit(priv);

        return DRV_OK;
}

int memprot_get(
        struct memprot_priv *priv,
        int segment,
        struct memprot_seginfo *seginfo
)
{
        volatile const struct grmemprot_segment *seg;

        seg = &priv->regs->seg[segment];
        seginfo->start = seg->psa;
        seginfo->end = seg->pea;
        seginfo->g = (seg->psc & GRMEMPROT_PSC_G) >> GRMEMPROT_PSC_G_BIT;
        seginfo->en = !!(seg->psc & GRMEMPROT_PSC_EN);

        return DRV_OK;
}

#if 0
/* Enable protection for segment */
int memprot_enable(
        struct memprot_priv *priv,
        int segment
)
{
        volatile struct grmemprot_regs *regs = priv->regs;

        unlockit(priv);
        /* segment enable */
        regs->seg[segment].psc |= GRMEMPROT_PSC_EN;
        lockit(priv);

        return DRV_OK;
}

/* Disable protection for segment */
int memprot_disable(
        struct memprot_priv *priv,
        int segment
)
{
        volatile struct grmemprot_regs *regs = priv->regs;

        unlockit(priv);
        /* segment disable */
        regs->seg[segment].psc &= ~GRMEMPROT_PSC_EN;
        lockit(priv);

        return DRV_OK;
}

#endif

