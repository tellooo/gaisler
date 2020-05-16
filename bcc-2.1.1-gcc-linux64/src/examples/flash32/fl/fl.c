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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fl.h>
#include "fl_priv.h"

uint8_t flash_read8(struct fl_ctx *ctx, uint32_t addr)
{
        uint8_t ret;
        ret = ctx->dev.read8(ctx->dev.user, addr);
        DBG(" 0x%08x -> 0x%02x\n", (unsigned) addr, (unsigned) ret);
        return ret;
}

void flash_write8(struct fl_ctx *ctx, uint32_t addr, uint8_t data)
{
        ctx->dev.write8(ctx->dev.user, addr, data);
        DBG("0x%08x <- 0x%02x\n", (unsigned) addr, (unsigned) data);
}

uint8_t flash_readx(struct fl_ctx *ctx, uint32_t addr)
{
        uint8_t ret;
        ret = ctx->dev.readx(ctx->dev.user, addr);
        DBG(" 0x%08x -> 0x%02x\n", (unsigned) addr, (unsigned) ret);
        return ret;
}

void flash_writedata(struct fl_ctx *ctx, uint32_t addr, uint32_t data)
{
        ctx->dev.writedata(ctx->dev.user, addr, data);
        DBG("0x%08x <- 0x%08x\n", (unsigned) addr, (unsigned) data);
}

uint32_t flash_read32(struct fl_ctx *ctx, uint32_t addr)
{
        uint32_t ret;
        ret = ctx->dev.read32(ctx->dev.user, addr);
        DBG(" 0x%08x -> 0x%08x\n", (unsigned) addr, (unsigned) ret);
        return ret;
}

static uint8_t def_read8(void *user, uint32_t addr)
{
        UNUSED(user);
        uint8_t tmp;
        __asm__ (" lduba [%1]1, %0 "
                : "=r" (tmp)
                : "r" (addr)
        );

        return tmp;
}

static void def_write8(void *user, uint32_t addr, uint8_t data)
{
        UNUSED(user);
        uint8_t *dst = (uint8_t *) addr;
        *dst = data;
}

/* Used to write a command to all flash devices */
static void def_write8_32(void *user, uint32_t addr, uint8_t data)
{
        UNUSED(user);
        uint32_t data32 = data<<24 | data<<16 | data<<8 | data;
        addr &= (~3);
        uint8_t *dst = (uint8_t *) addr;
        *dst = data32;
}

static uint8_t def_read8_32(void *user, uint32_t addr)
{
        UNUSED(user);
        uint32_t data;
        uint32_t addra = addr & (~3);

        __asm__ (" lda [%1]1, %0 "
                : "=r" (data)
                : "r" (addra)
        );
        DBG("addr=%08lx data=%08x (from 4 lanes)\n", addr & (~3), (unsigned) data);
        if ((addr & 3) == 3) { return (data >> 24) & 0xff; }
        if ((addr & 3) == 2) { return (data >> 16) & 0xff; }
        if ((addr & 3) == 1) { return (data >>  8) & 0xff; }
        return data & 0xff;
}

static uint32_t def_read32(void *user, uint32_t addr)
{
        UNUSED(user);
        uint32_t data;

        __asm__ (" lda [%1]1, %0 "
                : "=r" (data)
                : "r" (addr)
        );
        DBG("addr=%08lx data=%08x\n", addr, (unsigned) data);
        return data;
}

static void def_writedata(void *user, uint32_t addr, uint32_t data)
{
        UNUSED(user);
        uint32_t *dst = (uint32_t *) addr;
        *dst = data;
}

static int query8(struct fl_ctx *ctx) {
        DBG("entry\n");
        uint32_t paddr = ctx->dev.start;
        struct fl_cfi *cfi = &ctx->cfi;

        flash_write8(ctx, paddr + 0x55 * ctx->ba, CFI_QUERY);
        for (int i = 0; i < NELEM(cfi->query); i++) {
                cfi->query[i] = flash_read8(ctx, paddr + i * ctx->ba);
        }
        flash_write8(ctx, paddr + 0x55 * ctx->ba, CFI_READ);

        DBG(" -> 0x10: %c, 0x11: %c, 0x12: %c\n", cfi->query[0x10], cfi->query[0x11], cfi->query[0x12]);

        if (
                cfi->query[0x10] == 'Q' &&
                cfi->query[0x11] == 'R' &&
                cfi->query[0x12] == 'Y'
        ) {
                ctx->probed = 1;
                return 0;
        }
        DBG("no QRY\n");

        return 1;
}

