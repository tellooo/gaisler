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

#ifndef DRV_GRSPW_H
#define DRV_GRSPW_H

#include <stdint.h>
#include <stdlib.h>
#include <drv/regs/grspw.h>
#include <drv/auto.h>
#include <drv/osal.h>

/*** TX Packet flags ***/

/* Enable IRQ generation */
#define TXPKT_FLAG_IE 0x0040

/*
 * Enable Header CRC generation (if CRC is available in HW). Header CRC will be
 * appended (one byte at end of header)
 */
#define TXPKT_FLAG_HCRC 0x0100

/*
 * Enable Data CRC generation (if CRC is available in HW). Data CRC will be
 * appended (one byte at end of packet)
 */
#define TXPKT_FLAG_DCRC 0x0200

/*
 * Control how many bytes the beginning of the header the CRC should not be
 * calculated for.
 */
#define TXPKT_FLAG_NOCRC_MASK 0x0000000f
#define TXPKT_FLAG_NOCRC_LEN0 0x00000000
#define TXPKT_FLAG_NOCRC_LEN1 0x00000001
#define TXPKT_FLAG_NOCRC_LEN2 0x00000002
#define TXPKT_FLAG_NOCRC_LEN3 0x00000003
#define TXPKT_FLAG_NOCRC_LEN4 0x00000004
#define TXPKT_FLAG_NOCRC_LEN5 0x00000005
#define TXPKT_FLAG_NOCRC_LEN6 0x00000006
#define TXPKT_FLAG_NOCRC_LEN7 0x00000007
#define TXPKT_FLAG_NOCRC_LEN8 0x00000008
#define TXPKT_FLAG_NOCRC_LEN9 0x00000009
#define TXPKT_FLAG_NOCRC_LENa 0x0000000a
#define TXPKT_FLAG_NOCRC_LENb 0x0000000b
#define TXPKT_FLAG_NOCRC_LENc 0x0000000c
#define TXPKT_FLAG_NOCRC_LENd 0x0000000d
#define TXPKT_FLAG_NOCRC_LENe 0x0000000e
#define TXPKT_FLAG_NOCRC_LENf 0x0000000f

#define TXPKT_FLAG_INPUT_MASK ( \
        TXPKT_FLAG_NOCRC_MASK | \
        TXPKT_FLAG_IE | \
        TXPKT_FLAG_HCRC | \
        TXPKT_FLAG_DCRC \
)

/* Marks if packet was transmitted or not */
#define TXPKT_FLAG_TX 0x4000

/* Link Error */
#define TXPKT_FLAG_LINKERR 0x8000

#define TXPKT_FLAG_OUTPUT_MASK (TXPKT_FLAG_LINKERR)

/* GRSPW Link configuration options */
#define GRSPW_LINK_CFG ( \
        GRSPW_CTRL_LI | \
        GRSPW_CTRL_AS | \
        GRSPW_CTRL_LS | \
        GRSPW_CTRL_LD \
)

/*** RX Packet Flags ***/

/* Enable IRQ generation */
#define RXPKT_FLAG_IE 0x0010

#define RXPKT_FLAG_INPUT_MASK (RXPKT_FLAG_IE)

/* Packet was truncated */
#define RXPKT_FLAG_TRUNK 0x0800
/* Data CRC error (only valid if RMAP CRC is enabled) */
#define RXPKT_FLAG_DCRC 0x0400
/* Header CRC error (only valid if RMAP CRC is enabled) */
#define RXPKT_FLAG_HCRC 0x0200
/* Error in End-of-Packet */
#define RXPKT_FLAG_EEOP 0x0100
/* Marks if packet was recevied or not */
#define RXPKT_FLAG_RX 0x8000

#define RXPKT_FLAG_OUTPUT_MASK ( \
        RXPKT_FLAG_TRUNK | \
        RXPKT_FLAG_DCRC | \
        RXPKT_FLAG_HCRC | \
        RXPKT_FLAG_EEOP \
)

/* GRSPW control register interrupt source bits (requiring interrupt enable). */
#define GRSPW_CTRL_IRQSRC_MASK  (GRSPW_CTRL_LI | GRSPW_CTRL_TQ)

/* Software Defaults */
#define DEFAULT_RXMAX 4096      /* 4 KBytes Max RX Packet Size */
#define DEFAULT_DMAFLAGS 0      /* Spill (no spill disabled) */

