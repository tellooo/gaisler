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

#include <drv/timer.h>
#include <drv/regs/gptimer-regs.h>
#include <drv/osal.h>
#include <drv/drvret.h>

static int dev_count;

static struct drv_list devlist = { NULL, NULL };

int timer_register(struct timer_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int timer_init(struct timer_devcfg *devcfgs[])
{
        struct timer_devcfg **dev = &devcfgs[0];

        while (*dev) {
                timer_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int timer_dev_count(void)
{
        return dev_count;
}

struct timer_priv *timer_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }

        struct timer_devcfg *dev =
          (struct timer_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct timer_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }
        for (size_t i = 0; i < sizeof(priv->subopen); i++) {
                priv->subopen[i] = 0;
        }
        SPIN_INIT(&priv->devlock, "drvtimer");

        priv->regs = (struct gptimer_regs *)dev->regs.addr;
        priv->ipmask = GPTIMER_CTRL_IP;
        /* Scaler it is expected to be initialized by boot loader. */
        /* Initialize registers. */
        priv->regs->cfg = 0;
        priv->nsub = priv->regs->cfg & GPTIMER_CFG_TIMERS;
        priv->regs->latch_cfg = 0;

        return priv;
}

int timer_close(struct timer_priv *priv)
{
        if (priv->open) {
                /* Close all open sub timers */
                for (int i = 0; i < priv->nsub; i++) {
                        timer_sub_close(priv, i);
                }
                priv->regs->cfg = 0;
                priv->open = 0;
        }
        return DRV_OK;
}

void timer_set_scaler(struct timer_priv *priv, uint32_t value)
{
        priv->regs->scaler_value = value;
}

void timer_set_scaler_reload(struct timer_priv *priv, uint32_t value)
{
        priv->regs->scaler_reload = value;
}

uint32_t timer_get_scaler_reload(struct timer_priv *priv)
{
        return priv->regs->scaler_reload;
}

uint32_t timer_get_cfg(struct timer_priv *priv)
{
        return priv->regs->cfg;
}

void timer_set_cfg(struct timer_priv *priv, uint32_t value)
{
        priv->regs->cfg = value;
}

/* d: device handle */
void timer_set_latch_cfg(struct timer_priv *priv, uint32_t value)
{
        priv->regs->latch_cfg = value;
}

void *timer_sub_open(struct timer_priv *priv, int sub_no)
{
        if (sub_no < 0) {
                return NULL;
        }
        if (priv->nsub <= sub_no) {
                return NULL;
        }

        uint8_t psubopen;

        psubopen = osal_ldstub(&(priv->subopen[sub_no]));
        if (psubopen) {
                return NULL;
        }

        volatile struct gptimer_timer_regs *sregs;

        sregs = &priv->regs->timer[sub_no];

        return (void *)sregs;
}

int timer_sub_close(struct timer_priv *priv, int sub_no)
{
        if (sub_no < 0) {
                return DRV_NOTOPEN;
        }

        int ret = DRV_NOTOPEN;
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        if (priv->nsub <= sub_no) {
                goto out;
        }

        if (0 == priv->subopen[sub_no]) {
                goto out;
        }
        priv->subopen[sub_no] = 0;
        ret = DRV_OK;

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return ret;
}

void timer_kick(void *s)
{
        uint32_t ctrl = timer_get_ctrl(s);

        ctrl |= GPTIMER_CTRL_LD | GPTIMER_CTRL_EN;
        ctrl |= GPTIMER_CTRL_IP;
        timer_set_ctrl(s, ctrl);
}

void watchdog_system_restart(void *wdog_stmr)
{
        timer_set_reload(wdog_stmr, 1);
        timer_set_ctrl(wdog_stmr, GPTIMER_CTRL_IE);
        timer_kick(wdog_stmr);
        /* Loop forever */
        while (1) {
                ;
        }
}

const struct drv_devreg *timer_get_devreg(int dev_no)
{
        const struct
            timer_devcfg
            *dev =
            (const struct timer_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}

