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

/*** GRSPW2 driver DMA interface ***/

#include <stdlib.h>

#include <drv/grspw.h>
#include <drv/nelem.h>
#include <drv/osal.h>
#include <drv/drvret.h>

void grspw_hw_dma_stop(struct grspw_dma_priv *dma)
{
        volatile struct grspw_dma_regs *dregs = dma->regs;
        uint32_t ctrl;
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        ctrl = dregs->ctrl & (GRSPW_DMACTRL_LE | GRSPW_DMACTRL_EN |
                              GRSPW_DMACTRL_SP | GRSPW_DMACTRL_SA |
                              GRSPW_DMACTRL_NS);
        /* Abort currently transmitting packet and disable transmissions. */
        ctrl |= GRSPW_DMACTRL_AT;
        dregs->ctrl = ctrl;

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
}

/* Soft-reset DMA registers. Assumes DMA is not open. */
void grspw_hw_dma_softreset(struct grspw_dma_priv *dma)
{
        uint32_t ctrl;
        volatile struct grspw_dma_regs *dregs = dma->regs;

        ctrl = dregs->ctrl & (GRSPW_DMACTRL_LE | GRSPW_DMACTRL_EN);
        dregs->ctrl = ctrl;

        dregs->rxmax = DEFAULT_RXMAX;
        dregs->txdesc = 0;
        dregs->rxdesc = 0;
}

void grspw_dma_stop(struct grspw_dma_priv *dma)
{
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        if (dma->started == 0) {
                goto out;
        }

        dma->started = 0;
        grspw_hw_dma_stop(dma);

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
}

/* Initialize the RX and TX Descriptor Ring, with no packets. */
static void grspw_bdrings_init(struct grspw_dma_priv *dma)
{
        struct grspw_ring *r;

        /* Empty BD rings */
        dma->rx_ring_head = dma->rx_ring_base;
        dma->rx_ring_tail = dma->rx_ring_base;
        dma->tx_ring_head = dma->tx_ring_base;
        dma->tx_ring_tail = dma->tx_ring_base;

        /* Init RX Descriptors */
        r = (struct grspw_ring *)dma->rx_ring_base;
        for (int i = 0; i < GRSPW_RXBD_NR; i++) {
                /* Init Ring Entry */
                r[i].next = &r[i + 1];
                r[i].bd.rx = &dma->rx_bds[i];
                r[i].pkt = NULL;

                /* Init HW Descriptor */
                r[i].bd.rx->ctrl = 0;
                r[i].bd.rx->addr = 0;
        }
        r[GRSPW_RXBD_NR - 1].next = &r[0];

        /* Init TX Descriptors */
        r = (struct grspw_ring *)dma->tx_ring_base;
        for (int i = 0; i < GRSPW_TXBD_NR; i++) {
                /* Init Ring Entry */
                r[i].next = &r[i + 1];
                r[i].bd.tx = &dma->tx_bds[i];
                r[i].pkt = NULL;

                /* Init HW Descriptor */
                r[i].bd.tx->ctrl = 0;
                r[i].bd.tx->haddr = 0;
                r[i].bd.tx->dlen = 0;
                r[i].bd.tx->daddr = 0;
        }
        r[GRSPW_TXBD_NR - 1].next = &r[0];
}

/*
 * Initialize internal DMA software structures
 *
 * - Clear all Queues
 * - init BD ring
 *
 * This function assumes channel is in stopped state.
 */
static void grspw_dma_reset(struct grspw_dma_priv *dma)
{
        /* Empty RX and TX queues */
        grspw_list_clr(&dma->rx_sched);
        grspw_list_clr(&dma->tx_sched);
        dma->rx_sched_cnt = 0;
        dma->tx_sched_cnt = 0;

        grspw_bdrings_init(dma);
        grspw_dma_stats_clr(dma);
}

