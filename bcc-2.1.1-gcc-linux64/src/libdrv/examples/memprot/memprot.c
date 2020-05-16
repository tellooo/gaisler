/*
 * Copyright (c) 2019, Cobham Gaisler AB
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

static const char *DESCRIPTION =
"GR716 memory protection unit example.\n"
"NOTE: This example is functional only on the GR716.\n"
;

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drv/osal.h>
#include <drv/nelem.h>
#include <drv/drvret.h>

#include <drv/memprot.h>
#include <drv/gr716/memprot.h>

void printsi(const struct memprot_seginfo *si)
{
        printf("  start = %08x\n", (unsigned) si->start);
        printf("  end   = %08x\n", (unsigned) si->end);
        printf("  g     = %08x\n", (unsigned) si->g);
        printf("  en    = %d\n", !!si->en);
}
void printstarted(struct memprot_priv *dev)
{
        int s;

        s = memprot_isstarted(dev);
        printf(
                "%s: device is %s\n",
                __func__,
                s ? "STARTED" : "STOPPED"
        );
}

int example0(void)
{
        volatile uint32_t *const trywrite = (void *) 0x80004000;
        const int DEVNO = 0;
        struct memprot_priv *dev;
        int nseg;

        printf("%s: open memprot%d...\n", __func__, DEVNO);
        dev = memprot_open(0);
        nseg = memprot_nseg(dev);
        printf("%s: memprot%d has %d segments\n", __func__, DEVNO, nseg);
        puts("");
        printf("%s: device configuration at open():\n", __func__);
        struct memprot_seginfo si = { 0 };
        for (int i = 0; i < nseg; i++) {
                printf("SEGMENT %d\n", i);
                memprot_get(dev, i, &si);
                printsi(&si);
                puts("");
        }

        printf("%s: reset()...\n", __func__);
        memprot_reset(dev);
        printstarted(dev);
        puts("");


        si.start        = 0x80004000;
        si.end          = 0x800040ff;
        si.g            = 1 << 2;
        si.en           = 1;
        printf("%s: install example configuration on segment 0...\n", __func__);
        memprot_set(dev, 0, &si);
        printf("%s: reading back segment 0...\n", __func__);
        memprot_get(dev, 0, &si);
        printsi(&si);

        puts("");

        printf("%s: trying to write %p...\n", __func__, (void *) trywrite);
        *trywrite = 1;
        printf("%s: PASS - expected since core is disabled\n", __func__);
        puts("");
        printf("%s: starting (enabling) memprot%d...\n", __func__, DEVNO);

        memprot_start(dev);
        printstarted(dev);

        puts("");
        printf(
                "%s: writes to %p is now expected to trap with tt = 0x2B.\n",
                __func__,
                (void *) trywrite
        );
        printf("%s:   HINT: Use the GRMON 'reset' command ", __func__);
        printf("to disable memory protection\n");
        printf("%s: trying to write %p...\n", __func__, (void *) trywrite);
        *trywrite = 1;
        printf("%s: PASS (not expected)\n", __func__);
        puts("");

        return 0;
}

int main(void)
{
        if (CFG_TARGET_GR716) {
                memprot_init(GR716_MEMPROT_DRV_ALL);
        } else {
                memprot_autoinit();
        }

        puts("");
        puts("");
        puts(DESCRIPTION);

        const int dev_count = memprot_dev_count();

        printf("INFO: memprot_dev_count() -> %d\n", dev_count);
        if (dev_count < 1) {
                printf("INFO: no MEMPROT device found\n");
                return 1;
        }
        puts("");
        for (int i = 0; i < dev_count; i++) {
                const struct drv_devreg *devreg = NULL;

                devreg = memprot_get_devreg(i);
                printf(
                        "memprot%d: addr=%p, interrupt=%d\n",
                        i,
                        (void *) devreg->addr,
                        devreg->interrupt
                );
        }
        puts("");

        example0();

        return 0;
}

