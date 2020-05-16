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

#include <stddef.h>
#include <drv/gr716/i2cmst.h>
#include "pnp.h"
#include <bcc/ambapp_ids.h>

struct i2cmst_devcfg *GR716_I2CMST_DRV_ALL[] = {
        & (struct i2cmst_devcfg) {
                .regs = {
                        .addr       = GAISLER_I2CMST_0_PNP_APB,
                        .interrupt  = GAISLER_I2CMST_0_PNP_APB_IRQ,
                        .device_id  = GAISLER_I2CMST,
                        .version    = GAISLER_I2CMST_0_PNP_VERSION,
                },
        },
        & (struct i2cmst_devcfg) {
                .regs = {
                        .addr       = GAISLER_I2CMST_1_PNP_APB,
                        .interrupt  = GAISLER_I2CMST_1_PNP_APB_IRQ,
                        .device_id  = GAISLER_I2CMST,
                        .version    = GAISLER_I2CMST_1_PNP_VERSION,
                },
        },
        NULL
};

