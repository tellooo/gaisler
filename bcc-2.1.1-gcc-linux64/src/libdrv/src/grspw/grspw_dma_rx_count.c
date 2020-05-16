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

/* Get number of received packets not yet retrieved by driver. */
int grspw_dma_rx_count(struct grspw_dma_priv *dma)
{
        struct grspw_rxring *tail;
        uint32_t hwdesc;
        uint32_t swtaildesc;
        int swtaildesc_en;
        int count = 0;
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        tail = dma->rx_ring_tail;
        if (!tail->pkt) {
                /* No scheduled packets. */
                goto out;
        }
        /* At least one descriptor has been scheduled. */

        /*
         * IF "swtaildesc_en == 0" THEN
         *    "at least one descriptor has been processed"
         */

        swtaildesc_en = (tail->bd->ctrl >> GRSPW_RXBD_EN_BIT) & 0x1;

        hwdesc = dma->regs->rxdesc;
        /* Address of oldest scheduled descriptor. */
        swtaildesc = (uint32_t) tail->bd;

        /*
         * Calculate number of descriptors between latest HW-processed
         * descriptor and oldest SW-scheduled descriptor.
         */
        count = ((hwdesc - swtaildesc) & (BDTAB_SIZE - 1)) /
            (BDTAB_SIZE / GRSPW_RXBD_NR);

        /*
         * Handle the case when HW and SW pointer are equal because all RX
         * packets have been processed by hardware.
         */
        if ((0 == count) && (!swtaildesc_en)) {
                count = GRSPW_RXBD_NR;
        }

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return count;
}

