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
 * connected to an 8-bit data bus.
 */

#include <fl.h>
#include "fl_priv.h"

#define TOGGLE_BIT 6
#define TOGGLE_MASK (1<<TOGGLE_BIT)

static int erase_sector(struct fl_ctx *ctx, uint32_t busaddr)
{
        DBG("entry\n");
        uint8_t status;
        uint32_t paddr = ctx->dev.start;
        const int amul = ctx->amul;

        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, paddr + 0xAAA * amul, 0x80);
        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, busaddr      , 0x30);

        do {
                DBG("poll\n");
                status = flash_read8(ctx, busaddr);
        } while (0 == (status & CFI_STATUS_READY));

        return 0;
}

static int erase_chip(struct fl_ctx *ctx)
{
        DBG("entry\n");
        uint8_t status;
        uint32_t paddr = ctx->dev.start;
        const int amul = ctx->amul;

        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, paddr + 0xAAA * amul, 0x80);
        flash_write8(ctx, paddr + 0xAAA * amul, 0xAA);
        flash_write8(ctx, paddr + 0x555 * amul, 0x55);
        flash_write8(ctx, paddr + 0xAAA * amul, 0x10);

        do {
                DBG("poll\n");
                status = flash_read8(ctx, paddr);
        } while (0 == (status & CFI_STATUS_READY));

        return 0;
}

static int program_word(struct fl_ctx *ctx, uint32_t busaddr, uint32_t data)
{
        DBG("entry\n");
        uint32_t paddr = ctx->dev.start;

        for (int i = 0; i < 4; i++) {
                uint8_t toggle;
                uint8_t wbyte;
                uint32_t waddr;

                data = ((data << 8) & 0xffffff00) | ((data >> 24) & 0xff);
                wbyte = data & 0xff;
                waddr = busaddr + i;
                flash_write8(ctx, paddr + 0xAAA, 0xAA);
                flash_write8(ctx, paddr + 0x555, 0x55);
                flash_write8(ctx, paddr + 0xAAA, 0xA0);
                flash_write8(ctx, waddr        , wbyte);

                /* NOTE: toggle can be read from any address */
                toggle = flash_read8(ctx, waddr);
                do {
                        DBG("poll\n");
                        toggle ^= flash_read8(ctx, waddr);
                } while (toggle & TOGGLE_MASK);
        }

        return 0;
}

struct fl_ops fl_ops_amd1x8 = {
        .erase_sector   = erase_sector,
        .erase_chip     = erase_chip,
        .program_word   = program_word,
};

