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

#ifndef __IRQMP_REGS_H_
#define __IRQMP_REGS_H_

/*
 * Register description for IRQMP
 * IRQMP      - Multiprocessor Interrupt Controller
 * IRQ(A)MP   - Multiprocessor Interrupt Controller with extended ASMP support
 */

#include <stdint.h>

#define IRQMP_NCPU_MAX 16
/* IRQMP and IRQAMP interrupt controller timestamps */
struct irqmp_timestamp_regs {
        uint32_t counter;       /* 0x00 */
        uint32_t control;       /* 0x04 */
        uint32_t assertion;     /* 0x08 */
        uint32_t ack;           /* 0x0c */
};

/* IRQMP and IRQAMP interrupt controllers */
struct irqmp_regs {
        uint32_t ilevel;        /* 0x00 */
        uint32_t ipend;         /* 0x04 */
        uint32_t iforce0;       /* 0x08 */
        uint32_t iclear;        /* 0x0c */
        uint32_t mpstat;        /* 0x10 */
        uint32_t brdlst;        /* 0x14 */
        uint32_t errstat;       /* 0x18 */
        uint32_t wdogctrl;      /* 0x1c */
        uint32_t asmpctrl;      /* 0x20 */
        uint32_t icselr[2];     /* 0x24,0x28 */
        uint32_t notused13;   /* 0x2c */
        uint32_t notused20;   /* 0x30 */
        uint32_t notused21;   /* 0x34 */
        uint32_t notused22;   /* 0x38 */
        uint32_t notused23;   /* 0x3c */
        uint32_t pimask[IRQMP_NCPU_MAX];    /* 0x40 */
        uint32_t piforce[IRQMP_NCPU_MAX];   /* 0x80 */
        /* Extended IRQ registers */
        uint32_t pextack[IRQMP_NCPU_MAX];   /* 0xc0 */
        struct irqmp_timestamp_regs timestamp[16]; /* 0x100 */
        uint32_t resetaddr[IRQMP_NCPU_MAX]; /* 0x200 */
        uint32_t resv1[(0x300-0x240)/4];    /* 0x240 */
        /* Interrupt map register n */
        uint32_t map[64/4];                 /* 0x300 */
        /* Align to 4 KiB boundary */
        uint32_t resv2[(0x1000-0x340)/4];   /* 0x340 */
};

#define IRQMP_IFORCE0_IF_BIT    0
#define IRQMP_IFORCE0_IF        (0xfffe << IRQMP_IFORCE0_IF_BIT)

#define IRQMP_ICLEAR_IC_BIT    0
#define IRQMP_ICLEAR_IC        (0xfffe << IRQMP_ICLEAR_IC_BIT)
#define IRQMP_ICLEAR_EIC_BIT   16
#define IRQMP_ICLEAR_EIC       (0xffff << IRQMP_ICLEAR_EIC_BIT)

#define IRQMP_ASMPCTRL_NCTRL_BIT        28
#define IRQMP_ASMPCTRL_NCTRL            (0xf << IRQMP_ASMPCTRL_NCTRL_BIT)

#define IRQMP_ICSELR0_ICSEL7_BIT        0
#define IRQMP_ICSELR0_ICSEL7            (0xf << IRQMP_ICSELR0_ICSEL7_BIT)
#define IRQMP_ICSEL_PER_ICSELR          8

#define IRQMP_MPSTAT_NCPU_BIT   28
#define IRQMP_MPSTAT_NCPU       (0xf << IRQMP_MPSTAT_NCPU_BIT)
#define IRQMP_MPSTAT_EIRQ_BIT   16
#define IRQMP_MPSTAT_EIRQ       (0xf << IRQMP_MPSTAT_EIRQ_BIT)
#define IRQMP_MPSTAT_STATUS_BIT 0
#define IRQMP_MPSTAT_STATUS	(0xffff << IRQMP_MPSTAT_STATUS_BIT)

#define IRQMP_PIFORCE_IF_BIT    0
#define IRQMP_PIFORCE_IF        (0xfffe << IRQMP_PIFORCE_IF_BIT)

#define IRQMP_PEXTACK_EID       (0x1f << 0)

#define IRQMP_TCTRL_TSTAMP_BIT          27
#define IRQMP_TCTRL_TSTAMP              (0x1f << IRQMP_TCTRL_TSTAMP_BIT)
#define IRQMP_TCTRL_S1_BIT              26
#define IRQMP_TCTRL_S1                  (0x01 << IRQMP_TCTRL_S1_BIT)
#define IRQMP_TCTRL_S2_BIT              25
#define IRQMP_TCTRL_S2                  (0x01 << IRQMP_TCTRL_S2_BIT)
#define IRQMP_TCTRL_KS_BIT               5
#define IRQMP_TCTRL_KS                  (0x01 << IRQMP_TCTRL_KS_BIT)
#define IRQMP_TCTRL_TSISEL_BIT           0
#define IRQMP_TCTRL_TSISEL              (0x1f << IRQMP_TCTRL_TSISEL_BIT)

#endif

