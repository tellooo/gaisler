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

/*
 * Driver for the GRLIB I2C-master "I2CMST"
 *
 * Features:
 * - Non-blocking user interface
 * - Optionally interrupt driven
 * - User supplies chains of packets to send
 * - Automatic retry operation
 *
 * Authors:
 * Arvid Björkengren and Martin Åberg, Cobham Gaisler AB
 */
#include <stdint.h>
#include <stdlib.h>

#include <drv/i2cmst.h>
#include <drv/osal.h>
#include <drv/drvret.h>
#include <drv/nelem.h>

#if 0
#include <stdio.h>
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int i2cmst_register(struct i2cmst_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int i2cmst_init(struct i2cmst_devcfg *devcfgs[])
{
        struct i2cmst_devcfg **dev = &devcfgs[0];

        while (*dev) {
                i2cmst_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int i2cmst_dev_count(void)
{
        return dev_count;
}

const struct drv_devreg *i2cmst_get_devreg(int dev_no)
{
        const struct
            i2cmst_devcfg
            *dev =
            (const struct i2cmst_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}

/* Flags in pkt that should be cleared before being scheduled */
static const uint32_t I2CMST_FLAGS_CLEAR = {
        I2CMST_FLAGS_FINISHED |
        I2CMST_FLAGS_ERR |
        I2CMST_FLAGS_RETRIED |
        0
};

/* Prototypes */
static void i2cmst_interrupt(void *arg);

enum {
        I2CMST_STATE_IDLE = 0,  /* Do nothing */
        I2CMST_STATE_RETRY,     /* Send slave address */
        I2CMST_STATE_START,     /* Send slave address */
        I2CMST_STATE_WR,        /* Switch read/write mode */
        I2CMST_STATE_ADDR,      /* Send target address */
        I2CMST_STATE_READ,      /* Receive byte */
        I2CMST_STATE_WRITE,     /* Send byte */
        I2CMST_STATE_FINISHED,  /* Transfer finished */
};

/******************* Driver Implementation ***********************/
static inline void i2cmst_list_clr(struct i2cmst_list *list)
{
        list->head = NULL;
        list->tail = NULL;
}

int i2cmst_start(struct i2cmst_priv *pDev)
{
        volatile struct i2cmst_regs *regs = pDev->regs;
        static const struct i2cmst_stats stats_zero = {0};
        uint32_t ctrl;

        if ( pDev->running ) {
                return DRV_BUSY; /* EBUSY */
        }

        /* Clear stats */
        pDev->stats = stats_zero;

        /* Clear Scheduled, Ready and Sent list */
        i2cmst_list_clr(&pDev->queue);
        pDev->state = I2CMST_STATE_IDLE;
        pDev->scheduled = NULL;

        /* Software init */
        regs->prescale = (osal_busfreq() / (5*pDev->config.speed))-1;

        ctrl = I2CMST_CTRL_EN;
        if (pDev->config.isr_pkt_proc) {
                /* Register interrupt handler & Enable interrupt */
                osal_isr_register(
                        &pDev->isr_ctx,
                        pDev->irq,
                        i2cmst_interrupt,
                        pDev
                );

                /* Clear old interrupts */
                regs->c.cmd = I2CMST_CMD_IACK;

                /* Enable interrupts (Error and DMA TX) */
                ctrl |= I2CMST_CTRL_IEN;
        }

        /* Mark running before enabling the DMA transmitter */
        pDev->running = 1;

        /* Enable I2CMST Transmitter */
        regs->ctrl = ctrl;

        return DRV_OK;
}

int i2cmst_stop(struct i2cmst_priv *pDev)
{
        volatile struct i2cmst_regs *regs = pDev->regs;

        if ( !pDev->running ) {
                return DRV_BUSY;
        }

        /* Disable the transmitter & Interrupts */
        if (pDev->config.isr_pkt_proc) {
                osal_isr_unregister(
                        &pDev->isr_ctx,
                        pDev->irq,
                        i2cmst_interrupt,
                        pDev
                );
        }
        regs->ctrl = 0;

        /* Clear any pending interrupt  */
        regs->c.cmd = I2CMST_CMD_IACK;

        pDev->running = 0;

        return DRV_OK;
}

struct i2cmst_priv *i2cmst_open(int dev_no)
{
        static const struct i2cmst_config cfg_zero = {
                .speed = I2CMST_SPEED_STD
        };

        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }
        struct i2cmst_devcfg *dev =
            (struct i2cmst_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct i2cmst_priv *pDev = &dev->priv;

        /* Mark device taken */
        uint8_t popen;
        popen = osal_ldstub(&pDev->open);
        if (popen) {
                return NULL;
        }

        SPIN_INIT(&pDev->devlock, "i2cmst-lock");

        pDev->irq = dev->regs.interrupt;
        pDev->regs = (struct i2cmst_regs *) dev->regs.addr;

        pDev->running = 0;         /* not in running mode yet */

        pDev->config = cfg_zero;

        return pDev;
}

int i2cmst_close(struct i2cmst_priv *pDev)
{
        if ( pDev->running ){
                i2cmst_stop(pDev);
                pDev->running = 0;
        }

        SPIN_FREE(&pDev->devlock);

        /* Mark not open */
        pDev->open = 0;

        return DRV_OK;
}

static void i2cmst_state_start(struct i2cmst_priv *pDev)
{
        struct i2cmst_packet *pkt = pDev->scheduled;

        if (pDev->config.ten_bit_addr) {
                pDev->header[0] = (pkt->slave>>7) & 0x6;
                pDev->header[1] = (pkt->slave) & 0xFF;
                pDev->length = 2;
                DBG("[SETUP] START  0x%02x 0x%02x\n", pDev->header[0], pDev->header[1]);
        } else {
                pDev->header[0] = (pkt->slave & 0x7F) << 1;
                if (!(pkt->flags & I2CMST_FLAGS_ADDR)) {
                        pDev->header[0] |= pkt->flags & I2CMST_FLAGS_READ;
                }
                pDev->length = 1;
                DBG("[SETUP] START  0x%02x\n", pDev->header[0]);
        }
}

static void i2cmst_state_schedule(struct i2cmst_priv *pDev, struct i2cmst_packet *pkt)
{
        while(pkt != NULL && (pkt->flags & I2CMST_FLAGS_FINISHED)) {
                pkt = pkt->next;
        }

        pDev->scheduled = pkt;
        pDev->retry = 0;
        if (pDev->scheduled == NULL) {
                DBG("[SWITCH] %s -> IDLE\n",
                        pDev->state == I2CMST_STATE_IDLE ? "IDLE" : "FINISHED");
                pDev->state = I2CMST_STATE_IDLE;
                pDev->pos = 0;
                pDev->length = 0;
        } else {
                DBG("[SWITCH] %s -> START  Schedule\n",
                        pDev->state == I2CMST_STATE_IDLE ? "IDLE" : "FINISHED");
                pDev->state = I2CMST_STATE_START;
                pDev->pos = 0;
                i2cmst_state_start(pDev);
        }
}

static void i2cmst_state_switch(struct i2cmst_priv *pDev)
{
        struct i2cmst_packet *pkt = pDev->scheduled;

        /* Switch state */
        switch (pDev->state) {
        case I2CMST_STATE_RETRY:
                if (pDev->retry >= pDev->config.retry) {
                        pkt->flags |= I2CMST_FLAGS_ERR;
                        pDev->state = I2CMST_STATE_FINISHED;
                        pDev->regs->c.cmd = I2CMST_CMD_STO;
                        DBG("[SWITCH] RETRY -> FINISHED  cmd=0x%02x\n", I2CMST_CMD_STO);
                } else {
                        pDev->state = I2CMST_STATE_START;
                        DBG("[SWITCH] RETRY -> START\n");
                        pDev->retry++;
                        pkt->flags |= I2CMST_FLAGS_RETRIED;
                }
                break;
        case I2CMST_STATE_START:
                if (pkt->flags & I2CMST_FLAGS_ADDR) {
                        pDev->state = I2CMST_STATE_ADDR;
                        DBG("[SWITCH] START -> ADDR\n");
                } else if (pkt->flags & I2CMST_FLAGS_READ) {
                        if (pDev->config.ten_bit_addr) {
                                pDev->state = I2CMST_STATE_WR;
                                DBG("[SWITCH] START -> WR\n");
                        } else {
                                pDev->state = I2CMST_STATE_READ;
                                DBG("[SWITCH] START -> READ\n");
                        }
                } else {
                        pDev->state = I2CMST_STATE_WRITE;
                        DBG("[SWITCH] START -> WRITE\n");
                }
                break;
        case I2CMST_STATE_WR:
                if (pkt->flags & I2CMST_FLAGS_READ) {
                        pDev->state = I2CMST_STATE_READ;
                        DBG("[SWITCH] WR -> READ\n");
                } else {
                        pDev->state = I2CMST_STATE_WRITE;
                        DBG("[SWITCH] WR -> WRITE\n");
                }
                break;
        case I2CMST_STATE_ADDR:
                if (pkt->flags & I2CMST_FLAGS_READ) {
                        pDev->state = I2CMST_STATE_WR;
                        DBG("[SWITCH] ADDR -> WR\n");
                } else {
                        pDev->state = I2CMST_STATE_WRITE;
                        DBG("[SWITCH] ADDR -> WRITE\n");
                }
                break;
        case I2CMST_STATE_READ:
                pDev->state = I2CMST_STATE_FINISHED;
                DBG("[SWITCH] READ -> FINISHED\n");
                break;
        case I2CMST_STATE_WRITE:
                pDev->state = I2CMST_STATE_FINISHED;
                DBG("[SWITCH] WRITE -> FINISHED\n");
                break;
        default:
                break;
        }
}

static void i2cmst_state_setup(struct i2cmst_priv *pDev)
{
        struct i2cmst_packet *pkt = pDev->scheduled;

        pDev->pos = 0;
        pDev->length = 0;
        switch (pDev->state) {
        case I2CMST_STATE_FINISHED:
                /* 1. Set flags to indicate error(s) and other information */
                pkt->flags |= I2CMST_FLAGS_FINISHED; /* Mark sent */
                pDev->stats.packets_sent += 1;
                pDev->scheduled = NULL;
                DBG(
                        "[SETUP] FINISHED  Flags=0x%x, num=%u\n",
                        (unsigned int) pkt->flags,
                        (unsigned int) pDev->stats.packets_sent
                );
                i2cmst_state_schedule(pDev, pkt);
                break;
        case I2CMST_STATE_START:
                i2cmst_state_start(pDev);
                break;
        case I2CMST_STATE_WR:
                if (pDev->config.ten_bit_addr) {
                        pDev->header[0] = (pkt->slave>>7) & 0x6;
                        pDev->header[0] |= pkt->flags & I2CMST_FLAGS_READ;
                        pDev->length = 1;
                } else {
                        pDev->header[0] = (pkt->slave & 0x7F) << 1;
                        pDev->header[0] |= pkt->flags & I2CMST_FLAGS_READ;
                        pDev->length = 1;
                }
                DBG("[SETUP] WR  0x%02x\n", pDev->header[0]);
                break;
        case I2CMST_STATE_ADDR:
                pDev->length = 4;
                DBG("[SETUP] ADDR\n");
                break;
        case I2CMST_STATE_READ:
                pDev->length = pkt->length;
                DBG("[SETUP] READ\n");
                break;
        case I2CMST_STATE_WRITE:
                pDev->length = pkt->length;
                DBG("[SETUP] WRITE\n");
                break;
        default:
                break;
        }
}

static void i2cmst_state_recv(struct i2cmst_priv *pDev)
{
        switch (pDev->state) {
        case  I2CMST_STATE_READ:
                {
                        uint32_t data = pDev->regs->x.rx & 0xff;
                        pDev->scheduled->payload[pDev->pos] = data;
                        pDev->pos++;
                        DBG(
                                "[RECV] READ  data=0x%02x (%3d) pos=%3d\n",
                                (unsigned int) data,
                                (unsigned int) data,
                                pDev->pos-1
                        );
                }
                break;
        default:
                break;
        }
}

static void i2cmst_state_send(struct i2cmst_priv *pDev)
{
        uint32_t cmd = 0;
        uint32_t data;

        pDev->ack = 0;
        switch (pDev->state) {
        case I2CMST_STATE_START:
                if (pDev->pos == 0) {
                        cmd = I2CMST_CMD_WR|I2CMST_CMD_STA;
                        pDev->ack = !pDev->config.ten_bit_addr;
                } else {
                        cmd = I2CMST_CMD_WR;
                        pDev->ack = 1;
                }
                data = pDev->header[pDev->pos++];
                DBG(
                        "[SEND] START  cmd=0x%02x, data=0x%02x\n",
                        (unsigned int) cmd,
                        (unsigned int) data
                );
                pDev->regs->x.tx = data;
                pDev->regs->c.cmd = cmd;
                break;
        case I2CMST_STATE_WR:
                data = pDev->header[pDev->pos++];
                cmd = I2CMST_CMD_WR|I2CMST_CMD_STA;
                DBG(
                        "[SEND] WR  cmd=0x%02x, data=0x%02x\n",
                        (unsigned int) cmd,
                        (unsigned int) data
                );
                pDev->regs->x.tx = data;
                pDev->regs->c.cmd = cmd;
                pDev->ack = 1;
                break;
        case I2CMST_STATE_ADDR:
                data = ((uint8_t*)&pDev->scheduled->addr)[pDev->pos++];
                cmd = I2CMST_CMD_WR;
                DBG(
                        "[SEND] ADDR  cmd=0x%02x, data=0x%02x\n",
                        (unsigned int) cmd,
                        (unsigned int) data
                );
                pDev->regs->x.tx = data;
                pDev->regs->c.cmd = cmd;
                pDev->ack = 1;
                break;
        case I2CMST_STATE_WRITE:
                data = pDev->scheduled->payload[pDev->pos++];
                if (pDev->pos >= pDev->length) {
                        cmd = I2CMST_CMD_WR|I2CMST_CMD_STO;
                } else {
                        cmd = I2CMST_CMD_WR;
                }
                DBG(
                        "[SEND] WRITE  cmd=0x%02x, data=0x%02x\n",
                        (unsigned int) cmd,
                        (unsigned int) data
                );
                pDev->regs->x.tx = data;
                pDev->regs->c.cmd = cmd;
                pDev->ack = 1;
                break;
        case I2CMST_STATE_READ:
                if (pDev->pos == pDev->length-1) {
                        cmd = I2CMST_CMD_RD|I2CMST_CMD_ACK|I2CMST_CMD_STO;
                } else if (pDev->pos < pDev->length) {
                        cmd = I2CMST_CMD_RD;
                }
                DBG("[SEND] READ  cmd=0x%02x\n", (unsigned int) cmd);
                pDev->regs->c.cmd = cmd;

                break;
        default:
                break;
        }
}

static void i2cmst_state_transfer(struct i2cmst_priv *pDev)
{
        uint32_t status = pDev->regs->c.status;
        DBG("[STATUS] 0x%08x \n", (unsigned int) status);

        if (pDev->scheduled == NULL) {
                return;
        } else if (status & I2CMST_STATUS_TIP) {
                return;
        } else if ((status & I2CMST_STATUS_AL) &&
                   !(pDev->state == I2CMST_STATE_IDLE ||
                     pDev->state == I2CMST_STATE_FINISHED)) {
                pDev->stats.arbitration_lost++;
                pDev->length = 0;
                pDev->pos = 0;
                pDev->state = I2CMST_STATE_RETRY;
                DBG("[SWITCH] ... -> RETRY  Arbitration lost\n");
        } else if ((status & I2CMST_STATUS_RXACK) && pDev->ack /*&&
                        !(pDev->state == I2CMST_STATE_READ && pDev->pos==(pDev->length-1))*/) {
                pDev->state = I2CMST_STATE_RETRY;
                pDev->stats.packets_nack++;
                pDev->length = 0;
                pDev->pos = 0;
                DBG("[SWITCH] ... -> RETRY  Nack\n");
        }

        i2cmst_state_recv(pDev);

        if (pDev->pos >= pDev->length) {
#if 0
        if (
                pDev->state == I2CMST_STATE_START &&
                (status & I2CMST_STATUS_RXACK)
        ) {
                        pDev->state = I2CMST_STATE_RETRY;
                        DBG("[SWITCH] START -> RETRY  Nack\n");
                }
#endif
                i2cmst_state_switch(pDev);
                i2cmst_state_setup(pDev);
        }

        i2cmst_state_send(pDev);
}

int i2cmst_isstarted(struct i2cmst_priv *pDev)
{
        return !!pDev->running;
}

int i2cmst_set_retries(struct i2cmst_priv *pDev, int retries)
{
        if ( retries < 0 ) {
                return DRV_INVAL;
        }

        if ( pDev->running ) {
                return DRV_BUSY;
        }

        pDev->config.retry = retries;

        return DRV_OK;
}

int i2cmst_set_speed(struct i2cmst_priv *pDev, int speed)
{
        if ( speed < 0 ) {
                return DRV_INVAL;
        }

        if ( pDev->running ) {
                return DRV_BUSY;
        }

        pDev->config.speed = speed;

        return DRV_OK;
}

int i2cmst_set_interrupt_mode(struct i2cmst_priv *pDev, int enable)
{
        if ( pDev->running ) {
                return DRV_BUSY;
        }

        pDev->config.isr_pkt_proc = enable;

        return DRV_OK;
}

int i2cmst_set_ten_bit_addr(struct i2cmst_priv *pDev, int enable)
{
        if ( pDev->running ) {
                return DRV_BUSY;
        }

        pDev->config.ten_bit_addr = enable;

        return DRV_OK;
}

int i2cmst_get_stats(struct i2cmst_priv *pDev, struct i2cmst_stats *stats)
{
        if ( !stats ) {
                return DRV_INVAL;
        } else {
                *stats = pDev->stats;
        }

        return DRV_OK;
}

int i2cmst_clr_stats(struct i2cmst_priv *pDev)
{
        static const struct i2cmst_stats stats_zero = {0};
        pDev->stats = stats_zero;

        return DRV_OK;
}

/* Put a chain of frames at the back of the "Ready frames" queue. This
 * triggers the driver to put frames from the Ready queue into unused
 * available descriptors. (Ready -> Scheduled)
 */
int i2cmst_request(struct i2cmst_priv *pDev, struct i2cmst_list *chain)
{
        struct i2cmst_packet *curr;

        if ( !pDev->running ){
                return DRV_BUSY;
        }

        /* Get pointer to frame chain wished be sent */
        if ( chain ) {
                if ( !chain->tail || !chain->head ){
                        return DRV_INVAL;
                }

                /* Mark ready frames unsent by clearing I2CMST_FLAGS_FINISHED of all frames */
                curr = chain->head;
                chain->tail->next = NULL;
                while(curr != NULL){
                        curr->flags = curr->flags & ~(I2CMST_FLAGS_CLEAR);
                        curr = curr->next;
                }

                /* Put frames into ready queue
                 * (New Frames->READY)
                 */
                SPIN_IRQFLAGS(oldLevel);
                SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
                if ( pDev->queue.head ){
                        /* Frames already on ready queue (no free descriptors previously) ==>
                         * Put frames at end of ready queue
                         */
                        pDev->queue.tail->next = chain->head;
                        pDev->queue.tail = chain->tail;
                }else{
                        /* All frames is put into the ready queue for later processing */
                        pDev->queue.head = chain->head;
                        pDev->queue.tail = chain->tail;
                }
                SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);
                DBG("[REQUEST]\n");
        }

        /* Use all available free descriptors there are frames for
         * in the ready queue.
         * (READY->SCHEDULED)
         */
        if (pDev->config.isr_pkt_proc) {
                SPIN_IRQFLAGS(oldLevel);
                SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
                if ( pDev->state == I2CMST_STATE_IDLE ){
                        i2cmst_state_schedule(pDev, pDev->queue.head);
                        i2cmst_state_transfer(pDev);
                } else {
                        /* interrupt handler will pick up later */
                }
                SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);
        } else {
                if ( pDev->state == I2CMST_STATE_IDLE ){
                        i2cmst_state_schedule(pDev, pDev->queue.head);
                }
                i2cmst_state_transfer(pDev);
        }

        return DRV_OK;
}

/* Take all available sent frames from the "Sent frames" queue.
 * If no frames has been sent, the thread may get blocked if in blocking
 * mode. The blocking mode is not available if driver is not in running mode.
 *
 * Note this ioctl may return success even if the driver is not in STARTED mode.
 * This is because in case of a error (link error of similar) and the driver switch
 * from START to STOP mode we must still be able to get our frames back.
 *
 * Note in case the driver fails to send a frame for some reason (link error),
 * the sent flag is set to 0 indicating a failure.
 *
 */
int i2cmst_reclaim(struct i2cmst_priv *pDev, struct i2cmst_list *chain)
{
        struct i2cmst_packet *curr, *last, *first;

        if (!pDev->config.isr_pkt_proc) {
                i2cmst_state_transfer(pDev);
        }

        if (chain) {
                SPIN_IRQFLAGS(oldLevel);
                chain->head = NULL;
                chain->tail = NULL;
                /* Take all sent frames from sent queue to user-space queue */
                last = NULL;

                SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
                first = pDev->queue.head;
                curr = pDev->queue.head;
                while(curr != NULL && (curr->flags & I2CMST_FLAGS_FINISHED)) {
                        last = curr;
                        curr = curr->next;
                }
                
                if (curr) {
                        pDev->queue.head = curr;
                } else {
                        pDev->queue.head = NULL;
                        pDev->queue.tail = NULL;
                }
                SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);

                if (last) {
                        chain->head = first;
                        chain->tail = last;
                        chain->tail->next = NULL;
                }

                if ( chain->head ){
                        DBG("[RECLAIM]\n");
                        return DRV_OK;
                } else {
                        /* non-blocking mode, we quit */
                        return DRV_WOULDBLOCK;
                }
        }

        return DRV_OK;
}

static void i2cmst_interrupt(void *arg)
{
        SPIN_ISR_IRQFLAGS(irqflags);
        struct i2cmst_priv *pDev = arg;
        volatile struct i2cmst_regs *regs = pDev->regs;
        uint32_t status;

        SPIN_LOCK(&pDev->devlock, irqflags);
        status = regs->c.status;
        /* Clear interrupt */
        regs->c.cmd = I2CMST_CMD_IACK;

        /* Spurious Interrupt? */
        if ( !pDev->running ) {
                SPIN_UNLOCK(&pDev->devlock, irqflags);
                return;
        }

        if (!(status & I2CMST_STATUS_TIP)) {
                if ( pDev->config.isr_pkt_proc ) {
                        i2cmst_state_transfer(pDev);
                }
        }
        SPIN_UNLOCK(&pDev->devlock, irqflags);
}

