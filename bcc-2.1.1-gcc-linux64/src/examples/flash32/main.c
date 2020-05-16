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
#include <stdio.h>
#include <stdlib.h>
#include <bcc/bcc.h>
#include <mbdrv.h>
#include <ftmctrl-regs.h>
#include <fl.h>
#include "dbg.h"

enum exdev_t {
        EXDEV_1x8,      /*  8 bit data, no EDAC */
        EXDEV_2x16,     /* 32 bit data, no EDAC */
        EXDEV_5x8,      /* 32 bit data, 8 bit EDAC */
        EXDEV_3x16,     /* 32 bit data, 8 bit EDAC */
        EXDEV_NUM,
};

/*
 * ### USER CONFIGURATION BEGIN
 *
 * Set the MYEXDEV variable to match the PROM bank connection.
 */
//const enum exdev_t MYEXDEV = EXDEV_1x8;
//const enum exdev_t MYEXDEV = EXDEV_2x16;
//const enum exdev_t MYEXDEV = EXDEV_5x8;
const enum exdev_t MYEXDEV = EXDEV_3x16;

/* ERASE_CHIP: 1 - erase chip, 0 - do not erase chip */
const int ERASE_CHIP = 0;
//const int ERASE_CHIP = 1;

/* A user selected sector which the example optionally erases and programs. */
const int MYSECTOR = 19;


/* ERASE_MYSECTOR: 1 - erase MYSECTOR, 0 - do not erase MYSECTOR */
const int ERASE_MYSECTOR = 1;
//const int ERASE_MYSECTOR = 0;

/* PROGRAM_COUNT: Number of words to program and read back in MYSECTOR. */
const int PROGRAM_COUNT = 13;
//const int PROGRAM_COUNT = 0;

/* ### USER CONFIGURATION END */


/*
 * Initialize a fl_device structure and hardware, based on bus configuration in
 * exdev_t.
 *
 * The return value is suitable as a device/access configuration for fl_open().
 */
struct fl_device *preparedev(
        enum exdev_t what
);

void print_cfi(struct fl_cfi *cfi);

struct fl_cfi mycfi;
int main(void)
{
        int ret;
        struct fl_device *dev;
        struct fl_ctx *fl;
        uint32_t a;
        int b;

        printf("---- EXAMPLE BEGIN ----\n");
        dev = preparedev(MYEXDEV);
        assert(dev);

        fl = fl_open(dev);
        assert(fl);

        ret = fl_probe(fl);
        assert(0 == ret);

        ret = fl_get_cfi(fl, &mycfi);
        /*
         * The mycfi struct now contains information on the flash device which
         * is probed by fl_probe(). See fl.h for details.
         */

        puts("CFI info for one device:");
        print_cfi(&mycfi);
        puts("");

        /* Do some operations. */

        ret = 0;
        if (ERASE_CHIP) {
                printf("Erase chip ...\n");
                ret = fl_erase_chip(fl);
                assert(0 == ret);
        }

        /* pick a sector. mycfi.blocks is the total number of sectors in the device */
        if (MYSECTOR < 0) {
                b = mycfi.blocks - MYSECTOR;
        } else {
                b = MYSECTOR;
        }
        /* and determine which bus address it starts at. */
        a = fl_firstaddr(fl, b);
        if (0xffffffff == a) {
                printf("invalid sector: %d\n", b);
                exit(1);
        }

        if (ERASE_MYSECTOR) {
                printf("Erasing the sector %d at address %08x ...\n", b, (unsigned) a);
                ret = fl_erase_sector(fl, a);
                assert(0 == ret);
        }

        if (PROGRAM_COUNT) {
                for (int i = 0; i < (PROGRAM_COUNT * 4); i = i + 4) {
                        printf("Programming data at address 0x%08x ...\n", (unsigned) a + i);
                        ret = fl_program_word(fl, a + i, i);
                        assert(0 == ret);
                }

                printf("Reading back the programmed data ...\n");
                for (int i = 0; i < (PROGRAM_COUNT * 4); i = i + 4) {
                        uintptr_t t;
                        t = a + i;
                        printf("0x%08x: %08x\n", t, (unsigned) *((uint32_t *) t));
                }

                /*
                 * HINT: You can also use the mbdrv function
                 * mbdrv_read32plus8() to verify data+checkbit consistency.
                 */
        }

        fl_close(fl);

        printf("---- EXAMPLE END ----\n");

        return 0;
}

