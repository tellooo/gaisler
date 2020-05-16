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
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <drv/grcan.h>
#include <drv/osal.h>

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int grcan_register(struct grcan_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int grcan_init(struct grcan_devcfg *devcfgs[])
{
        struct grcan_devcfg **dev = &devcfgs[0];

        while (*dev) {
                grcan_register(*dev);
                dev++;
        }
        return DRV_OK;
}

#define FUNCDBG()
#define DBG(a, ...)
#define DBGC(a, ...)

#define WRAP_AROUND_TX_MSGS 1
#define GRCAN_MSG_SIZE sizeof(struct grcan_msg)

#ifndef GRCAN_DEFAULT_BAUD
 /* default to 500kbits/s */
 #define GRCAN_DEFAULT_BAUD 500000
#endif

#ifndef GRCAN_SAMPLING_POINT
 #define GRCAN_SAMPLING_POINT 80
#endif

int state2err[4] = {
        [STATE_STOPPED]  = GRCAN_RET_NOTSTARTED,
        [STATE_STARTED]  = GRCAN_RET_OK,
        [STATE_BUSOFF]   = GRCAN_RET_BUSOFF,
        [STATE_AHBERR]   = GRCAN_RET_AHBERR,
};

struct grcan_msg {
        unsigned int head[2];
        unsigned char data[8];
};

static void grcan_hw_reset(struct grcan_regs *regs);

static int grcan_hw_read_try(
        struct grcan_priv *pDev,
        struct grcan_regs *regs,
        struct grcan_canmsg *buffer,
        int max);

static int grcan_hw_write_try(
        struct grcan_priv *pDev,
        struct grcan_regs *regs,
        struct grcan_canmsg *buffer,
        int count);

static void grcan_hw_config(
        struct grcan_regs *regs,
        struct grcan_config *conf);

static void grcan_hw_accept(
        struct grcan_regs *regs,
        struct grcan_filter *afilter);

static void grcan_hw_sync(
        struct grcan_regs *regs,
        struct grcan_filter *sfilter);

static void grcan_interrupt(void *arg);

static inline unsigned int READ_REG(volatile unsigned int *addr)
{
        return *addr;
}

#ifdef GRCAN_DMA_BYPASS_CACHE
#define READ_DMA_WORD(address) _grcan_read_nocache((unsigned int)(address))
#define READ_DMA_BYTE(address) _grcan_read_nocache_byte((unsigned int)(address))
static unsigned char __inline__ _grcan_read_nocache_byte(unsigned int address)
{
        unsigned char tmp;
        __asm__ (" lduba [%1]1, %0 "
            : "=r"(tmp)
            : "r"(address)
        );
        return tmp;
}
static unsigned int __inline__ _grcan_read_nocache(unsigned int address)
{
        unsigned int tmp;
        __asm__ (" lda [%1]1, %0 "
                : "=r"(tmp)
                : "r"(address)
        );
        return tmp;
}
#else
#define READ_DMA_WORD(address) (*(volatile unsigned int *)(address))
#define READ_DMA_BYTE(address) (*(volatile unsigned char *)(address))
#endif

static void grcan_hw_reset(struct grcan_regs *regs)
{
        regs->ctrl = GRCAN_CTRL_RESET;
}

static int grcan_hw_start(struct grcan_priv *pDev)
{
        unsigned int tmp;
        (void) sizeof tmp;

        FUNCDBG();

        /* Check that memory has been allocated successfully */
        if (!pDev->tx || !pDev->rx)
                return DRV_NOMEM;

        /* Configure FIFO configuration register
         * and Setup timing
         */
        if (pDev->config_changed) {
                grcan_hw_config(pDev->regs, &pDev->config);
                pDev->config_changed = 0;
        }

        /* Setup receiver */
        pDev->regs->rx0addr = (unsigned int)pDev->rx;
        pDev->regs->rx0size = pDev->rxbuf_size;

        /* Setup Transmitter */
        pDev->regs->tx0addr = (unsigned int)pDev->tx;
        pDev->regs->tx0size = pDev->txbuf_size;

        /* Setup acceptance filters */
        grcan_hw_accept(pDev->regs, &pDev->afilter);

        /* Sync filters */
        grcan_hw_sync(pDev->regs, &pDev->sfilter);

        /* Clear status bits */
        tmp = READ_REG(&pDev->regs->stat);
        pDev->regs->stat = 0;

        /* Setup IRQ handling */

        /* Clear all IRQs */
        tmp = READ_REG(&pDev->regs->pir);
        pDev->regs->picr = 0x1ffff;

        pDev->regs->imr = (
                GRCAN_RXAHBERR_IRQ |
                GRCAN_TXAHBERR_IRQ |
                GRCAN_OFF_IRQ |
                0
        );

        /* Enable receiver/transmitter */
        pDev->regs->rx0ctrl = GRCAN_RXCTRL_ENABLE;
        pDev->regs->tx0ctrl = GRCAN_TXCTRL_ENABLE;

        /* Enable HurriCANe core */
        pDev->regs->ctrl = GRCAN_CTRL_ENABLE;

        /* Leave transmitter disabled, it is enabled when
         * trying to send something.
         */
        return DRV_OK;
}

static void grcan_hw_stop(struct grcan_priv *pDev)
{
        FUNCDBG();

        /* Mask all IRQs */
        pDev->regs->imr = 0;

        /* Disable receiver & transmitter */
        pDev->regs->rx0ctrl = 0;
        pDev->regs->tx0ctrl = 0;
}

static void grcan_hw_config(struct grcan_regs *regs, struct grcan_config *conf)
{
        unsigned int config = 0;

        /* Reset HurriCANe Core */
        regs->ctrl = 0;

        if (conf->silent)
                config |= GRCAN_CFG_SILENT;

        if (conf->abort)
                config |= GRCAN_CFG_ABORT;

        if (conf->selection.selection)
                config |= GRCAN_CFG_SELECTION;

        if (conf->selection.enable0)
                config |= GRCAN_CFG_ENABLE0;

        if (conf->selection.enable1)
                config |= GRCAN_CFG_ENABLE1;

        /* Timing */
        config |= (conf->timing.bpr << GRCAN_CFG_BPR_BIT) & GRCAN_CFG_BPR;
        config |= (conf->timing.rsj << GRCAN_CFG_RSJ_BIT) & GRCAN_CFG_RSJ;
        config |= (conf->timing.ps1 << GRCAN_CFG_PS1_BIT) & GRCAN_CFG_PS1;
        config |= (conf->timing.ps2 << GRCAN_CFG_PS2_BIT) & GRCAN_CFG_PS2;
        config |=
            (conf->timing.scaler << GRCAN_CFG_SCALER_BIT) & GRCAN_CFG_SCALER;

        /* Write configuration */
        regs->conf = config;

        /* Enable HurriCANe Core */
        regs->ctrl = GRCAN_CTRL_ENABLE;
}

static void grcan_hw_accept(
        struct grcan_regs *regs,
        struct grcan_filter *afilter
)
{
        /* Disable Sync mask totaly (if we change scode or smask
         * in an unfortunate way we may trigger a sync match)
         */
        regs->rx0mask = 0xffffffff;

        /* Set Sync Filter in a controlled way */
        regs->rx0code = afilter->code;
        regs->rx0mask = afilter->mask;
}

static void grcan_hw_sync(struct grcan_regs *regs, struct grcan_filter *sfilter)
{
        /* Disable Sync mask totaly (if we change scode or smask
         * in an unfortunate way we may trigger a sync match)
         */
        regs->smask = 0xffffffff;

        /* Set Sync Filter in a controlled way */
        regs->scode = sfilter->code;
        regs->smask = sfilter->mask;
}

static unsigned int grcan_hw_rxavail(
        unsigned int rp,
        unsigned int wp, unsigned int size
)
{
        if (rp == wp) {
                /* read pointer and write pointer is equal only
                 * when RX buffer is empty.
                 */
                return 0;
        }

        if (wp > rp) {
                return (wp - rp) / GRCAN_MSG_SIZE;
        } else {
                return (size - (rp - wp)) / GRCAN_MSG_SIZE;
        }
}

static unsigned int grcan_hw_txspace(
        unsigned int rp,
        unsigned int wp,
        unsigned int size
)
{
        unsigned int left;

        if (rp == wp) {
                /* read pointer and write pointer is equal only
                 * when TX buffer is empty.
                 */
                return size / GRCAN_MSG_SIZE - WRAP_AROUND_TX_MSGS;
        }

        /* size - 4 - abs(read-write) */
        if (wp > rp) {
                left = size - (wp - rp);
        } else {
                left = rp - wp;
        }

        return left / GRCAN_MSG_SIZE - WRAP_AROUND_TX_MSGS;
}

#define MIN_TSEG1 1
#define MIN_TSEG2 2
#define MAX_TSEG1 14
#define MAX_TSEG2 8

static int grcan_calc_timing(
        unsigned int baud,      /* The requested BAUD to calculate timing for */
        unsigned int core_hz,   /* Frequency in Hz of GRCAN Core */
        unsigned int sampl_pt,
        struct grcan_timing *timing     /* result is placed here */
)
{
        int best_error = 1000000000;
        int error;
        int best_tseg = 0, best_brp = 0, brp = 0;
        int tseg = 0, tseg1 = 0, tseg2 = 0;
        int sjw = 1;

        /* Default to 90% */
        if ((sampl_pt < 50) || (sampl_pt > 99)) {
                sampl_pt = GRCAN_SAMPLING_POINT;
        }

        if ((baud < 5000) || (baud > 1000000)) {
                /* invalid speed mode */
                return -1;
        }

        /* find best match, return -2 if no good reg
         * combination is available for this frequency
         */

        /* some heuristic specials */
        if (baud > ((1000000 + 500000) / 2))
                sampl_pt = 75;

        if (baud < ((12500 + 10000) / 2))
                sampl_pt = 75;

        /* tseg even = round down, odd = round up */
        for (
                tseg = (MIN_TSEG1 + MIN_TSEG2 + 2) * 2;
                tseg <= (MAX_TSEG2 + MAX_TSEG1 + 2) * 2 + 1;
                tseg++
        ) {
                brp = core_hz / ((1 + tseg / 2) * baud) + tseg % 2;
                if (
                        (brp <= 0) ||
                        ((brp > 256 * 1) && (brp <= 256 * 2) && (brp & 0x1)) ||
                        ((brp > 256 * 2) && (brp <= 256 * 4) && (brp & 0x3)) ||
                        ((brp > 256 * 4) && (brp <= 256 * 8) && (brp & 0x7)) ||
                        (brp > 256 * 8)
                )
                        continue;

                error = baud - core_hz / (brp * (1 + tseg / 2));
                if (error < 0) {
                        error = -error;
                }

                if (error <= best_error) {
                        best_error = error;
                        best_tseg = tseg / 2;
                        best_brp = brp - 1;
                }
        }

        if (best_error && (baud / best_error < 10)) {
                return -2;
        } else if (!timing)
                return 0;       /* nothing to store result in, but a valid bitrate can be calculated */

        tseg2 = best_tseg - (sampl_pt * (best_tseg + 1)) / 100;

        if (tseg2 < MIN_TSEG2) {
                tseg2 = MIN_TSEG2;
        }

        if (tseg2 > MAX_TSEG2) {
                tseg2 = MAX_TSEG2;
        }

        tseg1 = best_tseg - tseg2 - 2;

        if (tseg1 > MAX_TSEG1) {
                tseg1 = MAX_TSEG1;
                tseg2 = best_tseg - tseg1 - 2;
        }

        /* Get scaler and BRP from pseudo BRP */
        if (best_brp <= 256) {
                timing->scaler = best_brp;
                timing->bpr = 0;
        } else if (best_brp <= 256 * 2) {
                timing->scaler = ((best_brp + 1) >> 1) - 1;
                timing->bpr = 1;
        } else if (best_brp <= 256 * 4) {
                timing->scaler = ((best_brp + 1) >> 2) - 1;
                timing->bpr = 2;
        } else {
                timing->scaler = ((best_brp + 1) >> 3) - 1;
                timing->bpr = 3;
        }

        timing->ps1 = tseg1 + 1;
        timing->ps2 = tseg2;
        timing->rsj = sjw;

        return 0;
}

static int grcan_hw_read_try(
        struct grcan_priv *pDev,
        struct grcan_regs *regs,
        struct grcan_canmsg * buffer,
        int max
)
{
        int i, j;
        struct grcan_canmsg *dest;
        struct grcan_msg *source, tmp;
        unsigned int wp, rp, size, rxmax, addr;
        int trunk_msg_cnt;

        FUNCDBG();

        wp = READ_REG(&regs->rx0wr);
        rp = READ_REG(&regs->rx0rd);

        /*
         * Due to hardware wrap around simplification write pointer will
         * never reach the read pointer, at least a gap of 8 bytes.
         * The only time they are equal is when the read pointer has
         * reached the write pointer (empty buffer)
         *
         */
        if (wp != rp) {
                /* Not empty, we have received chars...
                 * Read as much as possible from DMA buffer
                 */
                size = READ_REG(&regs->rx0size);

                /* Get number of bytes available in RX buffer */
                trunk_msg_cnt = grcan_hw_rxavail(rp, wp, size);

                /* truncate size if user space buffer hasn't room for
                 * all received chars.
                 */
                if (trunk_msg_cnt > max)
                        trunk_msg_cnt = max;

                /* Read until i is 0 */
                i = trunk_msg_cnt;

                addr = (unsigned int)pDev->rx;
                source = (struct grcan_msg *)(addr + rp);
                dest = buffer;
                rxmax = addr + (size - GRCAN_MSG_SIZE);

                /* Read as many can messages as possible */
                while (i > 0) {
                        /* Read CAN message from DMA buffer */
                        tmp.head[0] = READ_DMA_WORD(&source->head[0]);
                        tmp.head[1] = READ_DMA_WORD(&source->head[1]);
                        if (tmp.head[1] & 0x4) {
                                DBGC(DBG_RX, "overrun\n");
                        }
                        if (tmp.head[1] & 0x2) {
                                DBGC(DBG_RX, "bus-off mode\n");
                        }
                        if (tmp.head[1] & 0x1) {
                                DBGC(DBG_RX, "error-passive mode\n");
                        }
                        /* Convert one grcan CAN message to one "software" CAN message */
                        dest->extended = tmp.head[0] >> 31;
                        dest->rtr = (tmp.head[0] >> 30) & 0x1;
                        if (dest->extended) {
                                dest->id = tmp.head[0] & 0x3fffffff;
                        } else {
                                dest->id = (tmp.head[0] >> 18) & 0xfff;
                        }
                        dest->len = tmp.head[1] >> 28;
                        for (j = 0; j < dest->len; j++) {
                                dest->data[j] = READ_DMA_BYTE(&source->data[j]);
                        }

                        /* wrap around if neccessary */
                        source =
                            ((unsigned int)source >= rxmax) ?
                            (struct grcan_msg *)addr : source + 1;
                        dest++; /* straight user buffer */
                        i--;
                }
                {
                        /* A bus off interrupt may have occured after checking pDev->started */
                        SPIN_IRQFLAGS(oldLevel);

                        SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
                        if (pDev->started == STATE_STARTED) {
                                regs->rx0rd = (unsigned int) source - addr;
                                regs->rx0ctrl = GRCAN_RXCTRL_ENABLE;
                        } else {
                                DBGC(DBG_STATE, "cancelled due to a BUS OFF error\n");
                                trunk_msg_cnt = state2err[pDev->started];
                        }
                        SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);
                }
                return trunk_msg_cnt;
        }
        return 0;
}

