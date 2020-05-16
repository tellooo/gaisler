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

#include <bsp.h>
#include "bcc/ambapp.h"
#include "int_irqmp_priv.h"

static const uint32_t ioarea = 0xfff00000;

static int get_eirq(volatile struct irqmp_regs *regs)
{
        return (regs->mpstat & IRQMP_MPSTAT_EIRQ) >> IRQMP_MPSTAT_EIRQ_BIT;
}

/*
 * Probe map registers
 * return:
 *   0: map registers not available
 *   1: map registers available
 */
static int get_map(volatile struct irqmp_regs *regs, uint32_t mask)
{
        if (mask <= 0x300) {
                return 0;
        }

        uint32_t map = 0;
        uint32_t rst = 0;
        uint32_t old_map = 0;
        uint32_t old_rst = 0;
        uint32_t inv = 0;

        old_map = regs->map[0];
        old_rst = regs->resetaddr[0];

        /* Check wrapping */
        inv =  ~old_rst;
        regs->map[0] = inv;
        rst = regs->resetaddr[0];
        if (rst != old_rst) {
                /* 0x200 was written, map not implemented */
                regs->resetaddr[0] = old_rst;
                return 0;
        }

        /* Check if 0x300 is writable */
        inv = ~old_map;
        regs->map[0] = inv;
        map = regs->map[0];

        if (map == old_map) {
                /* not writable */
                return 0;
        }

        /* 0x300 was written, map implemented */
        regs->map[0] = old_map;

        return 1;
}

static int get_nts(volatile struct irqmp_regs *regs, uint32_t mask)
{
        if (mask <= 0x100) {
                return 0;
        }
        return (regs->timestamp[0].control & IRQMP_TCTRL_TSTAMP) >>
          IRQMP_TCTRL_TSTAMP_BIT;
}

int __bcc_int_init(void)
{
        /*
         * If the BSP has set a handle value at compile-time then just return.
         * The rest of this function will be optimized away.
         */
        if (__BSP_INT_HANDLE) {
                return BCC_OK;
        }

        volatile struct irqmp_regs *regs;

        if (__bcc_int_handle) {
                /*
                 * Return without scanning if interrupt controller handle was
                 * defined at link time. But first probe extended interrupt
                 * number if needed.
                 */
                if (0 == __bcc_int_irqmp_eirq) {
                        regs = (struct irqmp_regs *) __bcc_int_handle;
                        __bcc_int_irqmp_eirq = get_eirq(regs);
                }

                return BCC_OK;
        }

        struct amba_apb_info apbi;

        regs = (volatile struct irqmp_regs *) ambapp_visit(
                ioarea,
                VENDOR_GAISLER,
                GAISLER_IRQMP,
                AMBAPP_VISIT_APBSLAVE,
                4,
                ambapp_findfirst_fn,
                &apbi
        );

        if (NULL == regs) {
                /* Interrupt controller not found by ambapp. */
                return BCC_NOT_AVAILABLE;
        }

        /*
         * Adjust IRQ(A)MP register base to a controller assigned to us by the
         * boot loader.
         *
         * The Asymmetric Multiprocessing Control Register will tell us number
         * of available internal interrupt controllers.
         */
        if (regs->asmpctrl & IRQMP_ASMPCTRL_NCTRL) {
                /*
                 * Interrupt Controller Select Registers are available. Find
                 * the one belonging to us and index into it to get an
                 * interrupt controller number. Then set regs to it.
                 */
                uint32_t icsel;
                uint32_t icselshift;
                uint32_t cpuid;

                cpuid = __bcc_cpuid;
                icsel = regs->icselr[cpuid / IRQMP_ICSEL_PER_ICSELR];
                icselshift = (
                        (IRQMP_ICSEL_PER_ICSELR-1) -
                        (cpuid % IRQMP_ICSEL_PER_ICSELR)
                ) * 4;
                icsel = (icsel >> icselshift) & IRQMP_ICSELR0_ICSEL7;
                /* NOTE: struct irqmp_regs is 4 KiB. */
                regs += icsel;
        }
        __bcc_int_handle = (uint32_t) regs;

        /* NOTE: EIRQ, MAP and NTS is always probed if IRQMP was scanned. */
        __bcc_int_irqmp_eirq = get_eirq(regs);
        __bcc_int_irqmp_map = get_map(regs, apbi.mask);
        __bcc_int_irqmp_nts = get_nts(regs, apbi.mask);

        return BCC_OK;
}