struct grspw_dma_priv *grspw_dma_open_userbuf(
        struct grspw_priv *priv,
        int chan_no,
        struct grspw_ring *rx_ring,
        struct grspw_ring *tx_ring,
        struct grspw_rxbd *rx_bds,
        struct grspw_txbd *tx_bds
)
{
        struct grspw_dma_priv *dma;

        if (priv->hwsup.ndma_chans <= chan_no) {
                return NULL;
        }

        dma = &priv->dma[chan_no];

        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        if (dma->open) {
                dma = NULL;
                goto out;
        }

        dma->started = 0;

        /* Set Default Configuration:
         *
         *  - MAX RX Packet Length = DEFAULT_RXMAX
         *  - No spill (wait for descriptor to be activated)
         */
        dma->cfg.rxmaxlen = DEFAULT_RXMAX;
        dma->cfg.flags = DEFAULT_DMAFLAGS;

        /* Install user supplied buffers. */
        dma->rx_bds = rx_bds;
        dma->tx_bds = tx_bds;

        /* Memory for the two descriptor rings is preallocated. */
        dma->rx_ring_base = (struct grspw_rxring *)rx_ring;
        dma->tx_ring_base = (struct grspw_txring *)tx_ring;

        grspw_dma_reset(dma);

        /* Take the device */
        dma->open = 1;

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return dma;
}

int grspw_dma_close_userbuf(struct grspw_dma_priv *dma)
{
        int ret = DRV_NOTOPEN;
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        /*
         * We check open since grspw_dma_close is called on all DMA channels by
         * grspw_close.
         */
        if (!dma->open) {
                goto out;
        }

        grspw_dma_stop(dma);

        dma->open = 0;
        dma->ubuf = NULL;
        ret = DRV_OK;

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);

        return ret;
}

int grspw_dma_start(struct grspw_dma_priv *dma)
{
        volatile struct grspw_dma_regs *dregs = dma->regs;
        uint32_t ctrl;

        if (dma->started) {
                /* Already started */
                return DRV_STARTED;
        }

        /* Initialize software structures */
        grspw_dma_reset(dma);

        /* RX and TX is not enabled until user fills SEND and READY Queue
         * with SpaceWire Packet buffers. So we do not have to worry about
         * IRQs for this channel just yet. However other DMA channels
         * may be active.
         *
         * Some functionality that is not changed during started mode is set up
         * once and for all here:
         *
         *   - RX MAX Packet length
         *   - TX Descriptor base address to first BD in TX ring (not enabled)
         *   - RX Descriptor base address to first BD in RX ring (not enabled)
         *   - Strip PID
         *   - Strip Address
         *   - No Spill
         *   - Receiver Enable
         *   - disable on link error (LE)
         *
         * Note that the address register and the address enable bit in DMACTRL
         * register must be left untouched, they are configured on a GRSPW
         * core level.
         *
         * Note that the receiver is enabled here, but since descriptors are
         * not enabled the GRSPW core may stop/pause RX (if NS bit set) until
         * descriptors are enabled or it may ignore RX packets (NS=0) until
         * descriptors are enabled (writing RD bit).
         */

        dregs->txdesc = (uint32_t)dma->tx_bds;
        dregs->rxdesc = (uint32_t)dma->rx_bds;

        /* MAX Packet length */
        dregs->rxmax = dma->cfg.rxmaxlen;

        ctrl =
            GRSPW_DMACTRL_TI |
            GRSPW_DMACTRL_RI |
            GRSPW_DMACTRL_AI |
            GRSPW_DMACTRL_PS |
            GRSPW_DMACTRL_PR |
            GRSPW_DMACTRL_TA |
            GRSPW_DMACTRL_RA |
            GRSPW_DMACTRL_RE |
            (dma->cfg.flags & DMAFLAG_MASK) << GRSPW_DMACTRL_NS_BIT;

        /* Protect while read-modify-write of dregs->ctrl. */
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        ctrl |= dregs->ctrl & GRSPW_DMACTRL_EN;
        dregs->ctrl = ctrl;

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);

        dma->started = 1;       /* open up other DMA interfaces */
        return DRV_OK;
}

/* Try to populate descriptor ring with as many USER packets as possible. The
 * packets assigned with a descriptor are put in the end of the scheduled list.
 *
 * The number of Packets scheduled is returned.
 *
 *  - USER List -> TX-SCHED List
 *  - Descriptors are initialized and enabled for transmission
 */