static int grcan_hw_write_try(
        struct grcan_priv *pDev,
        struct grcan_regs *regs,
        struct grcan_canmsg * buffer,
        int count
)
{
        unsigned int rp, wp, size, txmax, addr;
        int ret;
        struct grcan_msg *dest;
        struct grcan_canmsg *source;
        int space_left;
        unsigned int tmp;
        int i;

        DBGC(DBG_TX, "\n");
        /*FUNCDBG(); */

        rp = READ_REG(&regs->tx0rd);
        wp = READ_REG(&regs->tx0wr);
        size = READ_REG(&regs->tx0size);

        space_left = grcan_hw_txspace(rp, wp, size);

        /* is circular fifo full? */
        if (space_left < 1) {
                return 0;
        }

        /* Truncate size */
        if (space_left > count) {
                space_left = count;
        }
        ret = space_left;

        addr = (unsigned int)pDev->tx;

        dest = (struct grcan_msg *)(addr + wp);
        source = (struct grcan_canmsg *) buffer;
        txmax = addr + (size - GRCAN_MSG_SIZE);

        while (space_left > 0) {
                /* Convert and write CAN message to DMA buffer */
                if (source->extended) {
                        tmp = (1 << 31) | (source->id & 0x3fffffff);
                } else {
                        tmp = (source->id & 0xfff) << 18;
                }
                if (source->rtr) {
                        tmp |= (1 << 30);
                }
                dest->head[0] = tmp;
                dest->head[1] = source->len << 28;
                for (i = 0; i < source->len; i++) {
                        dest->data[i] = source->data[i];
                }
                source++;       /* straight user buffer */
                dest =
                    ((unsigned int)dest >= txmax) ?
                    (struct grcan_msg *)addr : dest + 1;
                space_left--;
        }

        /* A bus off interrupt may have occured after checking pDev->started */
        SPIN_IRQFLAGS(oldLevel);

        SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
        if (pDev->started != STATE_STARTED) {
                DBGC(DBG_STATE, "cancelled due to a BUS OFF error\n");
                ret = state2err[pDev->started];
                SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);
                return ret;
        }

        regs->tx0wr = (unsigned int) dest - addr;
        regs->tx0ctrl = GRCAN_TXCTRL_ENABLE;
        SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);

        return ret;
}

