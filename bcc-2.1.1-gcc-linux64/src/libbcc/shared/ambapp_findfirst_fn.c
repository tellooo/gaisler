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

#include "include/bcc/ambapp.h"

uint32_t ambapp_findfirst_fn(
        void *info,
        uint32_t vendor,
        uint32_t device,
        uint32_t type,
        uint32_t depth,
        void *arg
)
{
        (void) vendor;
        (void) device;
        (void) depth;
        struct amba_apb_info *apbi = info;
        struct amba_ahb_info *ahbi = info;

        if (NULL != arg) {
                if (AMBAPP_VISIT_APBSLAVE == type) {
                        struct amba_apb_info *uinfo = arg;
                        *uinfo = *apbi;
                } else if (
                        (AMBAPP_VISIT_AHBSLAVE  == type) ||
                        (AMBAPP_VISIT_AHBMASTER == type)
                ) {
                        struct amba_ahb_info *uinfo = arg;
                        *uinfo = *ahbi;
                }
        }

        return apbi->start;
}

