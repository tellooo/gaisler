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

struct grspw_userbuf {
        struct grspw_ring rx_ring[GRSPW_RXBD_NR];
        struct grspw_ring tx_ring[GRSPW_TXBD_NR];
        struct grspw_rxbd *rx_bds;
        struct grspw_txbd *tx_bds;
        uint8_t bdarea[3*0x400]; /* To allow for two 1KiB aligned 1KiB areas */
};

struct grspw_dma_priv *grspw_dma_open(
        struct grspw_priv *priv,
        int chan_no
)
{
        struct grspw_dma_priv *dma = NULL;
        struct grspw_userbuf *ubuf;
        uint32_t bdbase;

        ubuf = malloc(sizeof *ubuf);
        if (!ubuf) {
                return NULL;
        }

        /* Align to 1KiB boundary */
        bdbase = ((uint32_t) ubuf->bdarea + 0x3ff) & ~0x3ff;

        ubuf->rx_bds = (struct grspw_rxbd *) bdbase;
        ubuf->tx_bds = (struct grspw_txbd *) (bdbase + 0x400);

        dma = grspw_dma_open_userbuf(
                priv,
                chan_no,
                ubuf->rx_ring,
                ubuf->tx_ring,
                ubuf->rx_bds,
                ubuf->tx_bds
        );
        if (dma) {
                dma->ubuf = ubuf;
        } else {
                free(ubuf);
        }

        return dma;
}

int grspw_dma_close(
        struct grspw_dma_priv *dma
)
{
        int ret;
        struct grspw_userbuf *ubuf = dma->ubuf;

        /* NOTE: dma_close_userbuf() sets dma->ubuf NULL on success */
        ret = grspw_dma_close_userbuf(dma);
        if (DRV_OK == ret) {
                free(ubuf);
        }
        return ret;
}