int grcan_istxdone(struct grcan_priv *pDev)
{
        unsigned int rp, wp;
        FUNCDBG();

        /* loop until all data in circular buffer has been read by hw.
         * (write pointer != read pointer )
         *
         * Hardware doesn't update write pointer - we do
         */
        wp = READ_REG(&pDev->regs->tx0wr);
        rp = READ_REG(&pDev->regs->tx0rd);
        if (wp == rp) {
                return 1;
        }
        return 0;
}

int grcan_dev_count(void)
{
        return dev_count;
}

extern struct grcan_priv *grcan_open_userbuf(
        int dev_no,
        void *rxbuf,
        int rxbuf_size,
        void *txbuf,
        int txbuf_size
)
{
        struct grcan_priv *pDev;
        void *ret;

        FUNCDBG();

        if (dev_no < 0 || dev_count <= dev_no) {
                return NULL;
        }

        struct grcan_devcfg *dev =
            (struct grcan_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        pDev = &dev->priv;

        uint8_t popen = osal_ldstub(&pDev->open);
        if (popen) {
                return NULL;
        }

        SPIN_INIT(&pDev->devlock, "thegrcan");

        pDev->regs = (void *)dev->regs.addr;
        pDev->corefreq_hz = osal_busfreq();
        pDev->irq = dev->regs.interrupt;
        pDev->started = STATE_STOPPED;
        pDev->config_changed = 1;
        pDev->config.silent = 0;
        pDev->config.abort = 0;
        pDev->config.selection.selection = 0;
        pDev->config.selection.enable0 = 0;
        pDev->config.selection.enable1 = 1;
        pDev->rxa = pDev->txa = NULL;
        pDev->rx = rxbuf;
        pDev->tx = txbuf;
        pDev->txbuf_size = txbuf_size;
        pDev->rxbuf_size = rxbuf_size;

        DBG("rxbuf_size: %d, txbuf_size: %d\n", txbuf_size, rxbuf_size);

        /* Default to accept all messages */
        pDev->afilter.mask = 0x00000000;
        pDev->afilter.code = 0x00000000;

        /* Default to disable sync messages (only trigger when id is set to all ones) */
        pDev->sfilter.mask = 0xffffffff;
        pDev->sfilter.code = 0x00000000;

        /* Calculate default timing register values */
        grcan_calc_timing(GRCAN_DEFAULT_BAUD,pDev->corefreq_hz,GRCAN_SAMPLING_POINT,&pDev->config.timing);

        /* Clear statistics */
        memset(&pDev->stats,0,sizeof(struct grcan_stats));

        grcan_hw_reset(pDev->regs);

        ret = pDev;
        return ret;
}

int grcan_close_userbuf(struct grcan_priv *pDev)
{
        FUNCDBG();

        grcan_stop(pDev);

        grcan_hw_reset(pDev->regs);

        /* Mark Device as closed */
        pDev->open = 0;

        return 0;
}

int grcan_read(struct grcan_priv *pDev, struct grcan_canmsg *msg, size_t ucount)
{
        struct grcan_canmsg *dest;
        int nread;
        int req_cnt;

        FUNCDBG();

        dest = msg;
        req_cnt = ucount;

        if ( (!dest) || (req_cnt<1) )
                return GRCAN_RET_INVARG;

        if (pDev->started != STATE_STARTED) {
                return GRCAN_RET_NOTSTARTED;
        }

        DBGC(DBG_RX, "grcan_read [%p]: buf: %p len: %u\n", d, msg, (unsigned int) ucount);

        nread = grcan_hw_read_try(pDev,pDev->regs,dest,req_cnt);
        return nread;
}

int grcan_write(struct grcan_priv *pDev, struct grcan_canmsg *msg, size_t ucount)
{
        struct grcan_canmsg *source;
        int nwritten;
        int req_cnt;

        DBGC(DBG_TX,"\n");

        if ((pDev->started != STATE_STARTED) || pDev->config.silent)
                return GRCAN_RET_NOTSTARTED;

        req_cnt = ucount;
        source = (struct grcan_canmsg *) msg;

        /* check proper length and buffer pointer */
        if (( req_cnt < 1) || (source == NULL) ){
                return GRCAN_RET_INVARG;
        }

        nwritten = grcan_hw_write_try(pDev,pDev->regs,source,req_cnt);
        return nwritten;
}

int grcan_start(struct grcan_priv *pDev)
{
        FUNCDBG();

        if (grcan_get_state(pDev) == STATE_STARTED) {
                return GRCAN_RET_INVARG;
        }

        if ( (grcan_hw_start(pDev)) != DRV_OK ){
                return GRCAN_RET_NOTSTARTED;
        }

        /* Read and write are now open... */
        pDev->started = STATE_STARTED;
        DBGC(DBG_STATE, "STOPPED|BUSOFF|AHBERR->STARTED\n");

        /* Register interrupt routine and enable IRQ at IRQ ctrl */
        int ret;
        ret = osal_isr_register(
                &pDev->isr_ctx,
                pDev->irq,
                grcan_interrupt,
                pDev
        );
        if (DRV_OK != ret) {
                grcan_stop(pDev);
                return GRCAN_RET_NOTSTARTED;
        }

        return GRCAN_RET_OK;
}

int grcan_stop(struct grcan_priv *pDev)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        if (pDev->started == STATE_STOPPED)
                return GRCAN_RET_INVARG;

        SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
        if (pDev->started == STATE_STARTED) {
                grcan_hw_stop(pDev);
                DBGC(DBG_STATE, "STARTED->STOPPED\n");
        } else {
                /*
                 * started == STATE_[STOPPED|BUSOFF|AHBERR] so grcan_hw_stop()
                 * might already been called from ISR.
                 */
                DBGC(DBG_STATE, "[STOPPED|BUSOFF|AHBERR]->STOPPED\n");
        }
        pDev->started = STATE_STOPPED;
        SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);

        /* Disable interrupts */
        osal_isr_unregister(
                &pDev->isr_ctx,
                pDev->irq,
                grcan_interrupt,
                pDev
        );

        return GRCAN_RET_OK;
}

