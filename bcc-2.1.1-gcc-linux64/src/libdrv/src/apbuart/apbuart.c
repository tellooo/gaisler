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

#include <drv/apbuart.h>
#include <drv/regs/apbuart.h>
#include <drv/osal.h>
#include <drv/drvret.h>
#include <drv/nelem.h>

#define APBUART_DEBUG_MASK ( \
        APBUART_CTRL_DB | \
        APBUART_CTRL_LB | \
        APBUART_CTRL_FL \
)

static void apbuart_isr(void *arg);
static int dev_count;

static struct drv_list devlist;

int apbuart_register(struct apbuart_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int apbuart_dev_count(void)
{
	return dev_count;
}

int apbuart_init(struct apbuart_devcfg *devcfgs[])
{
        struct apbuart_devcfg **dev = &devcfgs[0];

        while (*dev) {
                apbuart_register(*dev);
                dev++;
        }
        return DRV_OK;
}

struct apbuart_priv *apbuart_open(int dev_no)
{
        uint8_t popen;
        struct apbuart_devcfg *dev;
        struct apbuart_priv *priv;

        dev = (struct apbuart_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        priv = &dev->priv;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }
        priv->regs = (volatile struct apbuart_regs *)dev->regs.addr;
        priv->regs->status = 0;
        /* Let UART debug tunneling be untouched. */
        priv->debug = 0;
        if (0) {
                priv->regs->ctrl &= priv->debug;
        }
        priv->apbfreq = osal_busfreq();
        priv->interrupt = dev->regs.interrupt;
        priv->mode = APBUART_MODE_UNCONFIGURED;
        SPIN_INIT(&priv->devlock, "drvuart");

        return priv;
}

int apbuart_set_debug(struct apbuart_priv *priv, int en)
{
        priv->debug = (uint32_t []) {0, APBUART_DEBUG_MASK}[!!en];
        return DRV_OK;
}

int apbuart_close(struct apbuart_priv *priv)
{
        int ret = DRV_OK;

        /* Let UART debug tunneling be untouched. */
        priv->regs->ctrl &= priv->debug;

        if (APBUART_MODE_INT == priv->mode) {
                ret = osal_isr_unregister(
                        &priv->isr_ctx,
                        priv->interrupt,
                        apbuart_isr,
                        priv
                );
        }
        priv->open = 0;

        return ret;
}

uint32_t apbuart_get_status(struct apbuart_priv *priv)
{
        return priv->regs->status;
}

void apbuart_set_status(struct apbuart_priv *priv, uint32_t status)
{
        priv->regs->status = status;
}

int apbuart_config(struct apbuart_priv *priv, const struct apbuart_config *cfg)
{
        const int PSHIFT = 4;
        const int PMASK = 3;
        uint32_t ctrl;
        SPIN_IRQFLAGS(plev);
        int ret = DRV_OK;

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        /* Let UART debug tunneling be untouched. */
        ctrl = priv->regs->ctrl & priv->debug;
        ctrl |=
            (APBUART_CTRL_TE | APBUART_CTRL_RE |
             (cfg->flow ? APBUART_CTRL_FL : 0));
        ctrl |= ((cfg->parity & PMASK) << PSHIFT);

        if (priv->mode != APBUART_MODE_UNCONFIGURED) {
                ret = DRV_BUSY;
                goto out;
        } else if (cfg->mode == APBUART_MODE_INT) {
                fifo_init(&priv->txfifo, cfg->txfifobuflen, cfg->txfifobuf);
                fifo_init(&priv->rxfifo, cfg->rxfifobuflen, cfg->rxfifobuf);
                ctrl |= APBUART_CTRL_RI;
                /* Register ISR with OSAL. */
                ret = osal_isr_register(
                        &priv->isr_ctx,
                        priv->interrupt,
                        apbuart_isr,
                        priv
                );
                if (DRV_OK != ret) {
                        priv->mode = APBUART_MODE_UNCONFIGURED;
                        goto out;
                }
        } else if (cfg->mode == APBUART_MODE_NONINT) {
                ;
        } else {
                ret = DRV_FAIL;
                goto out;
        }

        priv->regs->scaler =
            (((priv->apbfreq * 10) / (cfg->baud * 8)) - 5) / 10;
        priv->mode = cfg->mode;
        priv->regs->ctrl = ctrl;
        priv->regs->status = 0;

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return ret;
}

/* Non-interrupt interface */

int apbuart_outbyte(struct apbuart_priv *priv, uint8_t data)
{
        if (priv->regs->status & APBUART_STATUS_TF) {
                return 0;
        }
        priv->regs->data = data;

        return 1;
}

int apbuart_inbyte(struct apbuart_priv *priv)
{
        if (0 == (priv->regs->status & APBUART_STATUS_DR)) {
                return -1;
        }

        return priv->regs->data & 0xff;
}

/* Interrupt interface */

int apbuart_write(struct apbuart_priv *priv, const uint8_t *buf, int count)
{
        int i = 0;
        SPIN_IRQFLAGS(plev);

        /* Put user data in TX SW FIFO */
        while (i < count) {
                if (0 == fifo_put(&priv->txfifo, buf[i])) {
                        i++;
                } else {
                        /* TX SW FIFO full */
                        break;
                }
        }

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        /* Enable transmitter FIFO interrupts. */
        priv->regs->ctrl |= APBUART_CTRL_TF;

        /* Fill TX HW FIFO with as much data as possible. */
        while (!(APBUART_STATUS_TF & priv->regs->status)) {
                /* Still room in TX HW FIFO (it is not full). */
                uint8_t data;

                if (0 == fifo_get(&priv->txfifo, &data)) {
                        /* Got byte to send from TX SW FIFO. */
                        priv->regs->data = data;
                } else {
                        break;
                }
        }

        if (fifo_isempty(&priv->txfifo)) {
                /* TX SW FIFO is empty, everything is in TX HW FIFO. */
                priv->regs->ctrl &= ~APBUART_CTRL_TF;
        }

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);

        return i;
}

