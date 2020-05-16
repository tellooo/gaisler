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

#ifndef DRV_MEMSCRUB_REGS_H
#define DRV_MEMSCRUB_REGS_H

#include <stdint.h>

/* MEMORYSCRUBBER Registers layout */
struct memscrub_regs {
        uint32_t ahbstatus; /* 0x00 */
        uint32_t ahbfailing; /* 0x04 */
        uint32_t ahberc; /* 0x08 */
        uint32_t resv1; /* 0x0c */
        uint32_t status; /* 0x10 */
        uint32_t config; /* 0x14 */
        uint32_t rangel; /* 0x18 */
        uint32_t rangeh; /* 0x1c */
        uint32_t pos; /* 0x20 */
        uint32_t ethres; /* 0x24 */
        uint32_t init; /* 0x28 */
        uint32_t rangel2; /* 0x2c */
        uint32_t rangeh2; /* 0x30 */
};

#endif