int grcan_get_state(struct grcan_priv *pDev)
{
        FUNCDBG();

        return pDev->started;
}

int grcan_set_silent(struct grcan_priv *pDev, int silent)
{
        FUNCDBG();

        if (pDev->started == STATE_STARTED)
                return -1;

        pDev->config.silent = silent;
        pDev->config_changed = 1;

        return 0;
}

int grcan_set_abort(struct grcan_priv *pDev, int abort)
{
        FUNCDBG();

        if (pDev->started == STATE_STARTED)
                return -1;

        pDev->config.abort = abort;
        /* This Configuration parameter doesn't need HurriCANe reset
         * ==> no pDev->config_changed = 1;
         */

        return 0;
}

int grcan_set_selection(struct grcan_priv *pDev, const struct grcan_selection *selection)
{
        FUNCDBG();

        if (pDev->started == STATE_STARTED)
                return -1;

        if ( !selection )
                return -2;

        pDev->config.selection = *selection;
        pDev->config_changed = 1;

        return 0;
}

int grcan_get_stats(struct grcan_priv *pDev, struct grcan_stats *stats)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        if ( !stats )
                return -1;

        SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
        *stats = pDev->stats;
        SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);

        return 0;
}

int grcan_clr_stats(struct grcan_priv *pDev)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
        memset(&pDev->stats,0,sizeof(struct grcan_stats));
        SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);

        return 0;
}

