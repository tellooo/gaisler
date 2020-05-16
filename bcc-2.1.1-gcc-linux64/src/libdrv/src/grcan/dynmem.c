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
#include <drv/grcan.h>
#include <drv/osal.h>

#define BLOCK_SIZE (16*4)

/* grcan needs to have it buffers aligned to 1k boundaries */
#define BUFFER_ALIGNMENT_NEEDS 1024

/* Default Maximium buffer size for statically allocated buffers */
#ifndef TX_BUF_SIZE
 #define TX_BUF_SIZE (BLOCK_SIZE*16)
#endif

/* Make receiver buffers bigger than transmitt */
#ifndef RX_BUF_SIZE
 #define RX_BUF_SIZE ((3*BLOCK_SIZE)*16)
#endif

struct grcan_priv *grcan_open(int dev_no)
{
        struct grcan_priv *dev = NULL;
        void *rxa;
        void *txa;
        unsigned int rxbuf;
        unsigned int txbuf;

        rxa = malloc(RX_BUF_SIZE + BUFFER_ALIGNMENT_NEEDS);
        if (!rxa) {
                return NULL;
        }
        rxbuf = ((unsigned int)rxa + (BUFFER_ALIGNMENT_NEEDS-1))
            & ~(BUFFER_ALIGNMENT_NEEDS-1);

        txa = malloc(TX_BUF_SIZE + BUFFER_ALIGNMENT_NEEDS);
        if (!txa) {
                free(rxa);
                return NULL;
        }
        txbuf = ((unsigned int)txa + (BUFFER_ALIGNMENT_NEEDS-1))
            & ~(BUFFER_ALIGNMENT_NEEDS-1);

        dev = grcan_open_userbuf(
                dev_no,
                (void *) rxbuf,
                RX_BUF_SIZE,
                (void *) txbuf,
                TX_BUF_SIZE
        );
        if (dev) {
                dev->rxa = rxa;
                dev->txa = txa;
        } else {
                free(rxa);
                free(txa);
        }

        return dev;
}

int grcan_close(struct grcan_priv *priv)
{
        int ret;
        void *rxa;
        void *txa;

        rxa = priv->rxa;
        txa = priv->txa;
        ret = grcan_close(priv);

        if (DRV_OK == ret) {
                free(rxa);
                free(txa);
        }

        return ret;
}

