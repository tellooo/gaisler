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

/* SPICTRL definitions */

#ifndef _SPICTRL_REGS_H_
#define _SPICTRL_REGS_H_

#include <stdint.h>

struct spictrl_regs {
        uint32_t capability;    /* 0x00 */
        uint32_t resv0[7];      /* 0x04-0x1c */
        uint32_t mode;          /* 0x20 */
        uint32_t event;         /* 0x24 */
        uint32_t mask;          /* 0x28 */
        uint32_t command;       /* 0x2c */
        uint32_t tx;            /* 0x30 */
        uint32_t rx;            /* 0x34 */
        uint32_t slvsel;        /* 0x38 */
        uint32_t aslvsel;       /* 0x3c */
};

/* Capability register */
#define SPICTRL_CAPABILITY_SSSZ_BIT     24
#define SPICTRL_CAPABILITY_ASELA_BIT    17
#define SPICTRL_CAPABILITY_SSEN_BIT     16
#define SPICTRL_CAPABILITY_FDEPTH_BIT   8

#define SPICTRL_CAPABILITY_SSSZ         (0xff << SPICTRL_CAPABILITY_SSSZ_BIT)
#define SPICTRL_CAPABILITY_ASELA        (1 << SPICTRL_CAPABILITY_ASELA_BIT)
#define SPICTRL_CAPABILITY_SSEN         (1 << SPICTRL_CAPABILITY_SSEN_BIT)
#define SPICTRL_CAPABILITY_FDEPTH       (0xff << SPICTRL_CAPABILITY_FDEPTH_BIT)

/* Mode register */
#define SPICTRL_MODE_LOOP_BIT   30
#define SPICTRL_MODE_CPOL_BIT   29
#define SPICTRL_MODE_CPHA_BIT   28
#define SPICTRL_MODE_DIV16_BIT  27
#define SPICTRL_MODE_REV_BIT    26
#define SPICTRL_MODE_MS_BIT     25
#define SPICTRL_MODE_EN_BIT     24
#define SPICTRL_MODE_LEN_BIT    20
#define SPICTRL_MODE_PM_BIT     16
#define SPICTRL_MODE_ASEL_BIT   14
#define SPICTRL_MODE_FACT_BIT   13
#define SPICTRL_MODE_CG_BIT      7
#define SPICTRL_MODE_ASELDEL_BIT 5
#define SPICTRL_MODE_TAC_BIT     4
#define SPICTRL_MODE_IGSEL_BIT   2

#define SPICTRL_MODE_LOOP       (1 << SPICTRL_MODE_LOOP_BIT)
#define SPICTRL_MODE_CPOL       (1 << SPICTRL_MODE_CPOL_BIT)
#define SPICTRL_MODE_CPHA       (1 << SPICTRL_MODE_CPHA_BIT)
#define SPICTRL_MODE_DIV16      (1 << SPICTRL_MODE_DIV16_BIT)
#define SPICTRL_MODE_REV        (1 << SPICTRL_MODE_REV_BIT)
#define SPICTRL_MODE_MS         (1 << SPICTRL_MODE_MS_BIT)
#define SPICTRL_MODE_EN         (1 << SPICTRL_MODE_EN_BIT)
#define SPICTRL_MODE_LEN        (0xf << SPICTRL_MODE_LEN_BIT)
#define SPICTRL_MODE_PM         (0xf << SPICTRL_MODE_PM_BIT)
#define SPICTRL_MODE_ASEL       (1 << SPICTRL_MODE_ASEL_BIT)
#define SPICTRL_MODE_FACT       (1 << SPICTRL_MODE_FACT_BIT)
#define SPICTRL_MODE_CG         (0x1f << SPICTRL_MODE_CG_BIT)
#define SPICTRL_MODE_ASELDEL    (0x3 << SPICTRL_MODE_ASELDEL_BIT)
#define SPICTRL_MODE_TAC        (1 << SPICTRL_MODE_TAC_BIT)
#define SPICTRL_MODE_IGSEL      (1 << SPICTRL_MODE_IGSEL_BIT)

/* Event register */
#define SPICTRL_EVENT_TIP_BIT   31 /* NOTE: See GR712RC errata. */
#define SPICTRL_EVENT_AT_BIT    15
#define SPICTRL_EVENT_LT_BIT    14
#define SPICTRL_EVENT_OV_BIT    12
#define SPICTRL_EVENT_UN_BIT    11
#define SPICTRL_EVENT_MME_BIT   10
#define SPICTRL_EVENT_NE_BIT    9
#define SPICTRL_EVENT_NF_BIT    8

#define SPICTRL_EVENT_TIP       (1 << SPICTRL_EVENT_TIP_BIT)
#define SPICTRL_EVENT_AT        (1 << SPICTRL_EVENT_AT_BIT)
#define SPICTRL_EVENT_LT        (1 << SPICTRL_EVENT_LT_BIT)
#define SPICTRL_EVENT_OV        (1 << SPICTRL_EVENT_OV_BIT)
#define SPICTRL_EVENT_UN        (1 << SPICTRL_EVENT_UN_BIT)
#define SPICTRL_EVENT_MME       (1 << SPICTRL_EVENT_MME_BIT)
#define SPICTRL_EVENT_NE        (1 << SPICTRL_EVENT_NE_BIT)
#define SPICTRL_EVENT_NF        (1 << SPICTRL_EVENT_NF_BIT)

/* Mask register */
#define SPICTRL_MASK_TIPE_BIT   31
#define SPICTRL_MASK_LTE_BIT    14
#define SPICTRL_MASK_OVE_BIT    12
#define SPICTRL_MASK_UNE_BIT    11
#define SPICTRL_MASK_MMEE_BIT   10
#define SPICTRL_MASK_NEE_BIT    9
#define SPICTRL_MASK_NFE_BIT    8

#define SPICTRL_MASK_TIPE       (1 << SPICTRL_MASK_TIPE_BIT)
#define SPICTRL_MASK_LTE        (1 << SPICTRL_MASK_LTE_BIT)
#define SPICTRL_MASK_OVE        (1 << SPICTRL_MASK_OVE_BIT)
#define SPICTRL_MASK_UNE        (1 << SPICTRL_MASK_UNE_BIT)
#define SPICTRL_MASK_MMEE       (1 << SPICTRL_MASK_MMEE_BIT)
#define SPICTRL_MASK_NEE        (1 << SPICTRL_MASK_NEE_BIT)
#define SPICTRL_MASK_NFE        (1 << SPICTRL_MASK_NFE_BIT)

#endif

