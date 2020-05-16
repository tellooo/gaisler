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

#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "bcc/bcc.h"
#include "timer/timer_custom.h"

enum { US_PER_S = (1000 * 1000) };

int gettimeofday(
        struct timeval *tv,
        void *tz
)
{
        struct timezone *tzp = tz;
        uint32_t us;

        if (NULL != tv) {
                if (__bcc_timer_custom_get_timeval) {
                        __bcc_timer_custom_get_timeval(tv);
                } else {
                        us = bcc_timer_get_us();
                        /* NOTE: Newlib defines tv_sec and tv_usec as "long". */
                        tv->tv_sec = (us / US_PER_S);
                        tv->tv_usec = (us % US_PER_S);
                }
        }

        if (NULL != tzp) {
                tzp->tz_minuteswest = 0;
                tzp->tz_dsttime = 0;
        }

        return 0;
}

