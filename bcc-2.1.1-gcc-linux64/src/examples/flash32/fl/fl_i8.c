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
 * This file contains routines for programming Intel style flash devices
 * connected to an 8-bit data bus.
 */

#include <fl.h>
#include "fl_priv.h"

static int erase_sector(struct fl_ctx *ctx, uint32_t busaddr)
{
        DBG("entry\n");
        uint8_t status;

        flash_write8(ctx, busaddr, CFI_ERASE_SETUP);
        flash_write8(ctx, busaddr, CFI_CONFIRM);
        flash_write8(ctx, busaddr, CFI_STATUS);

        do {
                DBG("poll\n");
                status = flash_read8(ctx, busaddr);
        } while (0 == (status & CFI_STATUS_READY));

        flash_write8(ctx, busaddr, CFI_CLEAR_STATUS);
        flash_write8(ctx, ctx->dev.start, CFI_READ);

        return 0;
}

static int erase_chip(struct fl_ctx *ctx)
{
        UNUSED(ctx);
        return 1;
}

static int program_word(struct fl_ctx *ctx, uint32_t busaddr, uint32_t data)
{
        flash_write8(ctx, busaddr, CFI_CLEAR_STATUS);

        for (int i = 0; i < 4; i++) {
                uint8_t status;
                uint8_t wbyte;
                uint32_t waddr;

                data = ((data << 8) & 0xffffff00) | ((data >> 24) & 0xff);
                wbyte = data & 0xff;
                waddr = busaddr + i;

                flash_write8(ctx, waddr, CFI_WRITE);
                flash_write8(ctx, waddr, wbyte);

                do {
                        DBG("poll\n");
                        status = flash_read8(ctx, busaddr);
                } while (0 == (status & CFI_STATUS_READY));
        }

        return 0;
}

struct fl_ops fl_ops_i8 = {
        .erase_sector   = erase_sector,
        .erase_chip     = erase_chip,
        .program_word   = program_word,
};