/*
 * GRSPW RX/TX Packet structure.
 *
 * - For RX the 'hdr' and 'hlen' fields are not used, they are not written
 *   by driver.
 *
 * - The last packet in a list must have 'next' set to NULL.
 *
 * - data and hdr pointers are written without modification to hardware,
 *   this means that caller must do address translation to hardware
 *   address itself.
 *
 * - the 'flags' field are interpreted differently depending on transfer
 *   type (RX/TX). See XXPKT_FLAG_* options above.
 */
struct grspw_pkt {
        struct grspw_pkt *next;
        uint32_t pkt_id;        /* User assigned ID (not touched by driver) */
        void *data;             /* 4-byte or byte aligned depends on HW */
        void *hdr;              /* 4-byte or byte aligned depends on HW */
        uint32_t dlen;          /* Length of Data Buffer */
        uint16_t flags;         /* RX/TX Options */
        uint8_t hlen;           /* Length of Header Buffer */
};

/* GRSPW SpaceWire Packet List */
struct grspw_list {
        struct grspw_pkt *head;
        struct grspw_pkt *tail;
};

/* GRSPW Constants */
#define GRSPW_TXBD_NR 64        /* Maximum number of TX Descriptors */
#define GRSPW_RXBD_NR 128       /* Maximum number of RX Descriptors */
#define BDTAB_SIZE 0x400        /* BD Table Size (RX or TX) */
#define BDTAB_ALIGN 0x400       /* BD Table Alignment Requirement */

/* An entry in the TX descriptor Ring */
struct grspw_txring {
        struct grspw_txring *next;      /* Next Descriptor */
        struct grspw_txbd *bd;  /* Descriptor Address */
        struct grspw_pkt *pkt;  /* Packet description associated.NULL if none */
};

/* An entry in the RX descriptor Ring */
struct grspw_rxring {
        struct grspw_rxring *next;      /* Next Descriptor */
        struct grspw_rxbd *bd;  /* Descriptor Address */
        struct grspw_pkt *pkt;  /* Packet description associated.NULL if none */
};

struct grspw_ring {
        struct grspw_ring *next;        /* Next Descriptor */
        union {
                struct grspw_txbd *tx;  /* Descriptor Address */
                struct grspw_rxbd *rx;  /* Descriptor Address */
        } bd;
        struct grspw_pkt *pkt;  /* Packet description associated.NULL if none */
};

/* SpaceWire Link State */
typedef enum {
        SPW_LS_ERRRST = 0,
        SPW_LS_ERRWAIT = 1,
        SPW_LS_READY = 2,
        SPW_LS_STARTED = 3,
        SPW_LS_CONNECTING = 4,
        SPW_LS_RUN = 5
} spw_link_state_t;

/* Address Configuration */
struct grspw_addr_config {
        /* Ignore address field and put all received packets to first
         * DMA channel.
         */
        int8_t promiscuous;

        /* Default Node Address and Mask */
        uint8_t def_addr;
        uint8_t def_mask;
        /* DMA Channel custom Node Address and Mask */
        struct {
                int8_t node_en; /* Enable Separate Addr */
                uint8_t node_addr;      /* Node address */
                uint8_t node_mask;      /* Node address mask */
        } dma_nacfg[4];
};

/* Hardware Support in GRSPW Core */
struct grspw_hw_sup {
        int8_t rmap;            /* If RMAP in HW is available */
        int8_t rmap_crc;        /* If RMAP CRC is available */
        int8_t rx_unalign;      /* RX unaligned (byte boundary) access allowed*/
        int8_t nports;          /* Number of Ports (1 or 2) */
        int8_t ndma_chans;      /* Number of DMA Channels (1..4) */
        int    hw_version;      /* GRSPW Hardware Version */
        int8_t irq;             /* SpW Distributed Interrupt available if 1 */
};

/* grspw_rmap_set_ctrl() options */
#define RMAPOPTS_EN_RMAP        0x10000
#define RMAPOPTS_EN_BUF         0x20000

#define RMAPOPTS_MASK (RMAPOPTS_EN_RMAP | RMAPOPTS_EN_BUF)

