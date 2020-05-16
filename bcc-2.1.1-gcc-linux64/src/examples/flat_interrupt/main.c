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

/*
 * This example demonstrates how to override the default BCC interrupt trap
 * handler. The example interrupt trap handler provides very low interrupt
 * response time and a C compliant run-time environment. See the README in this
 * directory for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <bcc/bcc.h>
#include "fl_isr.h"

#ifndef _FLAT
 #error requires -mflat!
#endif

unsigned int ntrig[FL_ISR_NVECS];

static void defhandler(uintptr_t arg)
{
        if (0) {
                printf("%s: arg=%u\n", __func__, arg);
        }
        ntrig[arg]++;
}

static void handler1(uintptr_t arg)
{
        if (0) {
                printf("%s: arg=%u\n", __func__, arg);
        }
        /* Force nested */
        bcc_int_force(2);
        ntrig[arg]++;
}

static void tick(int n);


int main(void)
{
        puts("");
        puts("--- EXAMPLE BEGIN ---");

        fl_isr_init();
        /*
         * Register same user ISR handler but with different arg parameter for
         * all interrupt levels.
         */
        for (int i = 0; i < FL_ISR_NVECS; i++) {
                fl_isr_register(i, defhandler, i);
        }
        fl_isr_register(1, handler1, 1);

        bcc_int_unmask(1);
        bcc_int_unmask(2);

        puts("Forcing interrupt 1 once per second.");
        puts("Interrupt 1 will force interrupt 2 (nesting).");
        puts("");
        /* The tick and ntrig[] numbers printed on same line expected equal */

        tick(1000);

        puts("");
        puts("--- EXAMPLE END ---");

        return EXIT_SUCCESS;
}

static void tick(int n) {
        uint32_t t0;
        uint32_t tdiff;
        const uint32_t DURATION = 1000000;

        t0 = bcc_timer_get_us();
        for (int i = 1; i <= n; i++) {
                do {
                        uint32_t t1;
                        t1 = bcc_timer_get_us();
                        tdiff = t1 - t0;
                } while (tdiff <= DURATION);
                t0 += DURATION;


                printf("tick %3d | ", i);

                /* The ntrig array should be updated by this force */
                bcc_int_force(1);

                for (int j = 1; j < 3; j++) {
                        printf(" ntrig[%d] =%3u |", j, ntrig[j]);
                }
                puts("");
        }
}

