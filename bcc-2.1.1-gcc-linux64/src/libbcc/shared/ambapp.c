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

#include <stdint.h>
#include <stdlib.h>

#include "include/bcc/ambapp.h"

#define AMBA_CONF_AREA                  0x000ff000
#define AMBA_AHB_MASTER_CONF_AREA       0x00000000
#define AMBA_AHB_SLAVE_CONF_AREA        0x00000800
#define AMBA_TYPE_APBIO 0x1
#define AMBA_TYPE_MEM   0x2
#define AMBA_TYPE_AHBIO 0x3

#define ambapp_pnp_vendor(id) (((id) >> 24) & 0xff)
#define ambapp_pnp_device(id) (((id) >> 12) & 0xfff)
#define ambapp_pnp_ver(id) (((id)>>5) & 0x1f)
#define ambapp_pnp_irq(id) ((((id)>>5) & 0x60) | ((id) & 0x1f))

#define ambapp_pnp_start(bar)  (((bar) & 0xfff00000) & (((bar) & 0xfff0) << 16))
#define ambapp_pnp_bar_type(bar) ((bar) & 0xf)
#define ambapp_pnp_bar_mask(bar) (((bar)>>4) & 0xfff)
#define ambapp_pnp_apb_start(bar, apbbase) \
        ((apbbase) | ((((bar) & 0xfff00000)>>12) & (((bar) & 0xfff0)<<4)) )
#define ambapp_pnp_apb_mask(bar) \
        (~(ambapp_pnp_bar_mask(bar)<<8) & 0x000fffff)

#define AMBA_TYPE_AHBIO_ADDR(addr,base_ioarea) \
        ((uint32_t)(base_ioarea) | ((addr) >> 12))

#if 0
#include <stdio.h>
/* Verbose debugging */
#define DBG(x) if (1) { (x); }
#else
#define DBG(x)
#endif

int ambapp_get_ahbinfo(
        uint32_t ioarea,
        const struct ambapp_ahb_entry *ahb,
        struct amba_ahb_info *info
)
{
        if (NULL == ahb) { return 1; }
        if (NULL == info) { return 2; }

        int i;

        info->entry = ahb;
        info->ver = ambapp_pnp_ver(ahb->id);
        info->irq = ambapp_pnp_irq(ahb->id);
        for (i = 0; i < AMBA_AHB_NBARS; i++) {
                uint32_t addr;
                uint32_t mask;
                uint32_t bar;

                bar = ahb->bar[i];
                if (0 == bar) {
                        addr = 0;
                        mask = 0;
                } else {
                        addr = ambapp_pnp_start(bar);
                        if (ambapp_pnp_bar_type(bar) == AMBA_TYPE_AHBIO) {
                                addr = AMBA_TYPE_AHBIO_ADDR(addr, ioarea);
                                mask = (uint32_t)(ambapp_pnp_bar_mask(~bar) << 8) | 0xff;
                        } else {
                                mask = ~((uint32_t)(ambapp_pnp_bar_mask(bar) << 20));
                                /*
                                 * NOTE: If mask = 0, the bar is disabled
                                 * rather than occupying the full AHB address
                                 * range. (GRLIB IP Library)
                                 */
                                if (mask == 0xffffffff) {
                                        mask = 0;
                                }
                        }
                }
                info->bar[i].start = addr;
                info->bar[i].mask = mask;
                info->bar[i].type = ambapp_pnp_bar_type(bar);
        }

        return 0;
}

int ambapp_get_apbinfo(
        uint32_t apbbase,
        const struct ambapp_apb_entry *apb,
        struct amba_apb_info *info
)
{
        if (NULL == apb) { return 1; }
        if (NULL == info) { return 2; }

        info->entry = apb;
        info->ver = ambapp_pnp_ver(apb->id);
        info->irq = ambapp_pnp_irq(apb->id);
        info->start = ambapp_pnp_apb_start(apb->bar, apbbase);
        info->mask = ambapp_pnp_apb_mask(apb->bar);

        return 0;
}

static uint32_t ambapp_visit_apbctrl(
        uint32_t apbbase,
        uint32_t vendor,
        uint32_t device,
        uint32_t depth,
        uint32_t (*fn)(
                void *info,
                uint32_t vendor,
                uint32_t device,
                uint32_t type,
                uint32_t depth,
                void *arg
        ),
        void *fn_arg
)
{
        const struct ambapp_apb_entry *apb;
        uint32_t ret;
        uint32_t v;
        uint32_t d;
        int i;

        DBG(printf("%s: (%d) Scanning in 0x%08x\n", __func__, depth, apbbase));

        /* Visit APB slaves on this bus. */
        apb = (const struct ambapp_apb_entry *) (
                apbbase | AMBA_CONF_AREA
        );
        for (i = 0; i < AMBA_APB_NSLAVES ; i++) {
                v = ambapp_pnp_vendor(apb->id);
                d = ambapp_pnp_device(apb->id);
                if (
                        (v | d) &&
                        (0 == vendor || v == vendor) &&
                        (0 == device || d == device) &&
                        (NULL != fn)
                ) {
                        struct amba_apb_info info;

                        ambapp_get_apbinfo(apbbase, apb, &info);
                        ret = fn(&info, v, d, AMBAPP_VISIT_APBSLAVE, depth, fn_arg);
                        if (0 != ret) {
                                return ret;
                        }
                }
                apb++;
        }

        return 0;
}

