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

#ifndef FL_PRIV_H
#define FL_PRIV_H

#define UNUSED(v) ((void) sizeof (v))
#define NELEM(v) ((int) (sizeof (v) / sizeof (v[0])))

enum { FL_DEBUG = 0 };
#include <stdio.h>
#define DBG(...) if (FL_DEBUG) { \
        printf("%s:%d: ", __func__, __LINE__); \
        printf(__VA_ARGS__); \
}

struct fl_ops {
        int (*erase_sector)(struct fl_ctx *ctx, uint32_t busaddr);
        int (*erase_chip)(struct fl_ctx *ctx);
        int (*program_word)(struct fl_ctx *ctx, uint32_t busaddr, uint32_t data);
        /* Bits which indicate ready after erase sector/chip */
        uint32_t readymask;
        /* Bits which shall stop toggling at program complete */
        uint32_t togglemask;
        /* A TCB bus is present which shall be polled for ready or toggle */
        int xbus;
};

extern struct fl_ops fl_ops_i8;
extern struct fl_ops fl_ops_amd1x8;
extern struct fl_ops fl_ops_amd5x8;
extern struct fl_ops fl_ops_amd2x16;
extern struct fl_ops fl_ops_amd3x16;

struct fl_ctx {
        struct fl_ops           *ops;
        struct fl_device        dev;
        struct fl_cfi           cfi;
        int                     probed;
        int                     ba;
        /* Factor for flash address to bus address */
        int                     amul;
};

uint8_t flash_read8(struct fl_ctx *ctx, uint32_t addr);
void flash_write8(struct fl_ctx *ctx, uint32_t addr, uint8_t data);
uint32_t flash_read32(struct fl_ctx *ctx, uint32_t addr);
uint8_t flash_readx(struct fl_ctx *ctx, uint32_t addr);
void flash_writedata(struct fl_ctx *ctx, uint32_t addr, uint32_t data);

#endif

