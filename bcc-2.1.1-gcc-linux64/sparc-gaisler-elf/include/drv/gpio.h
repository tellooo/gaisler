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

#ifndef DRV_GPIO_H
#define DRV_GPIO_H

/*
 * Driver for GRLIB GRGPIO
 *
 * The main mechanics of this driver consist in managing GRGPIO device
 * open/close operations. Actual GPIO operations exported to the
 * user are highly connected on how the hardware registers work. Register
 * manipulation can be performed with constants defined in the file
 * regs/grgpio.h.
 */

#include <stdint.h>
#include <drv/regs/grgpio.h>
#include <drv/auto.h>
#include <drv/drvret.h>

struct gpio_devcfg;
struct gpio_priv;

/* return: number of devices registered */
int gpio_autoinit(void);
/* Register one device */
int gpio_register(struct gpio_devcfg *devcfg);
/* Register an array of devices */
int gpio_init(struct gpio_devcfg *devcfgs[]);

/* Retrieve number of GRGPIO devices registered to the driver. */
int gpio_dev_count(void);

/*
 * Open a GPIO device.
 *
 * - All GPIO ports are configured as inputs
 * - Interrupts are disabled.
 * - Internal data structures are initialized.
 * - Device is marked opened.
 * return: device handle
 */
struct gpio_priv *gpio_open(int dev_no);

/* Close a GPIO device */
int gpio_close(struct gpio_priv *priv);

/** Device register access **/

/* Write output register */
static inline int gpio_write(struct gpio_priv *priv, uint32_t val);

/* Read data register */
static inline uint32_t gpio_read(struct gpio_priv *priv);

/*
 * d: device handle
 * return: Register content before newval is written
 */
uint32_t gpio_data(struct gpio_priv *priv);

/*
 * d: device handle
 * set: 0 - do not write register, others - write register
 * newval: value to write to register
 * return: Register content before newval is written
 */
uint32_t gpio_output(struct gpio_priv *priv, int set, uint32_t newval);
uint32_t gpio_direction(struct gpio_priv *priv, int set, uint32_t newval);
uint32_t gpio_intmask(struct gpio_priv *priv, int set, uint32_t newval);
uint32_t gpio_intpol(struct gpio_priv *priv, int set, uint32_t newval);
uint32_t gpio_intedge(struct gpio_priv *priv, int set, uint32_t newval);
uint32_t gpio_intflag(struct gpio_priv *priv, int set, uint32_t newval);
uint32_t gpio_pulse(struct gpio_priv *priv, int set, uint32_t newval);

/* Atomic bit access */
static inline int gpio_output_or      (struct gpio_priv *priv, uint32_t mask);
static inline int gpio_output_and     (struct gpio_priv *priv, uint32_t mask);
static inline int gpio_direction_or   (struct gpio_priv *priv, uint32_t mask);
static inline int gpio_direction_and  (struct gpio_priv *priv, uint32_t mask);
static inline int gpio_intmask_or     (struct gpio_priv *priv, uint32_t mask);
static inline int gpio_intmask_and    (struct gpio_priv *priv, uint32_t mask);

/*
 * Configure GRGPIO bit i to generate interrupt on line intline.
 *
 * return: DRV_OK
 */
int gpio_intmap_set(struct gpio_priv *priv, int i, int intline);

/*
 * Get interrupt line for GRGPIO bit i
 *
 * return: interrupt line for GRGPIO bit i
 */
int gpio_intmap_get(struct gpio_priv *priv, int i);

/*
 * Return value of capability register
 *
 * Use with GRGPIO_CAP_ defines.
 * NOTE: only exists on version 2 and later
 */
uint32_t gpio_cap_get(struct gpio_priv *priv);

/*
 * For use with gpio_output()
 * "0=output disabled, 1=output enabled"
 */
#define GPIO_DIRECTION_IN               0
#define GPIO_DIRECTION_OUT              1

/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct gpio_priv {
        volatile struct grgpio_regs *regs;
        uint8_t open;
};

struct gpio_devcfg {
        struct drv_devreg regs;
        struct gpio_priv priv;
};

static inline int gpio_write(struct gpio_priv *priv, uint32_t val)
{
        priv->regs->output = val;
        return DRV_OK;
}

static inline uint32_t gpio_read(struct gpio_priv *priv)
{
        return priv->regs->data;
}

static inline int gpio_output_or (struct gpio_priv *priv, uint32_t mask)
{
        priv->regs->output_or = mask;
        return DRV_OK;
}

static inline int gpio_output_and(struct gpio_priv *priv, uint32_t mask)
{
        priv->regs->output_and = mask;
        return DRV_OK;
}

static inline int gpio_direction_or (struct gpio_priv *priv, uint32_t mask)
{
        priv->regs->direction_or = mask;
        return DRV_OK;
}

static inline int gpio_direction_and(struct gpio_priv *priv, uint32_t mask)
{
        priv->regs->direction_and = mask;
        return DRV_OK;
}

static inline int gpio_intmask_or (struct gpio_priv *priv, uint32_t mask)
{
        priv->regs->intmask_or = mask;
        return DRV_OK;
}

static inline int gpio_intmask_and(struct gpio_priv *priv, uint32_t mask)
{
        priv->regs->intmask_and = mask;
        return DRV_OK;
}

#endif