/* grspw_dma_config.flags options */
#define DMAFLAG_NO_SPILL        0x0001  /* See HW doc DMA-CTRL NS bit */
#define DMAFLAG_RESV1           0x0002  /* HAS NO EFFECT */
#define DMAFLAG_STRIP_ADR       0x0004  /* See HW doc DMA-CTRL SA bit */
#define DMAFLAG_STRIP_PID       0x0008  /* See HW doc DMA-CTRL SP bit */
#define DMAFLAG_RESV2           0x0010  /* HAS NO EFFECT */
#define DMAFLAG_MASK ( \
        DMAFLAG_NO_SPILL | \
        DMAFLAG_STRIP_ADR | \
        DMAFLAG_STRIP_PID \
)

struct grspw_dma_config {
        int flags;
        int rxmaxlen;           /* RX Max Packet Length */
};

/* Statistics per DMA channel */
struct grspw_dma_stats {
        /* IRQ Statistics */
        int irq_cnt;            /* Number of DMA IRQs generated by channel */
        /* Descriptor Statistics */
        int tx_pkts;            /* Number of Transmitted packets */
        int tx_err_link;        /* Number of Transmitted packets w Link Error */
        int rx_pkts;            /* Number of Received packets */
        int rx_err_trunk;       /* Number of Received Truncated packets */
        int rx_err_endpkt;      /* Number of Received packets with bad ending */

        int tx_sched_cnt_min;   /* Minimum number of packets in TX SCHED Q */
        int tx_sched_cnt_max;   /* Maximum number of packets in TX SCHED Q */

        int rx_sched_cnt_min;   /* Minimum number of packets in RX SCHED Q */
        int rx_sched_cnt_max;   /* Maximum number of packets in RX SCHED Q */
};

/* Masks for grspw_dma_get_status() / grspw_dma_clear_status(). */
#define GRSPW_DMA_STATUS_RA     0x0100  /* RX AHB Error */
#define GRSPW_DMA_STATUS_TA     0x0080  /* TX AHB Error */
#define GRSPW_DMA_STATUS_PR     0x0040  /* Packet received */
#define GRSPW_DMA_STATUS_PS     0x0020  /* Packet sent */
#define GRSPW_DMA_STATUS_MASK ( \
        GRSPW_DMA_STATUS_RA | \
        GRSPW_DMA_STATUS_TA | \
        GRSPW_DMA_STATUS_PR | \
        GRSPW_DMA_STATUS_PS \
)

/* GRSPW Error Condition */
#define GRSPW_DMA_STATUS_ERROR  (GRSPW_DMACTRL_RA | GRSPW_DMACTRL_TA)

/* grspw_get_linkcfg() / grspw_set_linkcfg() options */
#define LINKOPTS_ENABLE         0x0000
#define LINKOPTS_DISABLE        0x0001
#define LINKOPTS_START          0x0002
#define LINKOPTS_AUTOSTART      0x0004
#define LINKOPTS_ERRIRQ         0x0200  /* Enable Error Link IRQ */
#define LINKOPTS_MASK           0x0207  /* All above options */

/* grspw_get_tccfg() / grspw_set_tccfg() options */
#define TCOPTS_EN_RXIRQ         0x0100  /* Tick-Out IRQ */
#define TCOPTS_EN_TX            0x0400
#define TCOPTS_EN_RX            0x0800

/* grspw_get_tc() masks */
#define TCTRL_MASK 0xc0         /* Time control flags of time register */
#define TIMECNT_MASK 0x3f       /* Time counter of time register */

/* GRSPW time code control */
#define GRSPW_TC_CFG            (GRSPW_CTRL_TR | GRSPW_CTRL_TT | GRSPW_CTRL_TQ)

/** PRIVATE **/
struct grspw_userbuf;
/* Driver private structure. Shall never be referenced by user. */
struct grspw_dma_priv {
        volatile struct grspw_dma_regs *regs;
        int open;
        int started;
        struct grspw_dma_stats stats;   /* DMA Channel Statistics */
        struct grspw_dma_config cfg;    /* DMA Channel Configuration */
        struct grspw_userbuf *ubuf;

        /*** RX ***/

        /* RX Descriptor Ring */
        struct grspw_rxbd *rx_bds;      /* Descriptor Address */
        struct grspw_rxring *rx_ring_base;
        struct grspw_rxring *rx_ring_head;      /* Next descriptor to enable */
        struct grspw_rxring *rx_ring_tail;      /* Oldest enabled Descriptor */