static uint32_t ambapp_visit_ahbctrl(
        uint32_t ioarea,
        uint32_t vendor,
        uint32_t device,
        uint32_t flags,
        uint32_t depth,
        uint32_t maxdepth,
        uint32_t (*fn)(
                void *info,
                uint32_t vendor,
                uint32_t device,
                uint32_t type,
                uint32_t depth,
                void *arg
        ),
        void *fn_arg
)
{
        const struct ambapp_ahb_entry *ahb;
        uint32_t ret;
        uint32_t v;
        uint32_t d;
        int i;

        DBG(printf("%s: (%d) Scanning in ioarea 0x%08x\n", __func__, depth, ioarea));

        if (0 == (flags & AMBAPP_VISIT_AHBMASTER)) {
                goto skip_ahbmasters;
        }
        /* Visit AHB masters on this bus. */
        ahb = (const struct ambapp_ahb_entry *) (
                ioarea | AMBA_CONF_AREA | AMBA_AHB_MASTER_CONF_AREA
        );
        for (i = 0; i < AMBA_AHB_NMASTERS; i++) {
                v = ambapp_pnp_vendor(ahb->id);
                d = ambapp_pnp_device(ahb->id);
                if (
                        (v | d) &&
                        (0 == vendor || v == vendor) &&
                        (0 == device || d == device) &&
                        (NULL != fn)
                ) {
                        struct amba_ahb_info info;

                        ambapp_get_ahbinfo(ioarea, ahb, &info);
                        ret = fn(&info, v, d, AMBAPP_VISIT_AHBMASTER, depth, fn_arg);
                        if (0 != ret) {
                                return ret;
                        }
                }
                ahb++;
        }
    skip_ahbmasters:

        /* Visit AHB slaves on this bus. */
        ahb = (const struct ambapp_ahb_entry *) (
                ioarea | AMBA_CONF_AREA | AMBA_AHB_SLAVE_CONF_AREA
        );
        for (i = 0; i < AMBA_AHB_NSLAVES ; i++) {
                v = ambapp_pnp_vendor(ahb->id);
                d = ambapp_pnp_device(ahb->id);
                if (
                        (flags & AMBAPP_VISIT_AHBSLAVE) &&
                        (v | d) &&
                        (0 == vendor || v == vendor) &&
                        (0 == device || d == device) &&
                        (NULL != fn)
                ) {
                        struct amba_ahb_info info;

                        ambapp_get_ahbinfo(ioarea, ahb, &info);
                        ret = fn(&info, v, d, AMBAPP_VISIT_AHBSLAVE, depth, fn_arg);
                        if (0 != ret) {
                                return ret;
                        }
                }
                if (maxdepth <= depth) {
                        goto skip_bridges;
                }

                /* If AHB bridge then recurse. */
                /* NOTE: Enter independent of VISIT_AHBSLAVE. */
                if (
                        (VENDOR_GAISLER == v && GAISLER_AHB2AHB == d) ||
                        (VENDOR_GAISLER == v && GAISLER_L2CACHE == d) ||
                        (VENDOR_GAISLER == v && GAISLER_GRIOMMU == d) ||
                        0
                ) {
                        uint32_t newiobase;

                        newiobase = ahb->user[1];

                        if (0 != newiobase) {
                                DBG(printf("bridge at %08x, enter it...\n", newiobase));
                                ret = ambapp_visit_ahbctrl(
                                        newiobase,
                                        vendor,
                                        device,
                                        flags,
                                        depth + 1,
                                        maxdepth,
                                        fn,
                                        fn_arg
                                );
                                if (0 != ret) {
                                        return ret;
                                }
                        }
                }

                /* If APB bridge then recurse. */
                /* NOTE: Do not enter if 0 == AMBAPP_VISIT_APBSLAVE. */
                if (
                        (flags & AMBAPP_VISIT_APBSLAVE) &&
                        (VENDOR_GAISLER == v && GAISLER_APBMST == d)
                ) {
                        uint32_t apbbase;

                        apbbase = ambapp_pnp_start(ahb->bar[0]);
                        DBG(printf("apbctrl at %08x, enter it...\n", apbbase));
                        ret = ambapp_visit_apbctrl(
                                apbbase,
                                vendor,
                                device,
                                depth + 1,
                                fn,
                                fn_arg
                        );
                        if (0 != ret) {
                                return ret;
                        }
                }
            skip_bridges:

                ahb++;
        }

        return 0;
}

uint32_t ambapp_visit(
        uint32_t ioarea,
        uint32_t vendor,
        uint32_t device,
        uint32_t flags,
        uint32_t maxdepth,
        uint32_t (*fn)(
                void *info,
                uint32_t vendor,
                uint32_t device,
                uint32_t type,
                uint32_t depth,
                void *arg
        ),
        void *fn_arg
)
{
        uint32_t ret;

        ret = ambapp_visit_ahbctrl(
                ioarea,
                vendor,
                device,
                flags,
                0,
                maxdepth,
                fn,
                fn_arg
        );

        return ret;
}

