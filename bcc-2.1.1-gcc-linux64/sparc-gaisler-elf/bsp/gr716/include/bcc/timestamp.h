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

/*
 * API for working with the interrupt controller timestamp registers.
 * Implementation is inlined to be quick and prevent save in ISR.
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */

#ifndef BCC_TIMESTAMP_H
#define BCC_TIMESTAMP_H

#include <stdint.h>

enum {
        /* Assertion Stamped (S1) */
        BCC_TIMESTAMP_ASS       = 1 << 26,
        /* Acknowledge Stamped (S2) */
        BCC_TIMESTAMP_ACK       = 1 << 25,
};

/*
 * Return number of timestamp register sets available
 */
int bcc_timestamp_avail(void);

/* Reset timestamp registers */
static inline int bcc_timestamp_restart(int ts_no, int source);

/*
 * Get stamped status
 *
 * The function is used to determine whether the assertion, acknowledge or both
 * have been stamped for the timestamp register set.
 *
 * return: a combination of the masks BCC_TIMESTAMP_ASS and BCC_TIMESTAMP_ACK.
 */
static inline uint32_t bcc_timestamp_status(int ts_no);

/* Return value of interrupt assertion timestamp register */
static inline uint32_t bcc_timestamp_get_ass(int ts_no);

/* Return value of interrupt acknowledge timestamp register */
static inline uint32_t bcc_timestamp_get_ack(int ts_no);

/* Return interrupt timestamp counter register */
static inline uint32_t bcc_timestamp_get_cnt(void);


/* IMPLEMENTATION */
#include <bcc/bcc_param.h>
#include <bcc/regs/irqmp.h>

int bcc_timestamp_restart(int ts_no, int source)
{
        volatile struct irqmp_regs *regs =
            (struct irqmp_regs *) __bcc_int_handle;

        regs->timestamp[ts_no].control = (
                IRQMP_TCTRL_S1 |
                IRQMP_TCTRL_S2 |
                IRQMP_TCTRL_KS |
                (IRQMP_TCTRL_TSISEL & source) |
                0
        );

        return BCC_OK;
}

static inline uint32_t bcc_timestamp_status(int ts_no)
{
        uint32_t status;
        volatile struct irqmp_regs *regs;

        regs = (struct irqmp_regs *) __bcc_int_handle;
        status = regs->timestamp[ts_no].control;
        status &= (IRQMP_TCTRL_S1 | IRQMP_TCTRL_S2);
        return status;
}

static inline uint32_t bcc_timestamp_get_ass(int ts_no)
{
        volatile struct irqmp_regs *regs =
            (struct irqmp_regs *) __bcc_int_handle;

        return regs->timestamp[ts_no].assertion;
}


static inline uint32_t bcc_timestamp_get_ack(int ts_no)
{
        volatile struct irqmp_regs *regs =
            (struct irqmp_regs *) __bcc_int_handle;

        return regs->timestamp[ts_no].ack;
}

static inline uint32_t bcc_timestamp_get_cnt(void)
{
        volatile struct irqmp_regs *regs =
            (struct irqmp_regs *) __bcc_int_handle;

        return regs->timestamp[0].counter;
}

#endif

