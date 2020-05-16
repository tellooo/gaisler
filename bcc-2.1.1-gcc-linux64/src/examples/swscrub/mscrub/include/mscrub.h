/*
 * Software scrub library for AHBSTAT
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */

#ifndef _LIBMSCRUB_H_
#define _LIBMSCRUB_H_

#include <stdint.h>
#include <stdlib.h>

/*
 * Scrubber service configuration/state record.
 *
 * Before the scrubber service is called with a record the first time, all
 * mscrub_info fields must be initialized. Field 'next' shall be set equal to
 * 'start'.
 */
struct mscrub_info {
        /* Address of AHB status device (AHBSTAT). */
        volatile void *ahbstat_regs;
        /* Physical start address (32-bit word aligned) of the scrub area. */
        uint32_t start;
        /* Physical end address + 4 (32-bit word aligned) of the scrub area. */
        uint32_t end;
        /* Number of bytes to scrub for each call to mscrub_scrub(). */
        size_t blocksize;
        /*
         * The next 32-bit word address to scrub. Must be equal to start field
         * at initialization.
         */
        uint32_t next;
        /* Number of correctable errors detected. */
        uint32_t nce;
};

/*
 * Scrub EDAC read/write memory
 *
 * At each invocation, a fixed size block of RAM is read sequentially, one
 * 32-bit words at a time. In case a correctable EDAC error is detected by
 * AHBSTAT, the failing location is rewritten as atomic operation.
 *
 * Memory accesses to the target memory space is done with ASI 0x1C (cache and
 * MMU bypass).
 *
 * info: Scrubber service configuration/state record.
 * return: Number of locations rewritten during the invocation.
 */
uint32_t mscrub_scrub(
        struct mscrub_info *info
);

/*
 * Return total number of locations rewritten by scrubber service due to
 * detected correctable error.
 */
uint32_t mscrub_getnce(
        const struct mscrub_info *info
);

#endif

