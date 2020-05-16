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

#ifndef DRV_TIMER_H
#define DRV_TIMER_H

/*
 * HDSW driver for GPTIMER and GRTIMER
 *
 * The main mechanics of this driver consist in managing timer device and
 * subtimers open/close operations. Actual timer operations exported to the
 * user are highly connected on how the hardware registers work. Register
 * manipulation should be done with constants defined in the file
 * regs/gptimer-regs.h.
 */

#include <stdint.h>
#include <drv/auto.h>

#include <drv/osal.h>
#include <drv/regs/gptimer-regs.h>

struct timer_devcfg;
struct timer_priv;

/* return: number of devices registered */
int timer_autoinit(void);
/* Register one device */
int timer_register(struct timer_devcfg *devcfg);
/* Register an array of devices */
int timer_init(struct timer_devcfg *devcfgs[]);

/*** Device interface ***/

/* Retrieve number of timer devices registered to the driver. */
int timer_dev_count(void);

/*
 * Open timer device.
 * Configuration register and latch configuration
 * register is cleared. No other registers are touched.
 * return: device handle
 */
struct timer_priv *timer_open(int dev_no);

/* priv: device handle */
int timer_close(struct timer_priv *priv);

/** Device register access **/

/* priv: device handle */
void timer_set_scaler(struct timer_priv *priv, uint32_t value);

/* priv: device handle */
void timer_set_scaler_reload(struct timer_priv *priv, uint32_t value);

/* priv: device handle */
uint32_t timer_get_scaler_reload(struct timer_priv *priv);

/* priv: device handle */
uint32_t timer_get_cfg(struct timer_priv *priv);

/* priv: device handle */
void timer_set_cfg(struct timer_priv *priv, uint32_t value);

/* priv: device handle */
void timer_set_latch_cfg(struct timer_priv *priv, uint32_t value);

/* Subtimer interface */

/*
 * Open subtimer sub_no on device d. No registers are affected.
 *
 * priv: device handle
 * sub_no: subtimer 0...
 * return: subtimer handle
 */
void *timer_sub_open(struct timer_priv *priv, int sub_no);

/*
 * priv: device handle
 * sub_no: same as for open
 * return: DRV_OK or DRV_NOTOPEN
 */
int timer_sub_close(struct timer_priv *priv, int sub_no);

/* Subtimer register access. */

/* s: subtimer handle */
static inline uint32_t timer_get_counter(void *s)
{
        volatile struct gptimer_timer_regs *sregs = s;

        return sregs->counter;
}

/* s: subtimer handle */
static inline uint32_t timer_get_reload(void *s)
{
        volatile struct gptimer_timer_regs *sregs = s;

        return sregs->reload;
}

/* s: subtimer handle */
static inline void timer_set_reload(void *s, uint32_t value)
{
        volatile struct gptimer_timer_regs *sregs = s;

        sregs->reload = value;
}

/* s: subtimer handle */
static inline uint32_t timer_get_ctrl(void *s)
{
        volatile struct gptimer_timer_regs *sregs = s;

        return sregs->ctrl;
}

/* s: subtimer handle */
static inline void timer_set_ctrl(void *s, uint32_t value)
{
        volatile struct gptimer_timer_regs *sregs = s;

        sregs->ctrl = value;
}

/* s: subtimer handle */
static inline uint32_t timer_get_latch(void *s)
{
        volatile struct gptimer_timer_regs *sregs = s;

        return sregs->latch;
}

/* Subtimer helper functions */

/*
 * Stop subtimer and clear control register.
 *
 * s: subtimer handle
 */
static inline void timer_stop(void *s)
{
        timer_set_ctrl(s, 0);
}

/*
 * Load, enable and clear interrupt pending status of timer.
 */
void timer_kick(void *s);

/*
 * Return subtimer interrupt pending state.
 *
 * s: subtimer handle
 * return: 0 iff interrupt is not pending
 */
static inline int timer_ispend(void *s)
{
        return !!(GPTIMER_CTRL_IP & timer_get_ctrl(s));
}

/* NOTE: Assumes single task environment. */
void watchdog_system_restart(void *wdog_stmr);

/*
 * Returns device registers
 *
 * dev_no: the device number to return registers for
 *
 * return:
 * - Pointer to the device registers
 * - NULL if invalid device number
 */
const struct drv_devreg *timer_get_devreg(int dev_no);

/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct timer_priv {
        volatile struct gptimer_regs *regs;
        int nsub;
        uint8_t open;
        uint8_t subopen[GPTIMER_MAX_SUBTIMERS];
        /* Value used to clear GPTIMER_CTRL_IP */
        uint32_t ipmask;
        SPIN_DECLARE(devlock)
};

struct timer_devcfg {
        struct drv_devreg regs;
        struct timer_priv priv;
};

#endif

