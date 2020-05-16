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

#ifndef DRV_GRGPIO_REGS_H
#define DRV_GRGPIO_REGS_H

#include <stdint.h>

struct grgpio_regs {
        uint32_t data;          /* 0x000 */
        uint32_t output;        /* 0x004 */
        uint32_t direction;     /* 0x008 */
        uint32_t intmask;       /* 0x00C */
        uint32_t intpol;        /* 0x010 */
        uint32_t intedge;       /* 0x014 */
        uint32_t _res0;         /* 0x018 */
        uint32_t cap;           /* 0x01C */
        uint32_t irqmap[8];     /* 0x020 - 0x3C */
        uint32_t iavail;        /* 0x040 */
        uint32_t iflag;         /* 0x044 */
        uint32_t _res1;         /* 0x048 */
        uint32_t pulse;         /* 0x04C */
        uint32_t _res2;         /* 0x050 */
        uint32_t output_or;     /* 0x054 */
        uint32_t direction_or;  /* 0x058 */
        uint32_t intmask_or;    /* 0x05C */
        uint32_t _res3;         /* 0x060 */
        uint32_t output_and;    /* 0x064 */
        uint32_t direction_and; /* 0x068 */
        uint32_t intmask_and;   /* 0x06C */
        uint32_t _res4;         /* 0x070 */
        uint32_t output_xor;    /* 0x074 */
        uint32_t direction_xor; /* 0x078 */
        uint32_t intmask_xor;   /* 0x07C */
};

#define GRGPIO_CAP_PU_BIT       18
#define GRGPIO_CAP_IER_BIT      17
#define GRGPIO_CAP_IFL_BIT      16
#define GRGPIO_CAP_IRQGEN_BIT    8
#define GRGPIO_CAP_NLINES_BIT    0

#define GRGPIO_CAP_PU           (0x1 << GRGPIO_CAP_PU_BIT)
#define GRGPIO_CAP_IER          (0x1 << GRGPIO_CAP_IER_BIT)
#define GRGPIO_CAP_IFL          (0x1 << GRGPIO_CAP_IFL_BIT)
#define GRGPIO_CAP_IRQGEN       (0x1f << GRGPIO_CAP_IRQGEN_BIT)
#define GRGPIO_CAP_NLINES       (0x1f << GRGPIO_CAP_NLINES_BIT)

#endif