int apbuart_read(struct apbuart_priv *priv, uint8_t *buf, int count)
{
        int i = 0;
        SPIN_IRQFLAGS(plev);

        /* With interrupts enabled, dequeue RX SW FIFO to user buffer. */
        while (i < count) {
                if (0 == fifo_get(&priv->rxfifo, &buf[i])) {
                        i++;
                } else {
                        /* No more RX data available. */
                        break;
                }
        }

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        /* UART ISR can not intervene here. */
        while (i < count) {
                if (0 == fifo_get(&priv->rxfifo, &buf[i])) {
                        /*
                         * Try with RX SW FIFO first because some may have
                         * arrived before interrupts were disabled.
                         */
                        i++;
                } else if (priv->regs->status & APBUART_STATUS_DR) {
                        /* Then take from RX HW FIFO. */
                        buf[i] = priv->regs->data;
                        i++;
                } else {
                        /* No more RX data available. */
                        break;
                }
        }

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);

        return i;
}

int apbuart_drain(struct apbuart_priv *priv)
{
        if (APBUART_MODE_INT == priv->mode) {
                /* Wait for transmission to finish (TxIRQ turned off) */
                while ((priv->regs->ctrl & APBUART_CTRL_TF)) {
                        ;
                }
        }

        /*
         * Wait for transmission to finish (Transmitter FIFO empty and
         * transmitter shift register empty).
         */
        while ((APBUART_STATUS_TS | APBUART_STATUS_TE) !=
            (priv->regs->status & (APBUART_STATUS_TS | APBUART_STATUS_TE))) {
                ;
        }

        return DRV_OK;
}

static void apbuart_isr(void *arg)
{
        struct apbuart_priv *priv = arg;
        uint32_t status;
        SPIN_ISR_IRQFLAGS(plev);

        SPIN_LOCK(&priv->devlock, plev);
        /* Receive */
        while (APBUART_STATUS_DR & (status = priv->regs->status)) {
                /* Copy frame from RX HW FIFO to RX SW FIFO. */
                if (fifo_isfull(&priv->rxfifo)) {
                        break;
                }
                fifo_put(&priv->rxfifo, priv->regs->data);
        }

        /* Transmit */
        if (!(status & APBUART_STATUS_TF)) {
                /* TX HW FIFO is not full. */
                while (!(APBUART_STATUS_TF & (status = priv->regs->status))) {
                        /* Still room in TX HW FIFO (it is not full). */
                        uint8_t data;

                        if (0 == fifo_get(&priv->txfifo, &data)) {
                                /* Got byte to send from TX SW FIFO. */
                                priv->regs->data = data;
                        } else {
                                break;
                        }
                }
                if (fifo_isempty(&priv->txfifo)) {
                        /* No more data to send. Disable TX interrupts. */
                        priv->regs->ctrl &= ~APBUART_CTRL_TF;
                }
        }
        SPIN_UNLOCK(&priv->devlock, plev);
}

const struct drv_devreg *apbuart_get_devreg(int dev_no)
{
        const struct
            apbuart_devcfg
            *dev =
            (const struct apbuart_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}

