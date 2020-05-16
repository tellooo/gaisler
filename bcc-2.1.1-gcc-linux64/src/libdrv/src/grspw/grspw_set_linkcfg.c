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
#include <drv/drvret.h>

int grspw_set_linkcfg(struct grspw_priv *priv, uint32_t cfg)
{
        volatile struct grspw_regs *regs = priv->regs;
        uint32_t ctrl;
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        ctrl = regs->ctrl;
        ctrl = (ctrl & ~GRSPW_LINK_CFG) | (cfg & GRSPW_LINK_CFG);
        /* Enable Global IRQ only if some irq source is set */
        if (grspw_is_irqsource_set(ctrl)) {
                ctrl |= GRSPW_CTRL_IE;
        } else {
                ctrl &= ~GRSPW_CTRL_IE;
        }
        regs->ctrl = ctrl;

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return DRV_OK;
}