int grcan_set_speed(struct grcan_priv *pDev, unsigned int speed)
{
        struct grcan_timing timing;
        int ret;

        FUNCDBG();

        /* cannot change speed during run mode */
        if (pDev->started == STATE_STARTED)
                return GRCAN_RET_INVSTATE;

        /* get speed rate from argument */
        ret = grcan_calc_timing(speed, pDev->corefreq_hz, GRCAN_SAMPLING_POINT, &timing);
        if ( ret )
                return GRCAN_RET_INVARG;

        /* save timing/speed */
        pDev->config.timing = timing;
        pDev->config_changed = 1;

        return 0;
}

int grcan_set_btrs(struct grcan_priv *pDev, const struct grcan_timing *timing)
{
        FUNCDBG();

        /* Set BTR registers manually
         * Read GRCAN/HurriCANe Manual.
         */
        if (pDev->started == STATE_STARTED)
                return -1;

        if ( !timing )
                return -2;

        pDev->config.timing = *timing;
        pDev->config_changed = 1;

        return 0;
}

int grcan_set_afilter(struct grcan_priv *pDev, const struct grcan_filter *filter)
{
        FUNCDBG();

        if ( !filter ){
                /* Disable filtering - let all messages pass */
                pDev->afilter.mask = 0x0;
                pDev->afilter.code = 0x0;
        }else{
                /* Save filter */
                pDev->afilter = *filter;
        }
        /* Set hardware acceptance filter */
        grcan_hw_accept(pDev->regs,&pDev->afilter);

        return 0;
}

