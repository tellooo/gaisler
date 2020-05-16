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

static inline uint32_t REG_READ(volatile uint32_t *addr)
{
        return *addr;
}

static inline void REG_WRITE(volatile uint32_t *addr, uint32_t val)
{
        *addr = val;
}

int grspw_port_ctrl(struct grspw_priv *priv, int *port)
{
        volatile struct grspw_regs *regs = priv->regs;
        unsigned int ctrl;
        SPIN_IRQFLAGS(irqflags);

        if (port == NULL)
                return -1;

        if ((*port == 1) || (*port == 0)) {
                /* Select port user selected */
                if ((*port == 1) && (priv->hwsup.nports < 2))
                        return -1; /* Changing to Port 1, but only one port available */
                SPIN_LOCK_IRQ(&priv->devlock, irqflags);
                ctrl = REG_READ(&regs->ctrl);
                ctrl &= ~(GRSPW_CTRL_NP | GRSPW_CTRL_PS);
                ctrl |= (*port & 1) << GRSPW_CTRL_PS_BIT;
                REG_WRITE(&regs->ctrl, ctrl);
                SPIN_UNLOCK_IRQ(&priv->devlock, irqflags);
        } else if (*port > 1) {
                /* Select both ports */
                SPIN_LOCK_IRQ(&priv->devlock, irqflags);
                REG_WRITE(&regs->ctrl, REG_READ(&regs->ctrl) | GRSPW_CTRL_NP);
                SPIN_UNLOCK_IRQ(&priv->devlock, irqflags);
        }

        /* Get current settings */
        ctrl = REG_READ(&regs->ctrl);
        if (ctrl & GRSPW_CTRL_NP) {
                /* Any port, selected by hardware */
                if (priv->hwsup.nports > 1)
                        *port = 3;
                else
                        *port = 0; /* Port0 the only port available */
        } else {
                *port = (ctrl & GRSPW_CTRL_PS) >> GRSPW_CTRL_PS_BIT;
        }

        return 0;
}

int grspw_port_count(struct grspw_priv *priv)
{
        return priv->hwsup.nports;
}

int grspw_port_active(struct grspw_priv *priv)
{
        unsigned int status;

        status = REG_READ(&priv->regs->status);

        return (status & GRSPW_STS_AP) >> GRSPW_STS_AP_BIT;
}

