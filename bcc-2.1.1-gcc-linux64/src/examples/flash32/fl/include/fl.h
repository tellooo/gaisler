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

#ifndef FL_H
#define FL_H

#include <stdint.h>

enum {
        CFI_ERASE_SETUP         = 0x20,
        CFI_WRITE               = 0x40,
        CFI_CLEAR_STATUS        = 0x50,
        CFI_STATUS              = 0x70,
        CFI_CONFIG              = 0x90,
        CFI_QUERY               = 0x98,
        CFI_CONFIRM             = 0xd0,
        CFI_READ                = 0xff,
        CFI_READ_AMD            = 0xf0,
};

enum {
        CFI_STATUS_READY       = 0x80, // bit 7 of flash status register
        CFI_STATUS_SUSP_ERROR  = 0x40, // bit 6 of flash status register
        CFI_STATUS_ERASE_ERROR = 0x20, // bit 5 of flash status register
        CFI_STATUS_PROG_ERROR  = 0x10, // bit 4 of flash status register
        CFI_STATUS_VPP_ERROR   = 0x08, // bit 3 of flash status register
        CFI_STATUS_SUSP_STATUS = 0x04, // bit 2 of flash status register
        CFI_STATUS_LOCK_ERROR  = 0x02, // bit 1 of flash status register
};

enum {
        CFI_FAMILY_INTEL        = 1,
        CFI_FAMILY_AMD          = 2,
};

enum fl_width {
        FL_WIDTH_1x8,
        FL_WIDTH_5x8,
        FL_WIDTH_2x16,
        FL_WIDTH_3x16,
};

struct fl_device {
        uint32_t        start;
        enum fl_width   width;
        void            *user;
        uint8_t         (*read8)(void *user, uint32_t addr);
        void            (*write8)(void *user, uint32_t addr, uint8_t data);
        /* Read data with EDAC disabled */
        uint32_t        (*read32)(void *user, uint32_t addr);
        /* Read the TCB corresponding with addr */
        uint8_t         (*readx)(void *user, uint32_t addr);
        /* Write a 32-bit user value and let hardware calculate TCB. */
        void            (*writedata)(void *user, uint32_t addr, uint32_t data);
};

struct fl_cfi {
        uint16_t query[0x60];
        int family;
        int manuf_id;
        int device_id;
        int size;
        int regions;
        int blocks;
        struct cfi_region {
                int blocks;
                int size;
        } region[4];
        /* Start address of each block. */
        uint32_t block[16384];
};

struct fl_ctx;

/*
 * Open an fl_ instance given the information in dev.
 *
 * dev is copied internally at open and can be deallocated by the user at
 * return.
 *
 * The function fl_probe() must be called before operating on the device.
 *
 * return: handle to use with all other fl_functions.
 */
struct fl_ctx *fl_open(const struct fl_device *dev);

/*
 * Close fl_ instance.
 */
void fl_close(struct fl_ctx *ctx);

/*
 * Probe CFI and other flash memory information.
 *
 * This function must be called after fl_open() and before any other fl_
 * function.
 */
int fl_probe(struct fl_ctx *ctx);

/*
 * Copy probed CFI information to the user.
 */
int fl_get_cfi(struct fl_ctx *ctx, struct fl_cfi *cfi);

/* Erase the sector containing the bus address addr. */
int fl_erase_sector(struct fl_ctx *ctx, uint32_t busaddr);

/* Erase the entire area covered by the chip(s) */
int fl_erase_chip(struct fl_ctx *ctx);

/*
 * Program a 32-bit word to given address
 *
 * At return of this function, the value at location busaddr has the value
 * data. The location shall not have been programmed before the call.
 */
int fl_program_word(struct fl_ctx *ctx, uint32_t busaddr, uint32_t data);

/*
 * Get first CPU address for a given flash block number
 *
 * return: -1 if invalid block, else first CPU address of block
 */
uint32_t fl_firstaddr(struct fl_ctx *ctx, int block);

/*
 * Get last CPU address for a given flash block number
 *
 * return: -1 if invalid block, else last CPU address of block
 */
uint32_t fl_lastaddr(struct fl_ctx *ctx, int block);

char *fl_family_to_str(int family);

#endif