        /* Scheduled RX Packets Queue */
        struct grspw_list rx_sched;
        int rx_sched_cnt;

        /*** TX ***/

        /* TX Descriptor Ring */
        struct grspw_txbd *tx_bds;      /* Descriptor Address */
        struct grspw_txring *tx_ring_base;
        struct grspw_txring *tx_ring_head;
        struct grspw_txring *tx_ring_tail;

        /* Scheduled TX Packets Queue */
        struct grspw_list tx_sched;
        int tx_sched_cnt;
};

/* Driver private structure. Shall never be referenced by user. */
struct grspw_priv {
        volatile struct grspw_regs *regs;
        uint8_t open;
        SPIN_DECLARE(devlock)

        int irqsource;
        void (*userisr)(struct grspw_priv *priv, void *data);
        void *userisr_data;
        struct osal_isr_ctx isr_ctx;

        /* Features supported by Hardware */
        struct grspw_hw_sup hwsup;
        struct grspw_dma_priv dma[4];
};

/** END PRIVATE **/

struct grspw_devcfg {
        struct drv_devreg regs;
        struct grspw_priv priv;
};

/*** Device interface ***/

struct grspw_devcfg;
struct grspw_priv;

/* return: number of devices registered */
int grspw_autoinit(void);
/* Register one device */
int grspw_register(struct grspw_devcfg *devcfg);
/* Register an array of devices */
int grspw_init(struct grspw_devcfg *devcfgs[]);

/** Opening and closing device **/

/* Retrieve number of GRSPW devices registered to the driver */
int grspw_dev_count(void);

/* Open a GRSPW device */
struct grspw_priv *grspw_open(int dev_no);

/* Close a GRSPW device */
int grspw_close(struct grspw_priv *priv);

/* Get GRSPW hardware capabilities */
void grspw_hw_support(struct grspw_priv *priv, struct grspw_hw_sup *hw);

/** Link control **/

/* Get link configuration */
uint32_t grspw_get_linkcfg(struct grspw_priv *priv);

/* Set link configuration */
int grspw_set_linkcfg(struct grspw_priv *priv, uint32_t cfg);

/* Get clock divisor */
uint32_t grspw_get_clkdiv(struct grspw_priv *priv);

/* Set clock divisor */
int grspw_set_clkdiv(struct grspw_priv *priv, uint32_t clkdiv);

/* Get current SpaceWire link state */
spw_link_state_t grspw_link_state(struct grspw_priv *priv);

/* Get status register value */
uint32_t grspw_get_status(struct grspw_priv *priv);

/* Clear bits in the status register */
void grspw_clear_status(struct grspw_priv *priv, uint32_t status);

/** Node address configuration **/

/* Set node address configuration */
void grspw_addr_ctrl(
        struct grspw_priv *priv,
        const struct grspw_addr_config *cfg
);

/** Time-control codes **/

/* Get time-code configuration */
uint32_t grspw_get_tccfg(struct grspw_priv *priv);

/* Set time-code configuration */
void grspw_set_tccfg(struct grspw_priv *priv, uint32_t cfg);

/* Read TCTRL and TIMECNT.
 * TCTRL   = bits 7 and 6
 * TIMECNT = bits 5 to 0
 */

/* Get time register value */
uint32_t grspw_get_tc(struct grspw_priv *priv);

/** Interrupt Service Routine **/

/* Set user Interrupt Service Routine (ISR) */
void grspw_set_isr(
        struct grspw_priv *priv,
        void (*isr)(struct grspw_priv *priv, void *data),
        void *data
);

/** RMAP **/

/* Set RMAP configuration */
int grspw_rmap_set_ctrl(struct grspw_priv *priv, uint32_t options);
uint32_t grspw_rmap_get_ctrl(struct grspw_priv *priv);

/* Set RMAP destination key */
int grspw_rmap_set_destkey(struct grspw_priv *priv, uint32_t destkey);
uint32_t grspw_rmap_get_destkey(struct grspw_priv *priv);

/*** SpW Port Control Interface ***/

/* Select port, if
 * -1=The current selected port is returned
 * 0=Port 0
 * 1=Port 1
 * Other positive values=Both Port0 and Port1
 */