static int parse_query(struct fl_ctx *ctx)
{
        DBG("entry\n");
        struct fl_cfi *cfi = &ctx->cfi;
        uint16_t *raw = &cfi->query[0];

        cfi->family = 256 * raw[0x14] + raw[0x13];
        DBG("family:    %d (%s)\n", cfi->family, fl_family_to_str(cfi->family));
        cfi->size = 1 << raw[0x27];
        DBG("size:      0x%x (%d Mbit)\n", cfi->size, cfi->size / 1024 / 1024 * 8);
        cfi->regions = raw[0x2c];
        DBG("regions:   %d erase block regions\n", cfi->regions);

        uint32_t lastaddr = 0;
        cfi->blocks = 0;
        for (int i=0; i < cfi->regions && i < 4; i++) {
                int qoffs = 0x2d + i * 4;
                DBG("region %d\n", i + 1);
                cfi->region[i].blocks   = 256 * raw[qoffs + 1] + raw[qoffs] + 1;
                cfi->region[i].size     = 256 * (256 * raw[qoffs + 3] + raw[qoffs + 2]);
                DBG("  blocks:  %d\n", cfi->region[i].blocks);
                DBG("  size:    %d KiB\n", cfi->region[i].size / 1024);

                int j;
                int limit = cfi->blocks + cfi->region[i].blocks;
                for (j = cfi->blocks; j < limit && j < 2048; j++) {
                        //DBG("  block 0x%03x startaddr %08x\n", j, (unsigned) lastaddr);
                        cfi->block[j] = lastaddr;
                        lastaddr += cfi->region[i].size;
                }
                cfi->blocks += cfi->region[i].blocks;
        }

        cfi->manuf_id   = -1;
        cfi->device_id  = -1;
        if (cfi->family == 1) {
                /* Intel */
                flash_write8(ctx, ctx->dev.start, CFI_CONFIG);
                cfi->manuf_id = (
                        flash_read8(ctx, ctx->dev.start + 0 * ctx->ba)
                );
                cfi->device_id = (
                        flash_read8(ctx, ctx->dev.start + 1 * ctx->ba)
                );
                flash_write8(ctx, ctx->dev.start, CFI_READ);
        } else if (cfi->family == 2) {
                /* AMD */
                /* probing of  manu_id and device_id for AMD goes here... */
        } else {
                DBG("unknown family, no manufacturer id or device id probed\n");
        }
        DBG("manuf:     %3d (0x%04x)\n", cfi->manuf_id, cfi->manuf_id);
        DBG("device:    %3d (0x%04x)\n", cfi->device_id, cfi->device_id);

        return 0;
}

int fl_probe(struct fl_ctx *ctx)
{
        int ret;

        ret = 1;
        if (ctx->dev.width == FL_WIDTH_1x8) {
                ctx->ba = 2;
                ctx->amul = 1;
                ret = query8(ctx);
                if (ret) { return ret; }
        } else if (ctx->dev.width == FL_WIDTH_5x8) {
                ctx->ba = 8;
                ctx->amul = 4;
                ret = query8(ctx);
                if (ret) { return ret; }
        } else if (ctx->dev.width == FL_WIDTH_2x16) {
                ctx->ba = 4;
                ctx->amul = 2;
                ret = query8(ctx);
                if (ret) { return ret; }
        } else if (ctx->dev.width == FL_WIDTH_3x16) {
                ctx->ba = 4;
                ctx->amul = 2;
                ret = query8(ctx);
                if (ret) { return ret; }
        } else {
                return 1;
        }

        /*
         * Fill in cfi->region
         */
        ret = parse_query(ctx);
        if (ret) {
                return ret;
        }
        ret = 1;
        if (ctx->cfi.family == CFI_FAMILY_INTEL) {
                if (ctx->dev.width == FL_WIDTH_1x8) {
                        ctx->ops = &fl_ops_i8;
                        ret = 0;
                }
        } else if (ctx->cfi.family == CFI_FAMILY_AMD) {
                if (ctx->dev.width == FL_WIDTH_1x8) {
                        ctx->ops = &fl_ops_amd1x8;
                        ret = 0;
                } else if (ctx->dev.width == FL_WIDTH_5x8) {
                        ctx->ops = &fl_ops_amd5x8;
                        ret = 0;
                } else if (ctx->dev.width == FL_WIDTH_2x16) {
                        ctx->ops = &fl_ops_amd2x16;
                        ret = 0;
                } else if (ctx->dev.width == FL_WIDTH_3x16) {
                        ctx->ops = &fl_ops_amd3x16;
                        ret = 0;
                }
        }
        if (ret) {
                DBG("no ops for cfi.family=%d and width=%d\n", ctx->cfi.family, ctx->dev.width);
                ret = 1;
        }

        return ret;
}

