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

#include <drv/auto.h>
#include <drv/nelem.h>
#include <bcc/ambapp.h>

static uint32_t fn(
        void *info,
        uint32_t vendor,
        uint32_t device,
        uint32_t type,
        uint32_t depth,
        void *arg
)
{
        UNUSED(vendor); UNUSED(device); UNUSED(type); UNUSED(depth);
        struct auto_cfg *cfg = arg;
        struct amba_apb_info *apbi = info;

        struct drv_devreg *node;

        node = malloc(cfg->devsize);
        if (!node) {
                return 1;
        }
        node->addr = apbi->start;
        node->interrupt = apbi->irq;
        node->device_id = device;
        node->version = apbi->ver;
        cfg->reg(node);

        cfg->count++;

        return 0;
}

int drv_autoinit(struct auto_cfg *cfg)
{
        static const uint32_t ioarea = 0xfff00000;

        cfg->count = 0;
        ambapp_visit(
                ioarea,
                cfg->vendor,
                cfg->device,
                AMBAPP_VISIT_APBSLAVE,
                4,
                fn,
                cfg
        );
        return cfg->count;
}