int grcan_set_sfilter(struct grcan_priv *pDev, const struct grcan_filter *filter)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        if ( !filter ){
                /* disable TX/RX SYNC filtering */
                pDev->sfilter.mask = 0xffffffff;
                pDev->sfilter.mask = 0;

                 /* disable Sync interrupt */
                SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
                pDev->regs->imr = READ_REG(&pDev->regs->imr) & ~(GRCAN_RXSYNC_IRQ|GRCAN_TXSYNC_IRQ);
                SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);
        }else{
                /* Save filter */
                pDev->sfilter = *filter;

                /* Enable Sync interrupt */
                SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);
                pDev->regs->imr = READ_REG(&pDev->regs->imr) | (GRCAN_RXSYNC_IRQ|GRCAN_TXSYNC_IRQ);
                SPIN_UNLOCK_IRQ(&pDev->devlock, oldLevel);
        }
        /* Set Sync RX/TX filter */
        grcan_hw_sync(pDev->regs,&pDev->sfilter);

        return 0;
}

int grcan_get_status(struct grcan_priv *pDev, unsigned int *data)
{
        FUNCDBG();

        if ( !data )
                return -1;

        /* Read out the statsu register from the GRCAN core */
        data[0] = READ_REG(&pDev->regs->stat);

        return 0;
}