static int grspw_tx_schedule_send(struct grspw_dma_priv *dma,
                                  struct grspw_list *user)
{
        int cnt;
        uint32_t ctrl;
        void *hwaddr;
        /* Each grspw_txring describes one descriptor element. */
        struct grspw_txring *curr_bd;
        struct grspw_pkt *curr_pkt;
        struct grspw_pkt *last_pkt = NULL;
        struct grspw_list lst;

        /* Is USER Q empty? */
        if (grspw_list_is_empty(user)) {
                return 0;
        }

        cnt = 0;
        lst.head = curr_pkt = user->head;

        curr_bd = dma->tx_ring_head;
        while (!curr_bd->pkt) {
                /* Assign Packet to descriptor */
                curr_bd->pkt = curr_pkt;

                /* Set up header transmission */
                if (curr_pkt->hdr && curr_pkt->hlen) {
                        hwaddr = curr_pkt->hdr;
                        curr_bd->bd->haddr = (uint32_t)hwaddr;
                        ctrl = GRSPW_TXBD_EN | curr_pkt->hlen;
                } else {
                        ctrl = GRSPW_TXBD_EN;
                }
                /* Enable IRQ generation and CRC options as specified
                 * by user.
                 */
                ctrl |= (curr_pkt->flags & TXPKT_FLAG_INPUT_MASK) << 8;

                if (curr_bd->next == dma->tx_ring_base) {
                        /*
                         * Wrap around (only needed when smaller descriptor
                         * table)
                         */
                        ctrl |= GRSPW_TXBD_WR;
                }

                /* Prepare descriptor address. */
                if (curr_pkt->data && curr_pkt->dlen) {
                        hwaddr = curr_pkt->data;
                        curr_bd->bd->daddr = (uint32_t)hwaddr;
                        curr_bd->bd->dlen = curr_pkt->dlen;
                } else {
                        curr_bd->bd->daddr = 0;
                        curr_bd->bd->dlen = 0;
                }

                /* Enable descriptor */
                curr_bd->bd->ctrl = ctrl;

                last_pkt = curr_pkt;
                /* At next while condition test, curr_bd->pkt is checked. */
                curr_bd = curr_bd->next;
                cnt++;

                /* Get Next Packet from Ready Queue */
                if (curr_pkt == user->tail) {
                        /* Handled all in user queue. */
                        curr_pkt = NULL;
                        break;
                }
                curr_pkt = curr_pkt->next;
        }

        /* Have Packets been scheduled? */
        if (cnt > 0) {
                /* Prepare list for insertion/deleation */
                lst.tail = last_pkt;

                /* Remove scheduled packets from user queue */
                grspw_list_remove_head_list(user, &lst);

                /* Insert scheduled packets into scheduled queue */
                grspw_list_append_list(&dma->tx_sched, &lst);
                dma->tx_sched_cnt += cnt;
                if (dma->stats.tx_sched_cnt_max < dma->tx_sched_cnt) {
                        dma->stats.tx_sched_cnt_max = dma->tx_sched_cnt;
                }

                /* Update TX ring posistion */
                dma->tx_ring_head = curr_bd;

                /* Make hardware aware of the newly enabled descriptors */
                SPIN_IRQFLAGS(plev);

                SPIN_LOCK_IRQ(&priv->devlock, plev);

                ctrl = dma->regs->ctrl;
                ctrl &=
                    ~(GRSPW_DMACTRL_PS | GRSPW_DMACTRL_PR |
                      GRSPW_DMA_STATUS_ERROR);
                ctrl |= GRSPW_DMACTRL_TE;
                dma->regs->ctrl = ctrl;

                SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        }

        return cnt;
}

/* Schedule as many packets as possible (USER->SCHED) */
int grspw_dma_tx_send(struct grspw_dma_priv *dma, struct grspw_list *pkts)
{
        int nsched;

        if (dma->started == 0) {
                return -1;
        }

        /* Schedule as many packets as possible (USER->SCHED) */
        nsched = grspw_tx_schedule_send(dma, pkts);

        return nsched;
}

