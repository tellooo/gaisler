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

#ifndef DRV_I2CMST_H
#define DRV_I2CMST_H

#include <stdint.h>
#include <drv/regs/i2cmst.h>
#include <drv/auto.h>
#include <drv/osal.h>

struct i2cmst_devcfg;
struct i2cmst_priv;

/* return: number of devices registered */
int i2cmst_autoinit(void);
/* Register one device */
int i2cmst_register(struct i2cmst_devcfg *devcfg);
/* Register an array of devices */
int i2cmst_init(struct i2cmst_devcfg *devcfgs[]);

int i2cmst_dev_count(void);

const struct drv_devreg *i2cmst_get_devreg(int dev_no);

enum {
        I2CMST_SPEED_STD   = 100000,        /* Standard speed (100 kbit/s) */
        I2CMST_SPEED_FAST  = 400000,        /* Fast speed (400 kbit/s) */
};

struct i2cmst_regs_cfg {
        uint32_t addr;
        int      interrupt;
};


#define I2CMST_FLAGS_READ       (1<<0)  /* Read or write transfer */
#define I2CMST_FLAGS_FINISHED   (1<<1)  /* Transfer has finished */
#define I2CMST_FLAGS_ERR        (1<<2)  /* Error occurred in transfer */
#define I2CMST_FLAGS_ADDR       (1<<3)  /* Send remote address */
#define I2CMST_FLAGS_RETRIED    (1<<4)  /* At least one retry at transfer */

/* The I2CMST software representation of a Frame */
struct i2cmst_packet {
        struct i2cmst_packet    *next;    /* Next packet in chain */
        /* Options and status */
        uint32_t                flags;    /* bypass options, and sent/error status */
        int                     slave;    /* Slave address */
        uint32_t                addr;     /* Use with I2CMST_FLAGS_ADDR */
        int                     length;   /* Size of payload */
        uint8_t                 *payload; /* Data to send */
};

struct i2cmst_list {
        struct i2cmst_packet    *head;    /* First Frame in list */
        struct i2cmst_packet    *tail;    /* Last Frame in list */
};

struct i2cmst_stats {
        uint32_t packets_sent;
        uint32_t arbitration_lost;
        uint32_t packets_nack;
};

/* Driver operation controlling commands */
struct i2cmst_priv *i2cmst_open(int dev_no);
int i2cmst_close(struct i2cmst_priv *priv);
int i2cmst_start(struct i2cmst_priv *priv);
int i2cmst_stop(struct i2cmst_priv *priv);
int i2cmst_isstarted(struct i2cmst_priv *priv);

/* Available only in RUNNING mode */
int i2cmst_request(struct i2cmst_priv *priv, struct i2cmst_list *chain);

/* Available only in STOPPED mode */
int i2cmst_set_retries(struct i2cmst_priv *priv, int retries);
int i2cmst_set_speed(struct i2cmst_priv *priv, int speed);
int i2cmst_set_interrupt_mode(struct i2cmst_priv *priv, int enable);
int i2cmst_set_ten_bit_addr(struct i2cmst_priv *priv, int enable);

/* Available in both running and stopped mode */
int i2cmst_clr_stats(struct i2cmst_priv *priv);
int i2cmst_get_stats(struct i2cmst_priv *priv, struct i2cmst_stats *stats);
int i2cmst_reclaim(struct i2cmst_priv *priv, struct i2cmst_list *chain);


/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct i2cmst_config {
        int     speed;          /* Standard or fast speed */
        int     ten_bit_addr;   /* Ten bit addressing enabled */
        int     retry;          /* Max number of retries*/

        /* Interrupt options */
        int      isr_pkt_proc;  /* Enable ISR to process packets */
};

struct i2cmst_priv {
        volatile struct i2cmst_regs      *regs;
        int                     irq;

        uint8_t                 open;
        int                     running;

        /* Collections of frames Ready to be sent/ Scheduled for transmission/Sent
         * frames waiting for the user to reclaim
         */
        int                     pos;            /* Position in the scheduled transfer*/
        int                     ack;            /* Check RxACK*/
        int                     length;         /* Length of current transfer */
        uint8_t                 header[4];      /* Header buffer */
        int                     state;          /* State of the scheduled transfer*/
        int                     retry;          /* Transmission attemps */
        struct i2cmst_list      queue;          /* Frames waiting  */
        struct i2cmst_packet    *scheduled;     /* Frame being transmitted */

        struct i2cmst_config    config;
        struct i2cmst_stats     stats;

        struct osal_isr_ctx     isr_ctx;
        SPIN_DECLARE(devlock)
};

struct i2cmst_devcfg {
        struct drv_devreg regs;
        struct i2cmst_priv priv;
};

#endif

