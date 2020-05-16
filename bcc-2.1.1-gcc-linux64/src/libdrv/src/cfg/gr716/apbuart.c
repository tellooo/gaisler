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

#include <stddef.h>
#include <drv/gr716/apbuart.h>
#include "pnp.h"

static struct apbuart_devcfg GR716_DEV_CFG_0 = {
  .regs = {
    .addr   = GAISLER_APBUART_0_PNP_APB,
    .interrupt  = GAISLER_APBUART_0_PNP_APB_IRQ
  },
};

static struct apbuart_devcfg GR716_DEV_CFG_1 = {
  .regs = {
    .addr   = GAISLER_APBUART_1_PNP_APB,
    .interrupt  = GAISLER_APBUART_1_PNP_APB_IRQ
  },
};

static struct apbuart_devcfg GR716_DEV_CFG_2 = {
  .regs = {
    .addr   = GAISLER_APBUART_2_PNP_APB,
    .interrupt  = GAISLER_APBUART_2_PNP_APB_IRQ
  },
};

static struct apbuart_devcfg GR716_DEV_CFG_3 = {
  .regs = {
    .addr   = GAISLER_APBUART_3_PNP_APB,
    .interrupt  = GAISLER_APBUART_3_PNP_APB_IRQ
  },
};

static struct apbuart_devcfg GR716_DEV_CFG_4 = {
  .regs = {
    .addr   = GAISLER_APBUART_4_PNP_APB,
    .interrupt  = GAISLER_APBUART_4_PNP_APB_IRQ
  },
};

static struct apbuart_devcfg GR716_DEV_CFG_5 = {
  .regs = {
    .addr   = GAISLER_APBUART_5_PNP_APB,
    .interrupt  = GAISLER_APBUART_5_PNP_APB_IRQ
  },
};

struct apbuart_devcfg *GR716_APBUART_DRV_ALL[] = {
    &GR716_DEV_CFG_0,
    &GR716_DEV_CFG_1,
    &GR716_DEV_CFG_2,
    &GR716_DEV_CFG_3,
    &GR716_DEV_CFG_4,
    &GR716_DEV_CFG_5,
    NULL
};

//#define APBUART_SYS_FREQ (40*1000*1000)
//#define APBUART_DEBUG 1
