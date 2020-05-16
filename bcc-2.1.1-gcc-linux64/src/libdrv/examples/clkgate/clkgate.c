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
 * Print current clock gating unit configuration
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */

#include <stdio.h>
#include <string.h>
#include <drv/nelem.h>
#include <drv/clkgate.h>
#include <drv/gr716/clkgate.h>

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

static void showit(struct clkgate_priv *dev)
{
        int ret;
        uint32_t enabled;
        uint32_t disabled;

        ret = clkgate_status(dev, &enabled, &disabled);
        if (ret) {
                printf("ERROR: clkgate_status()\n");
                return;
        }
        printf("  enabled=%08x\n", (unsigned int) enabled);
        printf("  disabled=%08x\n", (unsigned int) disabled);
}

int main(void)
{
        int ret;

        printf("EXAMPLE BEGIN\n");

        if (CFG_TARGET_GR716) {
                printf("Init with GR716 static tables\n");
                clkgate_init(GR716_CLKGATE_DRV_ALL);
        } else {
                printf("Automatic init with AMBA Plug&Play and malloc()\n");
                clkgate_autoinit();
        }

        ret = clkgate_dev_count();
        printf("clkgate_dev_count() -> %d\n", ret);
        if (ret == 0) {
                printf("ERROR: to few devices\n");
                return 1;
        }
        for (int i = 0; i < ret; i++) {
                const struct drv_devreg *devreg = clkgate_get_devreg(i);
                printf(" clkgate%d: ", i);
                print_devreg(devreg);
                printf("\n");
        }
        puts("");

        for (int i = 0; i < ret; i++) {
                struct clkgate_priv *dev;
                dev = clkgate_open(i);
                if (!dev) {
                        printf("ERROR: error opening device 0\n");
                        return 1;
                }
                printf("clkgate%d configuration:\n", i);
                showit(dev);

                if (0) {
                        /* enable device 8 and 4 on this unit */
                        clkgate_enable(dev, 0x00000110);
                }
                if (0) {
                        /* disable device 31 on this unit */
                        clkgate_enable(dev, 1 << 31);
                }
                clkgate_close(dev);
                puts("");
        }

        printf("EXAMPLE END\n");

        return 0;
}

