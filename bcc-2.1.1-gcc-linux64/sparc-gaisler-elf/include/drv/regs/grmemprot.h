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

#ifndef DRV_GRMEMPROT_REGS_H
#define DRV_GRMEMPROT_REGS_H

#include <stdint.h>

struct grmemprot_segment {
        /* start */
        uint32_t psa;
        /* end */
        uint32_t pea;
        /* control */
        uint32_t psc;
        uint32_t _res0;
};

struct grmemprot_apb {
        /* 0x00 */
        /* "APB Control x Protection register 0" */
        uint32_t prot0;
        uint32_t prot1;
        uint32_t _resa[2];
        /* 0x10 */
        uint32_t prot2;
        uint32_t prot3;
        uint32_t _resb[2];
        /* 0x20 */
        uint32_t _resc[8];
        /* 0x40 */
};

/* Clock gating unit registers */
struct grmemprot_regs {
        /* Protection Configuration register */
        uint32_t pcr;
        struct grmemprot_segment seg[15];
        /* 0x0f4 */
        uint32_t _resx[3];

        /* 0x100 */
        struct grmemprot_apb apb0;
        /* 0x140 */
        struct grmemprot_apb apb1;
        /* 0x180 */
        struct grmemprot_apb apb3;
        /* 0x1c0 */
        struct grmemprot_apb apb4;
};

#define GRMEMPROT_PCR_NSEG_BIT          24
#define GRMEMPROT_PCR_PROT_BIT           1
#define GRMEMPROT_PCR_EN_BIT             0

#define GRMEMPROT_PCR_NSEG              (0x00ff << GRMEMPROT_PCR_NSEG_BIT)
#define GRMEMPROT_PCR_PROT              (0x0007 << GRMEMPROT_PCR_PROT_BIT)
#define GRMEMPROT_PCR_PROT_MAGIC        (0x0005 << GRMEMPROT_PCR_PROT_BIT)
#define GRMEMPROT_PCR_EN                (0x0001 << GRMEMPROT_PCR_EN_BIT)

#define GRMEMPROT_PSC_EN_BIT             0
#define GRMEMPROT_PSC_G_BIT             16

#define GRMEMPROT_PSC_EN                (0x0001 << GRMEMPROT_PSC_EN_BIT)
#define GRMEMPROT_PSC_G                 (0xffff << GRMEMPROT_PSC_G_BIT)

#endif

