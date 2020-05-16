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

#ifndef __BCC_BCC_INLINE_H_
#define __BCC_BCC_INLINE_H_

/*
 * Implementation of inline functions. This file shall only be included from
 * <bcc/bcc.h>.
 */

#include <bcc/capability.h>

#ifdef __BCC_BSP_HAS_PWRPSR

#include <bcc/leon.h>
static inline int bcc_set_pil_inline(int newpil)
{
        uint32_t ret;
        __asm__ volatile (
	        "sll %4, %1, %%o1\n\t"
	        "or %%o1, %2, %4\n\t"
                ".word 0x83882000\n\t"
	        "rd %%psr, %%o1\n\t"
	        "andn %%o1, %3, %%o2\n\t"
	        "wr %4, %%o2, %%psr\n\t"
	        "and %%o1, %3, %%o2\n\t"
	        "srl %%o2, %1, %0\n\t"
                : "=r" (ret)
                : "i" (PSR_PIL_BIT), "i" (PSR_ET), "i" (PSR_PIL), "r" (newpil)
                : "o1", "o2"
        );
        return ret;
}

#else

#include <bcc/leon.h>
static inline int bcc_set_pil_inline(int newpil)
{
        register uint32_t _val __asm__("o0") = newpil;
        /* NOTE: nop for GRLIB-TN-0018 */
        __asm__ volatile (
                "ta %1\nnop\n" :
                "=r" (_val) :
                "i" (BCC_SW_TRAP_SET_PIL), "r" (_val)
        );
        return _val;
}

#endif

static inline int bcc_int_disable(void)
{
        return bcc_set_pil_inline(15);
}

static inline void bcc_int_enable(int plevel)
{
        bcc_set_pil_inline(plevel);
}

static inline int bcc_timer_tick_init(void)
{
        return bcc_timer_tick_init_period(10 * 1000);
}

#endif