/* Scans the TX descriptor table for transmitted packets, and moves these
 * packets from the head of the scheduled queue to the tail of the user queue.
 *
 * Also, for all packets the status is updated.
 *
 *  - SCHED List -> USER List
 *
 * Return Value
 * Number of packet moved
 */
static int grspw_tx_process_scheduled(struct grspw_dma_priv *dma,
                                      struct grspw_list *user)
{
        struct grspw_txring *curr;
        struct grspw_pkt *last_pkt = NULL;
        int sent_pkt_cnt = 0;
        uint32_t ctrl;
        struct grspw_list lst;

        curr = dma->tx_ring_tail;

        /* Step into TX ring to find if packets have been scheduled for
         * transmission.
         */
        if (!curr->pkt) {
                return 0;       /* No scheduled packets, thus no sent, abort */
        }

        /* There has been Packets scheduled ==> scheduled Packets may have been
         * transmitted and needs to be collected into USER List.
         *
         * A temporary list "lst" with all sent packets is created.
         */
        lst.head = curr->pkt;

        /* Loop until first enabled "un-transmitted" SpW Packet is found.
         * An unused descriptor is indicated by an unassigned pkt field.
         */
        while (curr->pkt && !((ctrl = curr->bd->ctrl) & GRSPW_TXBD_EN)) {
                /* Handle one sent Packet */

                /* Remember last handled Packet so that insertion/removal from
                 * packet lists go fast.
                 */
                last_pkt = curr->pkt;

                /* Set flags to indicate error(s) and Mark Sent.
                 */
                last_pkt->flags =
                    (last_pkt->flags & ~TXPKT_FLAG_OUTPUT_MASK) |
                    (ctrl & TXPKT_FLAG_LINKERR) | TXPKT_FLAG_TX;

                /* Sent packet experienced link error? */
                if (ctrl & GRSPW_TXBD_LE) {
                        dma->stats.tx_err_link++;
                }

                curr->pkt = NULL;       /* Mark descriptor unused */

                /* Increment */
                curr = curr->next;
                sent_pkt_cnt++;
        }

        /* Remove all handled packets from TX-SCHED queue
         * Put all handled packets into USER queue
         */
        if (sent_pkt_cnt > 0) {
                /* Update Stats, Number of Transmitted Packets */
                dma->stats.tx_pkts += sent_pkt_cnt;

                /* Save TX ring posistion */
                dma->tx_ring_tail = curr;

                /* Prepare list for insertion/deleation */
                lst.tail = last_pkt;

                /* Remove sent packets from TX-SCHED queue */
                grspw_list_remove_head_list(&dma->tx_sched, &lst);
                dma->tx_sched_cnt -= sent_pkt_cnt;
                if (dma->stats.tx_sched_cnt_min > dma->tx_sched_cnt) {
                        dma->stats.tx_sched_cnt_min = dma->tx_sched_cnt;
                }

                /* Insert received packets into USER queue */
                grspw_list_append_list(user, &lst);
        }

        return sent_pkt_cnt;
}

int grspw_dma_tx_reclaim(struct grspw_dma_priv *dma, struct grspw_list *pkts)
{
        int nsent;

        if (dma->started == 0) {
                return -1;
        }
        /* Move transmitted packets to USER List (SCHED->USER) */
        nsent = grspw_tx_process_scheduled(dma, pkts);

        return nsent;
}

/* Must only be called in DMA channel stopped state. */
int grspw_dma_tx_flush(struct grspw_dma_priv *dma, struct grspw_list *pkts)
{
        int nsent;
        int nflush;

        if (dma->started) {
                return -1;
        }

        /* Move Scheduled packets to USER List (SCHED->USER) */
        nsent = grspw_tx_process_scheduled(dma, pkts);

        /* Move remaining (un-sent) packets in SCHED queue to the
         * USER Queue. These are never marked sent.
         */
        nflush = 0;
        if (!grspw_list_is_empty(&dma->tx_sched)) {
                grspw_list_append_list(pkts, &dma->tx_sched);
                grspw_list_clr(&dma->tx_sched);
                nflush = dma->tx_sched_cnt;
                dma->tx_sched_cnt = 0;
        }

        return nsent + nflush;
}