int grspw_port_ctrl(struct grspw_priv *priv, int *port);
/* Returns Number ports available in hardware */
int grspw_port_count(struct grspw_priv *priv);
/* Returns the current active port */
int grspw_port_active(struct grspw_priv *priv);

/*** DMA Interface ***/

/* Retrieve number of DMA channels available at GRSPW device */
int grspw_dma_count(struct grspw_priv *priv);

/* Open a GRSPW DMA channel */
/* Note: rx_bds and tx_bds must be aligned to 1 KiB */
/* Note: must be closed with grspw_dma_close_userbuf */
struct grspw_dma_priv *grspw_dma_open_userbuf(
        struct grspw_priv *priv,
        int chan_no,
        struct grspw_ring *rx_ring,
        struct grspw_ring *tx_ring,
        struct grspw_rxbd *rx_bds,
        struct grspw_txbd *tx_bds
);

/* Close a GRSPW DMA channel */
int grspw_dma_close_userbuf(struct grspw_dma_priv *priv);

struct grspw_dma_priv *grspw_dma_open(
        struct grspw_priv *priv,
        int chan_no
);

/* Close a GRSPW DMA channel */
int grspw_dma_close(struct grspw_dma_priv *priv);

/* Start a DMA channel */
int grspw_dma_start(struct grspw_dma_priv *priv);

/* Stop a DMA channel */
/*
 * NOTE: The user may want to flush the RX/TX SCHED queues with
 * grspw_dma_tx_flush and grspw_dma_rx_flush after stopping to get unprocessed
 * packets back.
 */
void grspw_dma_stop(struct grspw_dma_priv *priv);

/* Schedule List of packets for transmission at some point in
 * future.
 *
 * Add the requested packets to the SCHED List (USER->SCHED)
 *
 * NOTE: the TXPKT_FLAG_TX flag must not be set.
 *
 * Return Code
 *  <0   Error
 *  0    No pkt added (sched list full)
 *  >0   Number of pkts successfully added to sched list
 */
int grspw_dma_tx_send(struct grspw_dma_priv *priv, struct grspw_list *pkts);

/* Reclaim TX packet buffers that has previously been scheduled for transmission
 * with grspw_dma_tx_send(). The user list is not cleared.
 *
 * Move transmitted packets to SENT List (SCHED->USER)
 *
 * Return Code
 *  <0   Error
 *  0    No pkt reclaimed (sched list contains no sent pkts)
 *  >0   Number of pkts successfully reclaimed to user list
 */
int grspw_dma_tx_reclaim(struct grspw_dma_priv *priv, struct grspw_list *pkts);

/*
 * Like grspw_dma_tx_reclaim, but also move scheduled unsent packets to user
 * list. Can only be called in stopped state. Returns sum of sent packets
 * and unsent packets. The TXPKT_FLAG_TX packet flag indicates if the
 * packet was sent or not.
 *
 * Return Code
 *  <0   Error
 *  =>0  Number of sent and unsent packets added to user list
 */
int grspw_dma_tx_flush(struct grspw_dma_priv *priv, struct grspw_list *pkts);

/*
 * Count number of transmitted packets not yet reclaimed.
 *
 * This number indicates how many packets could be reclaimed with function
 * grspw_dma_tx_reclaim.
 *
 * Return Code
 *  Number of sent packets not yet reclaimed
 */
int grspw_dma_tx_count(struct grspw_dma_priv *priv);

/* Add more RX packet buffers for future for reception. The received packets
 * can later be read out with grspw_dma_rx_recv().
 *
 * Schedule as many packets as possible (USER->SCHED)
 *
 * Return Code
 *  <0   Error
 *  0    No pkt added (sched list full)
 *  >0   Number of pkts successfully added to sched queue
 */
int grspw_dma_rx_prepare(struct grspw_dma_priv *priv, struct grspw_list *pkts);

/* Get received RX packet buffers that has previously been scheduled for
 * reception with grspw_dma_rx_prepare().
 *
 * Move Scheduled packets to USER List (SCHED->USER)
 *
 * Return Code
 *  <0   Error
 *  0    No pkt received (sched list contains no recv pkts)
 *  >0   Number of received pkts successfully added to user list
 */
int grspw_dma_rx_recv(struct grspw_dma_priv *priv, struct grspw_list *pkts);

