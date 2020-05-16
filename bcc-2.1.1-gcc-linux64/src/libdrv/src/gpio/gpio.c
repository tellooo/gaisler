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

#include <drv/gpio.h>
#include <drv/osal.h>
#include <drv/drvret.h>
#include <drv/nelem.h>

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int gpio_register(struct gpio_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int gpio_init(struct gpio_devcfg *devcfgs[])
{
        struct gpio_devcfg **dev = &devcfgs[0];

        while (*dev) {
                gpio_register(*dev);
                dev++;
        }
        return DRV_OK;
}


int gpio_dev_count(void)
{
        return dev_count;
}

struct gpio_priv *gpio_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }

        struct gpio_devcfg *dev =
            (struct gpio_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct gpio_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        priv->regs = (struct grgpio_regs *)dev->regs.addr;
        /* Initialize registers. */
        priv->regs->intmask = 0;
        priv->regs->direction = 0;

        return priv;
}

int gpio_close(struct gpio_priv *priv)
{
        priv->regs->intmask = 0;
        priv->regs->direction = 0;
        priv->open = 0;
        return DRV_OK;
}

uint32_t gpio_data(struct gpio_priv *priv)
{
        uint32_t oldval = priv->regs->data;

        return oldval;
}

uint32_t gpio_output(struct gpio_priv *priv, int set, uint32_t newval)
{
        uint32_t oldval = priv->regs->output;

        if (set) {
                priv->regs->output = newval;
        }
        return oldval;
}

uint32_t gpio_direction(struct gpio_priv *priv, int set, uint32_t newval)
{
        uint32_t oldval = priv->regs->direction;

        if (set) {
                priv->regs->direction = newval;
        }
        return oldval;
}

uint32_t gpio_intmask(struct gpio_priv *priv, int set, uint32_t newval)
{
        uint32_t oldval = priv->regs->intmask;

        if (set) {
                priv->regs->intmask = newval;
        }
        return oldval;
}

uint32_t gpio_intpol(struct gpio_priv *priv, int set, uint32_t newval)
{
        uint32_t oldval = priv->regs->intpol;

        if (set) {
                priv->regs->intpol = newval;
        }
        return oldval;
}

uint32_t gpio_intedge(struct gpio_priv *priv, int set, uint32_t newval)
{
        uint32_t oldval = priv->regs->intedge;

        if (set) {
                priv->regs->intedge = newval;
        }
        return oldval;
}

uint32_t gpio_intflag(struct gpio_priv *priv, int set, uint32_t newval)
{
        uint32_t oldval = priv->regs->iflag;

        if (set) {
                priv->regs->iflag = newval;
        }
        return oldval;
}

uint32_t gpio_pulse(struct gpio_priv *priv, int set, uint32_t newval)
{
        uint32_t oldval = priv->regs->pulse;

        if (set) {
                priv->regs->pulse = newval;
        }
        return oldval;
}

int gpio_intmap_set(struct gpio_priv *priv, int i, int intline)
{
        int regnum = i / 4;
        int bitnum = (3 - (i & 3)) * 8;
        uint32_t thereg = priv->regs->irqmap[regnum];

        thereg &= ~(0x1f << bitnum);
        thereg |= (intline << bitnum);
        priv->regs->irqmap[regnum] = thereg;
        return DRV_OK;
}

int gpio_intmap_get(struct gpio_priv *priv, int i)
{
        int regnum = i / 4;
        int bitnum = (3 - (i & 3)) * 8;
        uint32_t thereg = priv->regs->irqmap[regnum];
        uint32_t oldval = (thereg >> bitnum) & 0x1f;

        return oldval;
}

uint32_t gpio_cap_get(struct gpio_priv *priv)
{
        return priv->regs->cap;
}