/* Try to populate descriptor ring with as many USER packets as possible. The
 * packets assigned with to a descriptor are put in the end of the scheduled
 * list.
 *
 * The number of Packets scheduled is returned.
 *
 *  - USER List -> RX-SCHED List
 *  - Descriptors are initialized and enabled for reception
 */
static int grspw_rx_schedule_ready(struct grspw_dma_priv *dma,
                                   struct grspw_list *user)
{
        int cnt;
        uint32_t ctrl;
        void *hwaddr;
        struct grspw_rxring *curr_bd;
        struct grspw_pkt *curr_pkt;
        struct grspw_pkt *last_pkt = NULL;
        struct grspw_list lst;

        /* Is Ready Q empty? */
        if (grspw_list_is_empty(user)) {
                return 0;
        }

        cnt = 0;
        lst.head = curr_pkt = user->head;

        curr_bd = dma->rx_ring_head;
        while (!curr_bd->pkt) {
                /* Assign Packet to descriptor */
                curr_bd->pkt = curr_pkt;

                /* Prepare descriptor address. */
                hwaddr = curr_pkt->data;
                curr_bd->bd->addr = (uint32_t)hwaddr;

                ctrl = GRSPW_RXBD_EN;
                if (curr_bd->next == dma->rx_ring_base) {
                        /* Wrap around (only needed when smaller descriptor
                         * table)
                         */
                        ctrl |= GRSPW_RXBD_WR;
                }

                if (curr_pkt->flags & RXPKT_FLAG_IE) {
                        /* User has requested interrupt at reception. */
                        ctrl |= GRSPW_RXBD_IE;
                }

                /* Enable descriptor */
                curr_bd->bd->ctrl = ctrl;

                last_pkt = curr_pkt;
                curr_bd = curr_bd->next;
                cnt++;

                /* Get Next Packet from Ready Queue */
                if (curr_pkt == user->tail) {
                        /* Handled all in ready queue. */
                        curr_pkt = NULL;
                        break;
                }
                curr_pkt = curr_pkt->next;
        }

        /* Has Packets been scheduled? */
        if (cnt > 0) {
                /* Prepare list for insertion/deleation */
                lst.tail = last_pkt;

                /* Remove scheduled packets from user queue */
                grspw_list_remove_head_list(user, &lst);

                /* Insert scheduled packets into scheduled queue */
                grspw_list_append_list(&dma->rx_sched, &lst);
                dma->rx_sched_cnt += cnt;
                if (dma->stats.rx_sched_cnt_max < dma->rx_sched_cnt) {
                        dma->stats.rx_sched_cnt_max = dma->rx_sched_cnt;
                }

                /* Update RX ring posistion */
                dma->rx_ring_head = curr_bd;

                /* Make hardware aware of the newly enabled descriptors */
                SPIN_IRQFLAGS(plev);

                SPIN_LOCK_IRQ(&priv->devlock, plev);

                ctrl = dma->regs->ctrl;
                ctrl &=
                    ~(GRSPW_DMACTRL_PS | GRSPW_DMACTRL_PR |
                      GRSPW_DMA_STATUS_ERROR);
                ctrl |= GRSPW_DMACTRL_RE | GRSPW_DMACTRL_RD;
                dma->regs->ctrl = ctrl;
                SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        }

        return cnt;
}

int grspw_dma_rx_prepare(struct grspw_dma_priv *dma, struct grspw_list *pkts)
{
        int nprep;

        if (dma->started == 0) {
                return -1;
        }

        /* Schedule as many packets as possible (USER->SCHED) */
        nprep = grspw_rx_schedule_ready(dma, pkts);

        return nprep;
}

/* Scans the RX desciptor table for scheduled packets that have been received,
 * and moves these packets from the head of the scheduled queue to the
 * tail of the user queue.
 *
 * Also, for all packets the status is updated.
 *
 *  - SCHED List -> USER List
 *
 * Return Value
 * Number of packets moved
 */
