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

/*
 * This example demonstrates how AHBSTAT interrupts can be used. For the
 * purpose of the example, the ISR counts the total number of errors
 * encountered and properties of the last error.
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */

static const int MYDEVNUM = 0;

#include <stdio.h>
#include <string.h>
#include <drv/nelem.h>
#include <drv/ahbstat.h>
#include <drv/gr716/ahbstat.h>

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

static void print_devreg(const struct drv_devreg *devreg)
{
        printf(
                "addr=%08x, interrupt=%2d, device_id=0x%03x, version=%d",
                (unsigned int) devreg->addr,
                devreg->interrupt,
                devreg->device_id,
                devreg->version
        );
}

struct myisr_ctx {
        int count;
        unsigned int last_status;
        unsigned int last_failing_address;
};
static volatile struct myisr_ctx myisr_ctx0;

static int (myisr)(
        volatile struct ahbstat_regs *regs,
        uint32_t status,
        uint32_t failing_address,
        void *userarg
);

static void testit(struct ahbstat_priv *dev)
{
        int ret;
        volatile struct ahbstat_regs *regs = ahbstat_get_regs(dev);
        volatile struct myisr_ctx *ctx = &myisr_ctx0;

        ret = ahbstat_set_user(dev, myisr, (void *) &myisr_ctx0);
        if (ret) {
                printf("ERROR: ahbstat_set_user()\n");
        }

        printf("Trig interrupt...\n");
        regs->status = AHBSTAT_STS_CE | AHBSTAT_STS_NE;
        /* Wait for driver ISR to clear status. */
        while (regs->status & (AHBSTAT_STS_CE | AHBSTAT_STS_NE)) {
                ;
        }
        printf("Interrupt condition cleared\n");
        if (0 == ctx->count) {
                printf("ERROR: User ISR has not been called\n");
        }
        printf("User ISR has been called %d times\n", ctx->count);
        printf(" count=%d\n", ctx->count);
        printf(" last_status=%08x\n", ctx->last_status);
        printf(" last_failing_address=%08x\n", ctx->last_failing_address);
}

int main(void)
{
        int ret;
        struct ahbstat_priv *dev0;

        printf("EXAMPLE BEGIN\n");

        if (CFG_TARGET_GR716) {
                printf("Init with GR716 static tables\n");
                ahbstat_init(GR716_AHBSTAT_DRV_ALL);
        } else {
                printf("Automatic init with AMBA Plug&Play and malloc()\n");
                ahbstat_autoinit();
        }

        ret = ahbstat_dev_count();
        printf("ahbstat_dev_count() -> %d\n", ret);
        if (ret == 0) {
                printf("ERROR: to few devices\n");
                return 1;
        }
        for (int i = 0; i < ret; i++) {
                const struct drv_devreg *devreg = ahbstat_get_devreg(i);
                printf(" ahbstat%d: ", i);
                print_devreg(devreg);
                printf("\n");
        }

        dev0 = ahbstat_open(MYDEVNUM);
        if (!dev0) {
                printf("ERROR: error opening device 0\n");
                return 1;
        }

        testit(dev0);

        ahbstat_close(dev0);

        printf("EXAMPLE END\n");

        return 0;
}

/*
 * Just log information for the last error. Note that there is a race on
 * accessing the ctx here and in the non-interrupt code. OK for this example.
 */
static int (myisr)(
        volatile struct ahbstat_regs *regs,
        uint32_t status,
        uint32_t failing_address,
        void *userarg
) {
        UNUSED(regs);
        volatile struct myisr_ctx *ctx = userarg;

        ctx->count++;
        ctx->last_status = status;
        ctx->last_failing_address = failing_address;

        /* User ISR returns 0 so driver ISR reenables AHBSTAT monitoring. */
        return 0;
}

