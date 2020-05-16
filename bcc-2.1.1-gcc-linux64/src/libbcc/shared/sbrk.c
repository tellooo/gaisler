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

#include "bcc/bcc_param.h"

/* Default heap limits from linker script. */
extern uint8_t __heap_min;

static const ptrdiff_t DEFAULT_STACK_SIZE = 8 * 1024;

/* End of currently allocated heap. */
static uint8_t *__sbrk_heap_end;

/*
 * Stack pointer at application entry. This value can be read to calculate
 * __bcc_heap_max.
 */
extern uint8_t *const __bcc_sp_at_entry;


void *sbrk(ptrdiff_t incr)
{
        uint8_t *prev_sbrk_heap_end;

        /* Initialization step */
        if (NULL == __bcc_heap_min) {
                /*
                 * User did not specified heap lower limit: get value from
                 * linker script.
                 */
                __bcc_heap_min = &__heap_min;
        }
        if (NULL == __bcc_heap_max) {
                /*
                 * User did not specify heap upper limit: calculate value from stack pointer
                 * at entry.
                 */
                __bcc_heap_max = __bcc_sp_at_entry - DEFAULT_STACK_SIZE;
        }
        if (NULL == __sbrk_heap_end) {
                __sbrk_heap_end = __bcc_heap_min;
        }

        prev_sbrk_heap_end = __sbrk_heap_end;
        if (__sbrk_heap_end + incr < __bcc_heap_max) {
                __sbrk_heap_end += incr;
                return (void *) prev_sbrk_heap_end;
        } else {
                return (void *) -1;
        }
}