static int grspw_rx_process_scheduled(struct grspw_dma_priv *dma,
                                      struct grspw_list *user)
{
        struct grspw_rxring *curr;
        struct grspw_pkt *last_pkt = NULL;
        int recv_pkt_cnt = 0;
        uint32_t ctrl;
        struct grspw_list lst;

        curr = dma->rx_ring_tail;

        /* Step into RX ring to find if packets have been scheduled for
         * reception.
         */
        if (!curr->pkt) {
                /* No scheduled packets, thus no received, abort */
                return 0;
        }

        /* There has been Packets scheduled ==> scheduled Packets may have been
         * received and needs to be collected into USER List.
         *
         * A temporary list "lst" with all received packets is created.
         */
        lst.head = curr->pkt;

        /* Loop until first enabled "unrecveived" SpW Packet is found.
         * An unused descriptor is indicated by an unassigned pkt field.
         */
        while (curr->pkt && !((ctrl = curr->bd->ctrl) & GRSPW_RXBD_EN)) {
                /* Handle one received Packet */

                /* Remember last handled Packet so that insertion/removal from
                 * Packet lists go fast.
                 */
                last_pkt = curr->pkt;

                /* Get Length of Packet in bytes. */
                last_pkt->dlen = (ctrl & GRSPW_RXBD_LEN) >> GRSPW_RXBD_LEN_BIT;

                /* Set flags to indicate error(s) and CRC information,
                 * and Mark Received.
                 */
                last_pkt->flags =
                    (last_pkt->flags & ~RXPKT_FLAG_OUTPUT_MASK) |
                    ((ctrl >> 20) & RXPKT_FLAG_OUTPUT_MASK) | RXPKT_FLAG_RX;

                /* Packet was Truncated? */
                if (ctrl & GRSPW_RXBD_TR) {
                        dma->stats.rx_err_trunk++;
                }

                /* Error End-Of-Packet? */
                if (ctrl & GRSPW_RXBD_EP) {
                        dma->stats.rx_err_endpkt++;
                }
                curr->pkt = NULL;       /* Mark descriptor unused */

                /* Increment */
                curr = curr->next;
                recv_pkt_cnt++;
        }

        /* 1. Remove all handled packets from scheduled queue
         * 2. Put all handled packets into user queue
         */
        if (recv_pkt_cnt > 0) {
                /* Update Stats, Number of Received Packets */
                dma->stats.rx_pkts += recv_pkt_cnt;

                /* Save RX ring posistion */
                dma->rx_ring_tail = curr;

                /* Prepare list for insertion/deleation */
                lst.tail = last_pkt;

                /* Remove received Packets from RX-SCHED queue */
                grspw_list_remove_head_list(&dma->rx_sched, &lst);
                dma->rx_sched_cnt -= recv_pkt_cnt;
                if (dma->stats.rx_sched_cnt_min > dma->rx_sched_cnt) {
                        dma->stats.rx_sched_cnt_min = dma->rx_sched_cnt;
                }

                /* Insert received Packets into USER queue */
                grspw_list_append_list(user, &lst);
        }

        return recv_pkt_cnt;
}

int grspw_dma_rx_recv(struct grspw_dma_priv *dma, struct grspw_list *pkts)
{
        int nrecv;

        if (dma->started == 0) {
                return -1;
        }

        /* Move Scheduled packets to USER List (SCHED->USER) */
        nrecv = grspw_rx_process_scheduled(dma, pkts);

        return nrecv;
}

int grspw_dma_rx_flush(struct grspw_dma_priv *dma, struct grspw_list *pkts)
{
        int nrecv;
        int nflush;

        if (dma->started) {
                return -1;
        }

        /* Move Scheduled packets to USER List (SCHED->USER) */
        nrecv = grspw_rx_process_scheduled(dma, pkts);

        /* Move remaining (un-received) packets in SCHED queue to the
         * USER Queue. These are never marked received.
         */
        nflush = 0;
        if (!grspw_list_is_empty(&dma->rx_sched)) {
                grspw_list_append_list(pkts, &dma->rx_sched);
                grspw_list_clr(&dma->rx_sched);
                nflush = dma->rx_sched_cnt;
                dma->rx_sched_cnt = 0;
        }

        return nrecv + nflush;
}

