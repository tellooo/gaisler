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

/*
 * This file contains routines for programming AMD style flash devices
 * connected to a 32-bit data bus with an optional 8-bit EDAC checkbit bus.
 *
 * Three different configurations are supported as described by the fl_ops
 * records at the end of this file.
 *
 * Commands are written to all devices in parallel. The devices are then
 * polled individually for ready or toggle status.
 */

#include <fl.h>
#include "fl_priv.h"

#define TOGGLE_BIT 6
#define TOGGLE_MASK (1<<TOGGLE_BIT)

#define STATUS_ALLREADY_16 ((uint32_t) \
        CFI_STATUS_READY << 16 | \
        CFI_STATUS_READY <<  0 | \
        0 \
)

#define STATUS_ALLREADY_8 ((uint32_t) \
        CFI_STATUS_READY << 24 | \
        CFI_STATUS_READY << 16 | \
        CFI_STATUS_READY <<  8 | \
        CFI_STATUS_READY <<  0 | \
        0 \
)

#define TOGGLE32_MASK_16 ((uint32_t) \
        TOGGLE_MASK << 16 | \
        TOGGLE_MASK <<  0 | \
        0 \
)

#define TOGGLE32_MASK_8 ((uint32_t) \
        TOGGLE_MASK << 24 | \
        TOGGLE_MASK << 16 | \
        TOGGLE_MASK <<  8 | \
        TOGGLE_MASK <<  0 | \
        0 \
)

static int waitready(struct fl_ctx *ctx, uint32_t busaddr)
{
        uint32_t status;
        const uint32_t readymask = ctx->ops->readymask;

        if (ctx->ops->xbus) {
                DBG("poll tcb[7..0]\n");
                do {
                        status = flash_readx(ctx, busaddr);
                } while (CFI_STATUS_READY != (status & CFI_STATUS_READY));
        }

        DBG("poll d[31..0]\n");
        do {
                status = flash_read32(ctx, busaddr);
        } while (readymask != (status & readymask));

        return 0;
}

/* NOTE: toggle can be read from any address */
static int waittoggle(struct fl_ctx *ctx, uint32_t busaddr)
{
        uint32_t toggle;
        const uint32_t togglemask = ctx->ops->togglemask;

        if (ctx->ops->xbus) {
                DBG("toggle-poll tcb[7..0]\n");
                toggle = flash_readx(ctx, busaddr);
                do {
                        toggle ^= flash_readx(ctx, busaddr);
                } while (toggle & TOGGLE_MASK);
        }

        DBG("toggle-poll d[31..0]\n");
        toggle = flash_read32(ctx, busaddr);
        do {
                toggle ^= flash_read32(ctx, busaddr);
        } while (toggle & togglemask);

        return 0;
}

static int erase_sector(struct fl_ctx *ctx, uint32_t busaddr)
{
        DBG("entry\n");
        uint32_t paddr = ctx->dev.start;
        const int amul = ctx->amul;

        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, paddr + 0xAAA * amul, 0x80);
        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, busaddr      , 0x30);
        waitready(ctx, busaddr);

        return 0;
}

static int erase_chip(struct fl_ctx *ctx)
{
        DBG("entry\n");
        uint32_t paddr = ctx->dev.start;
        const int amul = ctx->amul;

        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, paddr + 0xAAA * amul, 0x80);
        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, paddr + 0xAAA * amul, 0x10);
        waitready(ctx, paddr);

        return 0;
}

static int program_word(struct fl_ctx *ctx, uint32_t busaddr, uint32_t data)
{
        DBG("entry\n");
        uint32_t paddr = ctx->dev.start;
        const int amul = ctx->amul;

        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, paddr + 0xAAA * amul, 0xA0);
        flash_writedata(ctx, busaddr          , data);
        waittoggle(ctx, busaddr);

        return 0;
}

struct fl_ops fl_ops_amd5x8 = {
        .erase_sector   = erase_sector,
        .erase_chip     = erase_chip,
        .program_word   = program_word,
        .readymask      = STATUS_ALLREADY_8,
        .togglemask     = TOGGLE32_MASK_8,
        .xbus           = 1,
};

struct fl_ops fl_ops_amd3x16 = {
        .erase_sector   = erase_sector,
        .erase_chip     = erase_chip,
        .program_word   = program_word,
        .readymask      = STATUS_ALLREADY_16,
        .togglemask     = TOGGLE32_MASK_16,
        .xbus           = 1,
};

struct fl_ops fl_ops_amd2x16 = {
        .erase_sector   = erase_sector,
        .erase_chip     = erase_chip,
        .program_word   = program_word,
        .readymask      = STATUS_ALLREADY_16,
        .togglemask     = TOGGLE32_MASK_16,
        .xbus           = 0,
};

