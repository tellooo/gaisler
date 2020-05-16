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

#include <drv/grspw.h>
#include <drv/osal.h>
#include <drv/drvret.h>

void grspw_addr_ctrl(
        struct grspw_priv *priv,
        const struct grspw_addr_config *cfg
)
{
        volatile struct grspw_regs *regs = priv->regs;
        uint32_t ctrl;
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        /* Set Configuration */
        ctrl = regs->ctrl;
        ctrl &= ~GRSPW_CTRL_PM;
        ctrl |= (cfg->promiscuous << GRSPW_CTRL_PM_BIT);
        regs->ctrl = ctrl;
        regs->nodeaddr = (cfg->def_mask << 8) | cfg->def_addr;

        for (int i = 0; i < priv->hwsup.ndma_chans; i++) {
                ctrl = regs->dma[i].ctrl;
                ctrl &= ~(GRSPW_DMACTRL_PS | GRSPW_DMACTRL_PR |
                          GRSPW_DMA_STATUS_ERROR | GRSPW_DMACTRL_EN);
                ctrl |= (cfg->dma_nacfg[i].node_en << GRSPW_DMACTRL_EN_BIT);
                /*
                 * Always set addr, even if separate node address is not used
                 * for the channel.
                 */
                regs->dma[i].addr =
                    (cfg->dma_nacfg[i].node_addr & 0xff) |
                    ((cfg->dma_nacfg[i].node_mask & 0xff) << 8);
                regs->dma[i].ctrl = ctrl;
        }

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
}

