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

#ifndef DRV_GRCAN_H
#define DRV_GRCAN_H

#include <drv/auto.h>
#include <drv/osal.h>
struct grcan_devcfg;
struct grcan_priv;

/* return: number of devices registered */
int grcan_autoinit(void);
/* Register one device */
int grcan_register(struct grcan_devcfg *devcfg);
/* Register an array of devices */
int grcan_init(struct grcan_devcfg *devcfgs[]);

struct grcan_regs {
        volatile unsigned int conf;          /* 0x00 */
        volatile unsigned int stat;          /* 0x04 */
        volatile unsigned int ctrl;          /* 0x08 */
        volatile unsigned int dummy0[3];     /* 0x0C-0x014 */
        volatile unsigned int smask;         /* 0x18 */
        volatile unsigned int scode;         /* 0x1C */

        volatile unsigned int dummy1[56];    /* 0x20-0xFC */

        volatile unsigned int pimsr;         /* 0x100 */
        volatile unsigned int pimr;          /* 0x104 */
        volatile unsigned int pisr;          /* 0x108 */
        volatile unsigned int pir;           /* 0x10C */
        volatile unsigned int imr;           /* 0x110 */
        volatile unsigned int picr;          /* 0x114 */

        volatile unsigned int dummy2[58];    /* 0x118-0x1FC */

        volatile unsigned int tx0ctrl;       /* 0x200 */
        volatile unsigned int tx0addr;       /* 0x204 */
        volatile unsigned int tx0size;       /* 0x208 */
        volatile unsigned int tx0wr;         /* 0x20C */
        volatile unsigned int tx0rd;         /* 0x210 */
        volatile unsigned int tx0irq;        /* 0x214 */

        volatile unsigned int dummy3[58];    /* 0x218-0x2FC */

        volatile unsigned int rx0ctrl;       /* 0x300 */
        volatile unsigned int rx0addr;       /* 0x304 */
        volatile unsigned int rx0size;       /* 0x308 */
        volatile unsigned int rx0wr;         /* 0x30C */
        volatile unsigned int rx0rd;         /* 0x310 */
        volatile unsigned int rx0irq;        /* 0x314 */
        volatile unsigned int rx0mask;       /* 0x318 */
        volatile unsigned int rx0code;       /* 0x31C */
};

struct grcan_stats {
        unsigned int passive_cnt;
        unsigned int overrun_cnt;
        unsigned int rxsync_cnt;
        unsigned int txsync_cnt;
        unsigned int txloss_cnt;
        unsigned int ahberr_cnt;
        unsigned int ints;
        unsigned int busoff_cnt;
};

struct grcan_timing {
        unsigned char scaler;
        unsigned char ps1;
        unsigned char ps2;
        unsigned int  rsj;
        unsigned char bpr;
};

struct grcan_selection {
        int selection;
        int enable0;
        int enable1;
};

struct grcan_filter {
        unsigned long long mask;
        unsigned long long code;
};

/* CAN MESSAGE */
struct grcan_canmsg {
        char extended; /* 1= Extended Frame (29-bit id), 0= STD Frame (11-bit id) */
        char rtr; /* RTR - Remote Transmission Request */
        char unused; /* unused */
        unsigned char len;
        unsigned char data[8];
        unsigned int id;
};

enum {
        GRCAN_RET_OK            =  0,
        GRCAN_RET_INVARG        = -1,
        GRCAN_RET_NOTSTARTED    = -2,
        GRCAN_RET_TIMEOUT       = -3,
        /* Bus-off condition detected (request aborted by driver) */
        GRCAN_RET_BUSOFF        = -4,
        /* AHB error condition detected (request aborted by driver) */
        GRCAN_RET_AHBERR        = -5,
        /* Invalid state for operation */
        GRCAN_RET_INVSTATE      = -6,
};

/*
 * User functions can cause these transitions:
 *   STATE_STOPPED -> STATE_STARTED
 *   STATE_STARTED -> STATE_STOPPED
 *   STATE_BUSOFF  -> STATE_STOPPED
 *   STATE_AHBERR  -> STATE_STOPPED
 *
 * ISR can cause these transition
 *   STATE_STARTED -> STATE_BUSOFF
 *   STATE_STARTED -> STATE_AHBERR
 *
 * STATE_BUSOFF is entered from ISR on bus-off condition. STATE_AHBERR is
 * entered from ISR on AHB DMA errors on RX/TX operations. At transition the ISR
 * disables DMA, masks all interrupts and releases semaphores.
 */
enum grcan_state {
        STATE_STOPPED           = 0,
        STATE_STARTED           = 1,
        STATE_BUSOFF            = 2,
        STATE_AHBERR            = 3,
};

