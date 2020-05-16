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
#include <stdlib.h>

#include <bsp.h>
#include "bcc/bcc_param.h"
#include "bcc/ambapp.h"

static const uint32_t ioarea = 0xfff00000;

int __bcc_con_init(void)
{
        /*
         * If the BSP has set a handle value at compile-time then just return.
         * The rest of this function will be optimized away.
         */
        if (__BSP_CON_HANDLE) {
                return BCC_OK;
        }

        /* Skip scanning if handle was defined at link time. */
        if (__bcc_con_handle) {
                return BCC_OK;
        }

        __bcc_con_handle = ambapp_visit(
                ioarea,
                VENDOR_GAISLER,
                GAISLER_APBUART,
                AMBAPP_VISIT_APBSLAVE,
                4,
                ambapp_findfirst_fn,
                NULL
        );

        if (__bcc_con_handle) {
                return BCC_OK;
        } else {
                return BCC_NOT_AVAILABLE;
        }
}

