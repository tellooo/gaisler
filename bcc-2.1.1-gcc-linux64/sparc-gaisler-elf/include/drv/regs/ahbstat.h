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

#ifndef DRV_AHBSTAT_REGS_H
#define DRV_AHBSTAT_REGS_H

#include <stdint.h>

struct ahbstat_regs {
        uint32_t status;
        uint32_t failing;
        uint32_t status2;
        uint32_t failing2;
};

#define AHBSTAT_STS_ME_BIT 13
#define AHBSTAT_STS_FW_BIT 12
#define AHBSTAT_STS_CF_BIT 11
#define AHBSTAT_STS_AF_BIT 10
#define AHBSTAT_STS_CE_BIT 9
#define AHBSTAT_STS_NE_BIT 8
#define AHBSTAT_STS_HW_BIT 7
#define AHBSTAT_STS_HM_BIT 3
#define AHBSTAT_STS_HS_BIT 0

#define AHBSTAT_STS_ME (1 << AHBSTAT_STS_ME_BIT)
#define AHBSTAT_STS_FW (1 << AHBSTAT_STS_FW_BIT)
#define AHBSTAT_STS_CF (1 << AHBSTAT_STS_CF_BIT)
#define AHBSTAT_STS_AF (1 << AHBSTAT_STS_AF_BIT)
#define AHBSTAT_STS_CE (1 << AHBSTAT_STS_CE_BIT)
#define AHBSTAT_STS_NE (1 << AHBSTAT_STS_NE_BIT)
#define AHBSTAT_STS_HW (1 << AHBSTAT_STS_HW_BIT)
#define AHBSTAT_STS_HM (0xf << AHBSTAT_STS_HM_BIT)
#define AHBSTAT_STS_HS (0x7 << AHBSTAT_STS_HS_BIT)

#endif