void print_cfi(struct fl_cfi *cfi)
{
        printf("family:    %d (%s)\n", cfi->family, fl_family_to_str(cfi->family));
        printf("size:      %d Mbit\n", cfi->size / 1024 / 1024 * 8);
        printf("regions:   %d erase block regions\n", cfi->regions);
        if (cfi->manuf_id != -1) {
                printf("manuf:     %3d (0x%04x)\n", cfi->manuf_id, cfi->manuf_id);
        }
        if (cfi->device_id != -1) {
                printf("device:    %3d (0x%04x)\n", cfi->device_id, cfi->device_id);
        }
        int rfirst = 0;
        for (int i=0; i < cfi->regions && i < 4; i++) {
                printf("region %d\n", i + 1);
                printf("  blocks:  %d\n", cfi->region[i].blocks);
                printf("  size:    %d KiB\n", cfi->region[i].size / 1024);
                if (!"bogus") {
                printf(
                        "  addr:    %08x .. %08x (at device)\n",
                        (unsigned) cfi->block[rfirst],
                        (unsigned) cfi->block[rfirst] + cfi->region[i].blocks * cfi->region[i].size - 1
                );
                }
                rfirst += cfi->region[i].blocks;
        }
}

/*
 * Connection of the fl library with the the mbdrv interface
 *
 * The ex_ functions below are used by fl_ library for special access to PROM
 * EDAC bus.
 */

/* Used by fl_ to read CFI bytes (query) */
static uint8_t ex_read8(void *user, uint32_t addr)
{
        struct mbdrv_ctx *ctx = user;
        uint32_t data;

        mbdrv_read32(ctx, addr & (~3), &data);
        DBG("addr=%08lx data=%08x (from 4 lanes)\n", addr & (~3), (unsigned) data);
        if ((addr & 3) == 3) { return (data >> 24) & 0xff; }
        if ((addr & 3) == 2) { return (data >> 16) & 0xff; }
        if ((addr & 3) == 1) { return (data >>  8) & 0xff; }
        return data & 0xff;
}

/* Used by fl_ to write a command to all flash devices */
static void ex_write8(void *user, uint32_t addr, uint8_t data)
{
        struct mbdrv_ctx *ctx = user;
        uint32_t data32 = data<<24 | data<<16 | data<<8 | data;
        addr &= (~3);
        mbdrv_write32plus8(ctx, addr & (~3), data32, data);
        DBG("addr=%08lx data=%08x (to 5 lanes)\n", addr, data);
}

/*
 * Used by fl_ to read data bus without EDAC correction. Important for polling
 * status of flash devices on data lanes.
 */
static uint32_t ex_read32(void *user, uint32_t addr)
{
        struct mbdrv_ctx *ctx = user;
        uint32_t data;
        mbdrv_read32(ctx, addr, &data);
        return data;
}

/* Used by fl_ to read from checkbit flash. For example flash status polling. */
static uint8_t ex_readx(void *user, uint32_t addr)
{
        struct mbdrv_ctx *ctx = user;
        uint32_t data;
        uint32_t tcb;
        int trap;

        mbdrv_read32plus8(ctx, addr, &data, &tcb, &trap);
        DBG("addr=%08lx data=%08lx, tcb=%08lx, trap=%d\n", addr, data, tcb, trap);

        return tcb & 0xff;
}