int fl_get_cfi(struct fl_ctx *ctx, struct fl_cfi *cfi)
{
        if (!ctx->probed) {
                return 1;
        }
        *cfi = ctx->cfi;
        return 0;
}

int fl_erase_sector(struct fl_ctx *ctx, uint32_t busaddr)
{
        if (!ctx->probed) {
                return 1;
        }
        return ctx->ops->erase_sector(ctx, busaddr);
}

int fl_erase_chip(struct fl_ctx *ctx)
{
        if (!ctx->probed) {
                return 1;
        }
        return ctx->ops->erase_chip(ctx);
}

int fl_program_word(struct fl_ctx *ctx, uint32_t busaddr, uint32_t data)
{
        if (!ctx->probed) {
                return 1;
        }
        return ctx->ops->program_word(ctx, busaddr, data);
}


char *fl_family_to_str(int family) {
        if (1 == family) {
                return "Intel";
        } else if (2 == family) {
                return "AMD";
        }
        return "UNKNOWN";
}

void fl_init(struct fl_ctx *ctx, const struct fl_device *dev)
{
        DBG("entry\n");
        memset(ctx, 0, sizeof *ctx);
        ctx->dev = *dev;
        if (ctx->dev.width == FL_WIDTH_1x8) {
                if (NULL == ctx->dev.read8) {
                        ctx->dev.read8 = def_read8;
                }
                if (NULL == ctx->dev.write8) {
                        ctx->dev.write8 = def_write8;
                }
        } else if (ctx->dev.width == FL_WIDTH_5x8) {
                ;
        } else if (ctx->dev.width == FL_WIDTH_2x16) {
                if (NULL == ctx->dev.read8) {
                        ctx->dev.read8 = def_read8_32;
                }
                if (NULL == ctx->dev.write8) {
                        ctx->dev.write8 = def_write8_32;
                }
                if (NULL == ctx->dev.read32) {
                        ctx->dev.read32 = def_read32;
                }
                if (NULL == ctx->dev.writedata) {
                        ctx->dev.writedata = def_writedata;
                }
        } else if (ctx->dev.width == FL_WIDTH_3x16) {
                ;
        } else {
                ;
        }
}

struct fl_ctx *fl_open(const struct fl_device *dev)
{
        struct fl_ctx *ctx;

        DBG("entry\n");
        ctx = malloc(sizeof *ctx);
        if (ctx == NULL) {
                return NULL;
        }
        fl_init(ctx, dev);

        return ctx;
}

void fl_close(struct fl_ctx *ctx)
{
        DBG("entry\n");
        free(ctx);
}

uint32_t fl_firstaddr(struct fl_ctx *ctx, int block)
{
        const uint32_t nope = 0xffffffff;
        uint32_t busaddr;

        if (!ctx->probed) {
                return nope;
        }
        if (block < 0 || ctx->cfi.blocks <= block) {
                return nope;
        }

        busaddr = ctx->dev.start + ctx->cfi.block[block] * ctx->amul;

        return busaddr;
}

uint32_t fl_lastaddr(struct fl_ctx *ctx, int block)
{
        const uint32_t nope = 0xffffffff;
        uint32_t busaddr;

        if (!ctx->probed) {
                return nope;
        }
        if (block < 0 || ctx->cfi.blocks <= block) {
                return nope;
        }

        busaddr = ctx->dev.start + ctx->cfi.block[block+1] * ctx->amul -1;

        return busaddr;
}