/*
 * Like grspw_dma_rx_recv, but also move scheduled unreceived packets to user
 * list. Can only be called in stopped state. Returns sum of recevied packets
 * and unreceived packets. The RXPKT_FLAG_RX packet flag indicates if the
 * packet was received or not.
 *
 * Return Code
 *  <0   Error
 *  =>0  Number of recevied and unrecevied packets added to user list
 */
int grspw_dma_rx_flush(struct grspw_dma_priv *priv, struct grspw_list *pkts);

/*
 * Count number of received packets not yet retrieved by driver.
 *
 * This number indicates how many packets could be retrieved with function
 * grspw_dma_rx_recv.
 *
 * Return Code
 *  Number of received packets not yet retrieved by driver
 */
int grspw_dma_rx_count(struct grspw_dma_priv *priv);

/* Reads the DMA channel statistics */
void grspw_dma_stats_read(
        struct grspw_dma_priv *priv,
        struct grspw_dma_stats *stats
);

/* Clear DMA channel statistics */
void grspw_dma_stats_clr(struct grspw_dma_priv *priv);

/* Set DMA channel configuration options */
int grspw_dma_config(
        struct grspw_dma_priv *priv,
        struct grspw_dma_config *cfg
);

/* Read DMA channel configuration options */
int grspw_dma_config_read(
        struct grspw_dma_priv *priv,
        struct grspw_dma_config *cfg
);

/* Get DMA channel status flags. */
uint32_t grspw_dma_get_status(struct grspw_dma_priv *priv);

/* Clear DMA channel status flags. */
void grspw_dma_clear_status(struct grspw_dma_priv *priv, uint32_t status);

/*** GRSPW SpaceWire Packet List Handling Routines ***/

/* Initialize a packet queue */
static inline void grspw_list_clr(struct grspw_list *list)
{
        list->head = NULL;
        list->tail = NULL;
}

/* Determine if a queue is empty */
static inline int grspw_list_is_empty(struct grspw_list *list)
{
        return (list->head == NULL);
}

/* Append packet to end of queue */
static inline void grspw_list_append(
        struct grspw_list *list,
        struct grspw_pkt *pkt
)
{
        pkt->next = NULL;
        if (list->tail == NULL) {
                list->head = pkt;
        } else {
                list->tail->next = pkt;
        }
        list->tail = pkt;
}

/*
 * Append packets from one queue to the end of another queue
 *
 * - Packets in queue "alist" are appended to the "list" queue.
 * - No packets are unlinked from the "alist" queue.
 * - Queue "alist" must not be empty when the function is called.
 * - Queue "list" may be empty when function is called.
 */
static inline void grspw_list_append_list(
        struct grspw_list *list,
        struct grspw_list *alist
)
{
        alist->tail->next = NULL;
        if (list->tail == NULL) {
                list->head = alist->head;
        } else {
                list->tail->next = alist->head;
        }
        list->tail = alist->tail;
}

/*
 * Remove multiple packets from a queue
 *
 * Remove dlist (delete-list) from head of list. The two lists typically have
 * common elements from head down to some point (dlist->tail).
 * NOTE: dlist must not be empty.
 */
static inline void grspw_list_remove_head_list(
        struct grspw_list *list,
        struct grspw_list *dlist
)
{
        list->head = dlist->tail->next;
        if (list->head == NULL) {
                list->tail = NULL;
        }
        dlist->tail->next = NULL;
}

/* Remove head packet from a queue */
static inline struct grspw_pkt *grspw_list_remove_head(struct grspw_list *list)
{
        struct grspw_pkt *head;

        if (grspw_list_is_empty(list)) {
                return NULL;
        }
        head = list->head;
        list->head = head->next;
        if (list->head == NULL) {
                list->tail = NULL;
        }
        return head;
}

/* Perform hardware operations to stop a DMA channel operational mode */
void grspw_hw_dma_stop(struct grspw_dma_priv *dma);

void grspw_hw_dma_softreset(struct grspw_dma_priv *dma);
void grspw_isr(void *arg);

/* Return nonzero iff some supported irq source is set */
static inline uint32_t grspw_is_irqsource_set(uint32_t ctrl)
{
        return ctrl & GRSPW_CTRL_IRQSRC_MASK;
}

#endif