/* Used by fl_ to write 32-bit data plus corresponding checkbits */
static void ex_writedata(void *user, uint32_t addr, uint32_t data)
{
        struct mbdrv_ctx *ctx = user;
        mbdrv_write32data(ctx, addr, data);
        DBG("addr=%08lx data=%08lx\n", addr, data);
}

int mctrl_pwen(void *arg, int onoff)
{
        (void) sizeof arg;
        volatile struct ftmctrl_regs *mctrl = (void *) 0x80000000;
        if (onoff) {
                mctrl->mcfg1 |= FTMCTRL_MCFG1_PWEN;
        } else {
                mctrl->mcfg1 &= ~FTMCTRL_MCFG1_PWEN;
        }
        return 0;
}

int mctrl_pe(void *arg, int onoff)
{
        (void) sizeof arg;
        volatile struct ftmctrl_regs *mctrl = (void *) 0x80000000;
        if (onoff) {
                mctrl->mcfg3 |= FTMCTRL_MCFG3_PE;
        } else {
                mctrl->mcfg3 &= ~FTMCTRL_MCFG3_PE;
        }
        return 0;
}

/*
 * The fl_device records are used to describe
 *   - and the flash memory configuration on the bus
 *   - routines for accessing the memory bus the flash is located on
 */
struct fl_device fl_dev1x8 =
{
        .start          = 0,
        .width          = FL_WIDTH_1x8,
};

struct fl_device fl_dev2x16 =
{
        .start          = 0,
        .width          = FL_WIDTH_2x16,
};

struct fl_device fl_dev5x8 =
{
        .start          = 0,
        .width          = FL_WIDTH_5x8,
        .read8          = ex_read8,
        .write8         = ex_write8,
        .read32         = ex_read32,
        .readx          = ex_readx,
        .writedata      = ex_writedata,
};

struct fl_device fl_dev3x16 =
{
        .start          = 0,
        .width          = FL_WIDTH_3x16,
        .read8          = ex_read8,
        .write8         = ex_write8,
        .read32         = ex_read32,
        .readx          = ex_readx,
        .writedata      = ex_writedata,
};

/* Data exported by the "safe" execution environment. */
struct safe_info {
        void (*trap_soft)(void);
        struct mbdrv_ops *ops;
};

static struct safe_info **sinfo = (struct safe_info **) 0xa000000c;

/*
 * Initialize the isolated environment needed for accessing the FTMCTRL
 * checkbit bus. This function is called when flash is configured with a
 * checkbit bus.
 *
 * - Install software trap 255 with a routine in the on-chip RAM.
 * - Functions for the mbdrv interface also come from on-chip RAM.
 * - Enable PROM EDAC
 */
void init_mbpromedac(struct mbdrv_ctx *ctx)
{
        ctx->mctrl = (void *) 0x80000000;
        bcc_set_trap(255, (*sinfo)->trap_soft);
        ctx->ops = (*sinfo)->ops;
        bcc_flush_icache();
        mctrl_pe(NULL, 1);
}

struct mbdrv_ctx mbdrv_promedac;
struct fl_device *preparedev(enum exdev_t what)
{
        if (what < 0) { return NULL; }

        struct fl_device *dev = NULL;
        struct mbdrv_ctx *mbdrv = &mbdrv_promedac;

        mctrl_pe(NULL, 0);
        mctrl_pwen(NULL, 1);
        if (what == EXDEV_1x8) {
                dev = &fl_dev1x8;

        } else if (what == EXDEV_2x16) {
                dev = &fl_dev2x16;

        } else if (what == EXDEV_5x8) {
                dev = &fl_dev5x8;
                init_mbpromedac(mbdrv);
                dev->user = mbdrv;

        } else if (what == EXDEV_3x16) {
                dev = &fl_dev3x16;
                init_mbpromedac(mbdrv);
                dev->user = mbdrv;
        }

        return dev;
}

