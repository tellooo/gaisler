/*
 * Copyright (c) 2017, Cobham Gaisler AB
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

#ifndef __BCC_AMBAPP_H_
#define __BCC_AMBAPP_H_

#include <stdint.h>
#include <stdlib.h>
#include "ambapp_ids.h"

#define AMBA_AHB_NMASTERS               64
#define AMBA_AHB_NSLAVES                64
#define AMBA_AHB_NBARS                  4

#define AMBA_APB_NSLAVES                16

#define AMBAPP_VISIT_AHBMASTER  0x0001
#define AMBAPP_VISIT_AHBSLAVE   0x0002
#define AMBAPP_VISIT_APBSLAVE   0x0004

struct ambapp_ahb_entry {
        uint32_t id;
        uint32_t user[3];
        uint32_t bar[AMBA_AHB_NBARS];
};

struct ambapp_apb_entry {
        uint32_t id;
        uint32_t bar;
};

/* BAR = Bank Address Register */
struct amba_ahb_bar {
        uint32_t start;
        uint32_t mask;
        uint8_t type;
};

struct amba_ahb_info {
        uint8_t ver;
        uint8_t irq;
        struct amba_ahb_bar bar[AMBA_AHB_NBARS];
        /* Reference to the AHB Plug&Play configuration record */
        const struct ambapp_ahb_entry *entry;
};

struct amba_apb_info {
        uint8_t ver;
        uint8_t irq;
        uint32_t start;
        uint32_t mask;
        /* Reference to the APB Plug&Play configuration record */
        const struct ambapp_apb_entry *entry;
};

/*
 * Visit each AMBA Plug&Play device
 *
 * A recursive AMBA Plug&Play scanning is performed, depth first. Information
 * records are filled in and supplied to a user function on a user criteria
 * match.
 *
 * When the user function returns non-zero, then the device scanning is aborted
 * and ambapp_visit() returns the return value of the user function.
 *
 * The ambapp_visit() function does not allocate dynamic or static memory: all
 * state is on the stack.
 *
 * Example use cases for ambapp_visit():
 * - Count number of devices/buses
 * - Build a device tree
 * - Find a specific device given a criteria.
 *
 * Parameters:
 * - ioarea:    IO area of bus to start device scanning
 * - vendor:    Vendor ID to visit, or 0 for all vendors IDs
 * - device:    Device ID to visit, or 0 for all device IDs
 * - flags:     Mask of device types to visit (AMBAPP_VISIT_AHBMASTER,
 *              AMBAPP_VISIT_AHBSLAVE, AMBAPP_VISIT_APBSLAVE)
 * - maxdepth:  Maximum bridge depth to visit
 * - fn:        User function called when a device is matched
 *   - info:    Pointer to "struct amba_apb_info" or "struct amba_ahb_info" (see type)
 *   - vendor:  Vendor ID for matched device
 *   - device:  Device ID for matched device
 *   - type:    Type of matched device (AMBAPP_VISIT_AHBMASTER,
 *              AMBAPP_VISIT_AHBSLAVE, AMBAPP_VISIT_APBSLAVE)
 *   - depth:   Bridge depth of matched device. First depth is 0.  The depth
 *              increments with one for each recursed bridge.
 *   - arg:     User argument which was given to ambapp_visit() parameter
 *              fn_arg
 *   - return:  0: continue scanning. non-zero: abort scanning and propagate
 *              return value to ambapp_visit() for return.
 * - fn_arg:    User argument provided with each call to fn(). ambapp_visit()
 *              never dereference it.
 * - return:    0: fn() did never return non-zero. non-zero: fn() returned this
 *              value.
 */
extern uint32_t ambapp_visit(
        uint32_t ioarea,
        uint32_t vendor,
        uint32_t device,
        uint32_t flags,
        uint32_t maxdepth,
        uint32_t (*fn)(
                void *info,
                uint32_t vendor,
                uint32_t device,
                uint32_t type,
                uint32_t depth,
                void *arg
        ),
        void *fn_arg
);

/*
 * Return first matched device
 *
 * This function can be used as fn parameter to ambapp_visit() to return the
 * address of the first matched device.
 *
 * If arg is not NULL, then a struct amba_apb_info or struct amba_ahb_info is
 * written to arg depending on the type parameter. Useful when more information
 * than the address of the first bar is required.
 */
extern uint32_t ambapp_findfirst_fn(
        void *info,
        uint32_t vendor,
        uint32_t device,
        uint32_t type,
        uint32_t depth,
        void *arg
);

/*
 * Get information on AHB Plug&Play record
 *
 * return: 0 iff success
 */
extern int ambapp_get_ahbinfo(
        uint32_t ioarea,
        const struct ambapp_ahb_entry *ahbcfgrec,
        struct amba_ahb_info *info
);

/*
 * Get information on APB Plug&Play record
 *
 * return: 0 iff success
 */
extern int ambapp_get_apbinfo(
        uint32_t apbbase,
        const struct ambapp_apb_entry *apbcfgrec,
        struct amba_apb_info *info
);

#endif