#define GRCAN_CFG_ABORT      0x00000001
#define GRCAN_CFG_ENABLE0    0x00000002
#define GRCAN_CFG_ENABLE1    0x00000004
#define GRCAN_CFG_SELECTION  0x00000008
#define GRCAN_CFG_SILENT     0x00000010
#define GRCAN_CFG_BPR        0x00000300
#define GRCAN_CFG_RSJ        0x00007000
#define GRCAN_CFG_PS1        0x00f00000
#define GRCAN_CFG_PS2        0x000f0000
#define GRCAN_CFG_SCALER     0xff000000

#define GRCAN_CFG_BPR_BIT    8
#define GRCAN_CFG_RSJ_BIT    12
#define GRCAN_CFG_PS1_BIT    20
#define GRCAN_CFG_PS2_BIT    16
#define GRCAN_CFG_SCALER_BIT 24

#define GRCAN_CTRL_RESET  0x2
#define GRCAN_CTRL_ENABLE 0x1

#define GRCAN_TXCTRL_ENABLE 1
#define GRCAN_TXCTRL_ONGOING 1

#define GRCAN_RXCTRL_ENABLE 1
#define GRCAN_RXCTRL_ONGOING 1

/* Relative offset of IRQ sources to AMBA Plug&Play */
#define GRCAN_IRQ_IRQ 0
#define GRCAN_IRQ_TXSYNC 1
#define GRCAN_IRQ_RXSYNC 2

#define GRCAN_ERR_IRQ        0x1
#define GRCAN_OFF_IRQ        0x2
#define GRCAN_OR_IRQ         0x4
#define GRCAN_RXAHBERR_IRQ   0x8
#define GRCAN_TXAHBERR_IRQ   0x10
#define GRCAN_RXIRQ_IRQ      0x20
#define GRCAN_TXIRQ_IRQ      0x40
#define GRCAN_RXFULL_IRQ     0x80
#define GRCAN_TXEMPTY_IRQ    0x100
#define GRCAN_RX_IRQ         0x200
#define GRCAN_TX_IRQ         0x400
#define GRCAN_RXSYNC_IRQ     0x800
#define GRCAN_TXSYNC_IRQ     0x1000
#define GRCAN_RXERR_IRQ      0x2000
#define GRCAN_TXERR_IRQ      0x4000
#define GRCAN_RXMISS_IRQ     0x8000
#define GRCAN_TXLOSS_IRQ     0x10000

#define GRCAN_STAT_PASS      0x1
#define GRCAN_STAT_OFF       0x2
#define GRCAN_STAT_OR        0x4
#define GRCAN_STAT_AHBERR    0x8
#define GRCAN_STAT_ACTIVE    0x10
#define GRCAN_STAT_RXERRCNT  0xff00
#define GRCAN_STAT_TXERRCNT  0xff0000

/*
 * Return number of GRCAN devices available to driver
 */
extern int grcan_dev_count(void);

/*
 * Open a GRCAN device
 *
 * dev_no:      Device number to open
 * return:      Device handle to use with all other grcan_ API functions. The
 *              function returns NULL if device can not be opened.
 */
extern struct grcan_priv *grcan_open(int dev_no);

/*
 * Close a GRCAN device
 *
 * return: This function always returns 0 (success)
 */
extern int grcan_close(struct grcan_priv *priv);

/*
 * Open a GRCAN device with user supplied buffers
 *
 * Same as grcan_open(), but the user provides DMA buffers. Must be closed with
 * grcan_close_userbuf(). No dynamic memory allocation (malloc()) is done.
 */
extern struct grcan_priv *grcan_open_userbuf(
        int dev_no,
        void *rxbuf,
        int rxbuf_size,
        void *txbuf,
        int txbuf_size
);

/*
 * Close a GRCAN device
 *
 * return: This function always returns 0 (success)
 */
extern int grcan_close_userbuf(struct grcan_priv *priv);

/*
 * Receive CAN messages
 *
 * Multiple CAN messages can be received in one call.
 *
 * d: Device handle
 * msg: Pointer to receive messages
 * count: Number of CAN messages to receive
 *
 * return:
 *   >=0:                       Number of CAN messages received. This can be
 *                              less than the count parameter.
 *   GRCAN_RET_INVARG:          count parameter less than one or NULL msg.
 *   GRCAN_RET_NOTSTARTED:      Device not in started mode
 *   GRCAN_RET_BUSOFF:          A read was interrupted by a bus-off error.
 *                              Device has left started mode.
 *   GRCAN_RET_AHBERR:          Similar to BUSOFF, but was caused by AHB Error.
 */
extern int grcan_read(
        struct grcan_priv *priv,
        struct grcan_canmsg *msg,
        size_t count
);