/* The interrupt is unmasked */
int grcan_txint(struct grcan_priv *priv, int txint)
{
        struct grcan_regs *regs = priv->regs;
        unsigned int imr;
        unsigned int imr_set;
        SPIN_IRQFLAGS(oldLevel);

        SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);

        imr_set = 0;
        if (txint == 1) {
                imr_set = GRCAN_TX_IRQ;
        } else if (txint == -1) {
                imr_set = GRCAN_TXEMPTY_IRQ;
        }

        /* Clear pending Tx IRQ */
        regs->picr = imr_set;
        imr = READ_REG(&regs->imr);
        imr &= ~(GRCAN_TX_IRQ | GRCAN_TXEMPTY_IRQ);
        imr |= imr_set;
        regs->imr = imr;

        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        return 0;
}

int grcan_rxint(struct grcan_priv *priv, int rxint)
{
        struct grcan_regs *regs = priv->regs;
        unsigned int imr;
        unsigned int imr_set;
        SPIN_IRQFLAGS(oldLevel);

        SPIN_LOCK_IRQ(&pDev->devlock, oldLevel);

        imr_set = 0;
        if (rxint == 1) {
                imr_set = GRCAN_RX_IRQ;
        } else if (rxint == -1) {
                imr_set = GRCAN_RXFULL_IRQ;
        }

        /* Clear pending Rx IRQ */
        regs->picr = imr_set;
        imr = READ_REG(&regs->imr);
        imr &= ~(GRCAN_RX_IRQ | GRCAN_RXFULL_IRQ);
        imr |= imr_set;
        regs->imr = imr;

        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        return 0;
}

void grcan_set_isr(
        struct grcan_priv *priv,
        void (*isr)(struct grcan_priv *priv, void *data),
        void *data
)
{
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        priv->userisr = isr;
        priv->userisr_data = data;

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
}

/* Error indicators */
#define GRCAN_IRQ_ERRORS \
                (GRCAN_RXAHBERR_IRQ | GRCAN_TXAHBERR_IRQ | GRCAN_OFF_IRQ)
