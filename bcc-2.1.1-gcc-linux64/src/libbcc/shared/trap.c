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

#include <stdint.h>

#include "bcc/bcc.h"
#include "bcc/leon.h"

/*
 * Template for MVT trap table entry with the following signature:
 * - %l0: psr
 * - %l1: pc
 * - %l2: npc
 * - %l6: arg
 */
static const uint32_t mvt_templ[4] = {
        0xac102000, /* mov          0, %l6 */
        0x27100049, /* sethi        %hi(0x40012400), %l3 */
        0x81c4e2f4, /* jmp          %l3 + 0x2f4	! 400126f4 <leonbare_irq_entry> */
        0xa1480000  /* rd           %psr, %l0 */
};

/* Install MVT trap table entry */
static int mvt_set_trap(
        uint32_t base,
        int tt,
        void (*handler)(void),
        uint32_t arg
)
{
        uint32_t *entry;
        uint32_t h;

        tt &= 0xff;
        entry = (uint32_t *) base;
        entry += 4 * tt;

        h = (uint32_t) handler;
        entry[0] =  mvt_templ[0] | (arg & 0x00001fff);
        entry[1] = (mvt_templ[1] & ~(0xffffffff >> 10)) | (h >> 10);
        entry[2] = (mvt_templ[2] &  (0xffffffff << 13)) | (h & 0x3ff);
        entry[3] =  mvt_templ[3];

        return BCC_OK;
}

/* SVT implementation only supports the default trap table. */
extern uint32_t __bcc_trap_table_svt_level0;
extern uint32_t __bcc_trap_table_svt_allbad;

/* Install SVT trap table entry */
static int svt_set_trap(
        int tt,
        void (*handler)(void)
)
{
        int index0;
        int index1;
        uint32_t *table0;
        uint32_t *table1;

        index0 = (tt >> 4) & 0xF;
        index1 = tt & 0xF;
        table0 = &__bcc_trap_table_svt_level0;
        table1 = (uint32_t *) table0[index0];

        if ((&__bcc_trap_table_svt_allbad) == table1) {
                /* Not allowed to manipulate the default bad table. */
                return BCC_FAIL;
        }
        table1[index1] = (uint32_t) handler;

        return BCC_OK;
}

enum {
        TRAP_TABLE_TYPE_MVT = 0,
        TRAP_TABLE_TYPE_SVT = 1
};
extern uint8_t __bcc_trap_table_type;

int bcc_set_trap(
        int tt,
        void (*handler)(void)
)
{
        /* NOTE: Checking value of the symbol, not the value of the variable. */
        if (TRAP_TABLE_TYPE_SVT == (uintptr_t) &__bcc_trap_table_type) {
                return svt_set_trap(tt, handler);
        } else {
                uint32_t base;

                base = bcc_get_tbr() & TBR_TBA;
                return mvt_set_trap(base, tt, handler, tt);
        }
}

