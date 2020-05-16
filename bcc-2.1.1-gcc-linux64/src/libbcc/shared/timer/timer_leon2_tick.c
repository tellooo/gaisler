/*
 * Copyright (c) 2017, Cobham Gaisler AB
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
 * Mini driver for supporting BCC / C library time functionality.
 */
#include <sys/time.h>
#include "bcc/bcc_param.h"
#include "bcc/regs/leon2timer.h"
#include "timer_custom.h"

enum {
        US_PER_S = (1000 * 1000),
        LEON2TIMER_RESOLUTION = US_PER_S,
};

struct timer_tick_priv {
        struct bcc_isr_node isr;
        volatile struct timeval tickval;
        uint32_t usec_per_tick;
};
static struct timer_tick_priv priv;

/* Fill in a timeval: can be used by gettimeofday(). */
static void __bcc_timer_tick_get_timeval(
        struct timeval *tv
)
{
        time_t sec;
        suseconds_t usec;
        int plevel;

        plevel = bcc_int_disable();

        sec = priv.tickval.tv_sec;
        usec = priv.tickval.tv_usec;

        bcc_int_enable(plevel);

        tv->tv_sec = sec;
        tv->tv_usec = usec;
}

static uint32_t __bcc_timer_tick_get_us(void)
{
        struct timeval tv;

        __bcc_timer_tick_get_timeval(&tv);

        return tv.tv_sec * US_PER_S + tv.tv_usec;
}

static void __bcc_timer_tick_isr(
        void *arg,
        int source
)
{
        (void) source;
        (void) arg;

        priv.tickval.tv_usec += priv.usec_per_tick;
        if (US_PER_S <= priv.tickval.tv_usec) {
                priv.tickval.tv_usec -= US_PER_S;
                priv.tickval.tv_sec++;
        }
}

/* Public function for initializing this driver. */
int bcc_timer_tick_init_period(
        uint32_t usec_per_tick
)
{
        if (
                (0 == __bcc_timer_handle) ||
                (0 == __bcc_timer_interrupt)
        ) {
                return BCC_NOT_AVAILABLE;
        }

        /* Service can be enabled only once. */
        if (__bcc_timer_custom_get_timeval) { return BCC_FAIL; }

        /* Default to 100 tick interrupts per second */
        if (0 == usec_per_tick) {
                usec_per_tick = 10 * 1000;
        }
        priv.usec_per_tick = usec_per_tick;

        /* Get initial values from timer. */
        {
                uint32_t us;

                us = bcc_timer_get_us();
                priv.tickval.tv_sec = us / US_PER_S;
                priv.tickval.tv_usec = us % US_PER_S;
        }

        volatile struct leon2timer_regs *regs = (void *) __bcc_timer_handle;
        volatile struct leon2timer_timer_regs *tmr = &regs->timer[0];
        int intsource = __bcc_timer_interrupt;

        bcc_int_mask(intsource);
        bcc_int_clear(intsource);

        /* Stop timer. */
        tmr->ctrl = 0;

        /* Assume timer scaler value configured for 1 MHz. */
        tmr->counter = 0;
        tmr->reload = priv.usec_per_tick - 1;
        tmr->ctrl = (
                LEON2TIMER_CTRL_LD |
                LEON2TIMER_CTRL_RL |
                LEON2TIMER_CTRL_EN
        );

        int ret;
        priv.isr.source = intsource;
        priv.isr.handler = __bcc_timer_tick_isr;
        priv.isr.arg = (void *) tmr;
        ret = bcc_isr_register_node(&priv.isr);
        if (BCC_OK != ret) {
                return ret;
        }

        /* Install vectors to override some simple BCC time functionality. */
        __bcc_timer_custom_get_us = __bcc_timer_tick_get_us;
        __bcc_timer_custom_get_timeval = __bcc_timer_tick_get_timeval;
        bcc_int_unmask(intsource);

        return BCC_OK;
}