#define GRCAN_STAT_ERRORS (GRCAN_STAT_AHBERR | GRCAN_STAT_OFF)
/* Warning & RX/TX sync indicators */
#define GRCAN_IRQ_WARNS \
                (GRCAN_ERR_IRQ | GRCAN_OR_IRQ | GRCAN_TXLOSS_IRQ | \
                 GRCAN_RXSYNC_IRQ | GRCAN_TXSYNC_IRQ)
#define GRCAN_STAT_WARNS (GRCAN_STAT_OR | GRCAN_STAT_PASS)

/* Handle the IRQ */
static void grcan_interrupt(void *arg)
{
        struct grcan_priv *pDev = arg;
        struct grcan_regs *regs = pDev->regs;
        unsigned int status = READ_REG(&regs->pimsr);
        unsigned int canstat = READ_REG(&regs->stat);
        SPIN_ISR_IRQFLAGS(irqflags);

        /* Spurious IRQ call? */
        if ( !status && !canstat )
                return;

        if (pDev->started != STATE_STARTED) {
                DBGC(DBG_STATE, "not STARTED (unexpected interrupt)\n");
                regs->picr = status;
                return;
        }

        FUNCDBG();

        if ( (status & GRCAN_IRQ_ERRORS) || (canstat & GRCAN_STAT_ERRORS) ) {
                /* Bus-off condition interrupt
                 * The link is brought down by hardware, we wake all threads
                 * that is blocked in read/write calls and stop futher calls
                 * to read/write until user has called ioctl(fd,START,0).
                 */
                SPIN_LOCK(&pDev->devlock, irqflags);
                DBGC(DBG_STATE, "STARTED->BUSOFF|AHBERR\n");
                pDev->stats.ints++;
                if ((status & GRCAN_OFF_IRQ) || (canstat & GRCAN_STAT_OFF)) {
                        /* CAN Bus-off interrupt */
                        DBGC(DBG_STATE, "BUSOFF: status: 0x%x, canstat: 0x%x\n",
                                status, canstat);
                        pDev->started = STATE_BUSOFF;
                        pDev->stats.busoff_cnt++;
                } else {
                        /* RX or Tx AHB Error interrupt */
                        //printk("AHBERROR: status: 0x%x, canstat: 0x%x\n",
                        //      status, canstat);
                        pDev->started = STATE_AHBERR;
                        pDev->stats.ahberr_cnt++;
                }
                grcan_hw_stop(pDev); /* this mask all IRQ sources */
                regs->picr = 0x1ffff; /* clear all interrupts */
                /*
                 * Prevent driver from affecting bus. Driver can be started
                 * again with grcan_start().
                 */
                SPIN_UNLOCK(&pDev->devlock, irqflags);

                /*
                 * NOTE: Another interrupt may be pending now so ISR could be
                 * executed one more time aftert this (first) return.
                 */
                return;
        }

        SPIN_LOCK(&pDev->devlock, irqflags);

        /* Increment number of interrupts counter */
        pDev->stats.ints++;
        if ((status & GRCAN_IRQ_WARNS) || (canstat & GRCAN_STAT_WARNS)) {

                if ( (status & GRCAN_ERR_IRQ) || (canstat & GRCAN_STAT_PASS) ) {
                        /* Error-Passive interrupt */
                        pDev->stats.passive_cnt++;
                }

                if ( (status & GRCAN_OR_IRQ) || (canstat & GRCAN_STAT_OR) ) {
                        /* Over-run during reception interrupt */
                        pDev->stats.overrun_cnt++;
                }

                if ( status & GRCAN_TXLOSS_IRQ ) {
                        pDev->stats.txloss_cnt++;
                }

                if ( status & GRCAN_TXSYNC_IRQ ) {
                        /* TxSync message transmitted interrupt */
                        pDev->stats.txsync_cnt++;
                }

                if ( status & GRCAN_RXSYNC_IRQ ) {
                        /* RxSync message received interrupt */
                        pDev->stats.rxsync_cnt++;
                }
        }

        SPIN_UNLOCK(&pDev->devlock, irqflags);

        /* Clear IRQs */
        regs->picr = status;

        /* Delegate to user function, if installed. */
        if (pDev->userisr) {
                (pDev->userisr) (pDev, pDev->userisr_data);
        }
}