/*
 * Transmit CAN messages
 *
 * Multiple CAN messages can be transmit in one call.
 *
 * d: Device handle
 * msg: Pointer to messages to transmit
 * count: Number of CAN messages to transmit
 *
 * return:
 *   >=0:                       Number of CAN messages transmitted. This can be
 *                              less than the count parameter.
 *   GRCAN_RET_INVARG:          count parameter less than one.
 *   GRCAN_RET_NOTSTARTED:      Device not in started mode
 *   GRCAN_RET_BUSOFF:          A write was interrupted by a Bus-off error.
 *                              Device has left started mode
 *   GRCAN_RET_AHBERR:          Similar to BUSOFF, but was caused by AHB Error.
 */
extern int grcan_write(
        struct grcan_priv *priv,
        struct grcan_canmsg *msg,
        size_t count
);

/*
 * Returns current GRCAN software state
 *
 * If STATE_BUSOFF or STATE_AHBERR is returned then the function grcan_stop()
 * shall be called before continue using the driver.
 *
 * d: Device handle
 * return:
 *   STATE_STOPPED              Stopped
 *   STATE_STARTED              Started
 *   STATE_BUSOFF               Bus-off has been detected
 *   STATE_AHBERR               AHB error has been detected
 */
extern int grcan_get_state(struct grcan_priv *priv);

/* The remaining functions return 0 on success and non-zero on failure. */

/* Functions controlling operational
 * mode
 */
/* Bring the link up after open or bus-off */
extern int grcan_start(struct grcan_priv *priv);
/* stop to change baud rate/config or closing down */
extern int grcan_stop(struct grcan_priv *priv);
/* Return 1 iff all TX messages have been sent */
extern int grcan_istxdone(struct grcan_priv *priv);

/* Functions that require connection
 * to be stopped
 */
/* enable silent mode read only state */
extern int grcan_set_silent(struct grcan_priv *priv, int silent);
/* enable/disable stopping link on AHB Error */
extern int grcan_set_abort(struct grcan_priv *priv, int abort);
/* Set Enable0,Enable1,Selection */
extern int grcan_set_selection(
        struct grcan_priv *priv,
        const struct grcan_selection *selection
);
/* Set baudrate by using driver's baud rate timing calculation routines */
extern int grcan_set_speed(struct grcan_priv *priv, unsigned int hz);
/* Set baudrate by specifying the timing registers manually */
extern int grcan_set_btrs(struct grcan_priv *priv, const struct grcan_timing *timing);

/* Functions can be called whenever */
/* Get statistics */
extern int grcan_get_stats(struct grcan_priv *priv, struct grcan_stats *stats);
/* Clear statistics */
extern int grcan_clr_stats(struct grcan_priv *priv);
/* Set Acceptance filters, provide pointer to "struct grcan_filter" or NULL to disable filtering (let all messages pass) */
extern int grcan_set_afilter(struct grcan_priv *priv, const struct grcan_filter *filter);
/* Set Sync Messages RX/TX filters, NULL disables the IRQ completely */
extern int grcan_set_sfilter(struct grcan_priv *priv, const struct grcan_filter *filter);
/* Get status register of GRCAN core */
extern int grcan_get_status(struct grcan_priv *priv, unsigned int *status);

/*
 * Generate TX interrupt
 *
 * n:
 *    0: never
 *    1: every message
 *   -1: when all messages have been transmitted
 */
int grcan_txint(struct grcan_priv *priv, int n);

/*
 * Generate RX interrupt
 *
 * n:
 *    0: never
 *    1: every message
 *   -1: when RX buffer is full
 */
int grcan_rxint(struct grcan_priv *priv, int n);

/* Set user Interrupt Service Routine (ISR) */
void grcan_set_isr(
        struct grcan_priv *priv,
        void (*isr)(struct grcan_priv *priv, void *data),
        void *data
);

/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct grcan_config {
        struct grcan_timing timing;
        struct grcan_selection selection;
        int abort;
        int silent;
};

struct grcan_priv {
        struct drvmgr_dev *dev; /* Driver manager device */
        unsigned int baseaddr, ram_base;
        struct grcan_regs *regs;
        int irq;
        int minor;
        uint8_t open;
        int started;
        unsigned int channel;
        unsigned int corefreq_hz;

        /* Circular DMA buffers */
        void *rxa;
        void *txa;
        struct grcan_msg *rx;
        struct grcan_msg *tx;
        unsigned int rxbuf_size;    /* requested RX buf size in bytes */
        unsigned int txbuf_size;    /* requested TX buf size in bytes */

        struct grcan_filter sfilter;
        struct grcan_filter afilter;
        int config_changed; /* 0=no changes, 1=changes ==> a Core reset is needed */
        struct grcan_config config;
        struct grcan_stats stats;

        void (*userisr)(struct grcan_priv *priv, void *data);
        void *userisr_data;
        struct osal_isr_ctx isr_ctx;
        SPIN_DECLARE(devlock)
};

struct grcan_devcfg {
        struct drv_devreg regs;
        struct grcan_priv priv;
};

#endif

