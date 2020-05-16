/*
 * Copyright 2018 Cobham Gaisler AB
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include <drv/grspw.h>
#include <drv/nelem.h>
#include <drv/osal.h>
#include <drv/drvret.h>

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int grspw_register(struct grspw_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int grspw_init(struct grspw_devcfg *devcfgs[])
{
        struct grspw_devcfg **dev = &devcfgs[0];

        while (*dev) {
                grspw_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int grspw_dev_count(void)
{
        return dev_count;
}

/* Hardware Action:
 *  - stop DMA
 *  - do not bring down the link (RMAP may be active)
 *  - RMAP settings untouched (RMAP may be active)
 *  - timecodes are disabled
 *  - IRQ generation disabled
 *  - status not cleared (let user analyze it if requested later on)
 *  - Node address / First DMA channels Node address
 *    is untouched (RMAP may be active)
 */
static void grspw_hw_stop(struct grspw_priv *priv)
{
        uint32_t ctrl;

        for (int i = 0; i < priv->hwsup.ndma_chans; i++) {
                grspw_hw_dma_stop(&priv->dma[i]);
        }

        ctrl = priv->regs->ctrl;
        priv->regs->ctrl =
            ctrl & (GRSPW_CTRL_LD | GRSPW_CTRL_LS | GRSPW_CTRL_AS |
                    GRSPW_CTRL_RE | GRSPW_CTRL_RD | GRSPW_CTRL_NP |
                    GRSPW_CTRL_PS);
}

/*
 * Soft reset of GRSPW core registers. Assumes device is not open.
 */
static void grspw_hw_softreset(struct grspw_priv *priv)
{
        for (int i = 0; i < priv->hwsup.ndma_chans; i++) {
                grspw_hw_dma_softreset(&priv->dma[i]);
        }

        priv->regs->status = 0xffffffff;
        priv->regs->time = 0;
}

static void init_hwsup(struct grspw_priv *priv, const struct drv_devreg *devreg)
{
        struct grspw_hw_sup *hw = &priv->hwsup;
        volatile struct grspw_regs *regs = priv->regs;
        uint32_t ctrl;

        ctrl = regs->ctrl;
        hw->rmap = (ctrl & GRSPW_CTRL_RA) >> GRSPW_CTRL_RA_BIT;
        hw->rmap_crc = (ctrl & GRSPW_CTRL_RC) >> GRSPW_CTRL_RC_BIT;
        hw->rx_unalign = (ctrl & GRSPW_CTRL_RX) >> GRSPW_CTRL_RX_BIT;
        hw->nports = 1 + ((ctrl & GRSPW_CTRL_PO) >> GRSPW_CTRL_PO_BIT);
        hw->ndma_chans = 1 + ((ctrl & GRSPW_CTRL_NCH) >> GRSPW_CTRL_NCH_BIT);
        /* NOTE: Distributed interrupts are not supported by this driver. */
        hw->irq = ((ctrl & GRSPW_CTRL_ID) >> GRSPW_CTRL_ID_BIT);

	/* Construct hardware version identification */
        hw->hw_version = devreg->device_id << 16 | devreg->version;
}


struct grspw_priv *grspw_open(int dev_no)
{
        uint8_t popen;

        if (dev_no < 0 || dev_count <= dev_no) {
                return NULL;
        }

        struct grspw_devcfg *dev =
            (struct grspw_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct grspw_priv *priv = &dev->priv;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        /* GRSPW device lock. Protects CTRL and DMACTRL registers from ISR. */
        /* NOTE: on every open */
        SPIN_INIT(&priv->devlock, priv->devname);

        priv->regs = (void *)dev->regs.addr;
        priv->irqsource = dev->regs.interrupt;
        priv->userisr = NULL;
        priv->userisr_data = NULL;

        for (int i = 0; i < NELEM(priv->dma); i++) {
                priv->dma[i].regs = &priv->regs->dma[i];
                priv->dma[i].open = 0;
                priv->dma[i].ubuf = NULL;
        }

        init_hwsup(priv, &dev->regs);
        grspw_hw_stop(priv);
        grspw_hw_softreset(priv);

        /* Register interrupt handler and enable irq source. */
        int ret;

        ret = osal_isr_register(
                &priv->isr_ctx,
                priv->irqsource,
                grspw_isr,
                priv
        );
        if (DRV_OK != ret) {
                priv->open = 0;
                return NULL;
        }

        return priv;
}

int grspw_close(struct grspw_priv *priv)
{
        SPIN_IRQFLAGS(plev);
        int ret;

        for (int i = 0; i < priv->hwsup.ndma_chans; i++) {
                struct grspw_dma_priv *dma = &priv->dma[i];
                if (dma->open && dma->ubuf) {
                        return DRV_FAIL;
                }
        }

        /* Unregister interrupt handler. */
        ret = osal_isr_unregister(
                &priv->isr_ctx,
                priv->irqsource,
                grspw_isr,
                priv
        );

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        /* Stop Hardware from doing DMA, put HW into "startup-state". */
        for (int i = 0; i < priv->hwsup.ndma_chans; i++) {
                grspw_dma_close_userbuf(&priv->dma[i]);
        }
        grspw_hw_stop(priv);

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        SPIN_FREE(&priv->devlock);

        priv->open = 0;
        return ret;
}

