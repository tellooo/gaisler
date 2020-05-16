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

static const char *DESCRIPTION =
"This program tries to detect I2C devices on all available\n"
"I2CMST controllers. It does so by performing a read on each\n"
"non-reserved I2C address. 7-bit addressing is used.\n";
/*
 * Author: Martin Åberg, Cobham Gaisler AB
 */

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drv/osal.h>
#include <drv/nelem.h>
#include <drv/drvret.h>

#include <drv/i2cmst.h>
#include <drv/gr716/i2cmst.h>

#define WORDS 4
uint32_t rxbuf[WORDS];

struct i2cmst_packet pkt[4];
struct i2cmst_list pkts;

static int scandev(struct i2cmst_priv *dev)
{
        int ndev = 0;
        int ret;
        const int ten_bit_addr = 0;
        const int retries = 1;
        ret = i2cmst_set_retries(dev, retries);
        assert(DRV_OK == ret);
        ret = i2cmst_set_speed(dev, I2CMST_SPEED_STD);
        assert(DRV_OK == ret);
        ret = i2cmst_set_ten_bit_addr(dev, ten_bit_addr);
        assert(DRV_OK == ret);
        ret = i2cmst_set_interrupt_mode(dev, 1);
        assert(DRV_OK == ret);

        ret = i2cmst_start(dev);
        assert(DRV_OK == ret);

        for (int devaddr = 0; devaddr < 0x80; devaddr++) {
                if (devaddr < 0x10) {
                        continue;
                }
                if ((0xf9 & devaddr) == 0xf9) {
                        /* Device ID */
                        continue;
                }
                if ((0xf8 & devaddr) == 0xf0) {
                        /* 10-bit slave addressing */
                        continue;
                }

                /* Read  */
                pkt[0].next = NULL;
                pkt[0].flags = I2CMST_FLAGS_READ;
                //pkt[0].flags = 0;
                pkt[0].slave = devaddr;
                pkt[0].addr = 0;
                pkt[0].length = 1;
                rxbuf[0] = 0xff;
                pkt[0].payload = (uint8_t*)&rxbuf[0];

                pkts.head = &pkt[0];
                pkts.tail = &pkt[0];
                i2cmst_request(dev, &pkts);
                assert(DRV_OK == ret);

                struct i2cmst_packet *ptr = NULL;
                pkts.head = NULL;
                pkts.tail = NULL;
                while (ptr == NULL) {
                        i2cmst_reclaim(dev, &pkts);
                        ptr = pkts.head;
                }

                uint32_t flags = pkt[0].flags;
                /* Verify */
                assert(flags & I2CMST_FLAGS_FINISHED);
                if ((flags & I2CMST_FLAGS_ERR) == 0) {
                        ndev++;
                        printf(
                                " Detected I2C device at I2C address 0x%02x",
                                devaddr
                        );
                        if (0) {
                                printf("  flags=0x%02x\n", (unsigned) flags);
                        }
                        puts("");
                } else {
                        ;
                }

        }

        ret = i2cmst_stop(dev);
        assert(DRV_OK == ret);
        printf(" - This I2C master got ");
        if (ndev) {
                printf("response on %d I2C address(es)\n", ndev);
        } else {
                printf("no response on any I2C address\n");
        }

        return 0;
}

int main(void)
{
        if (CFG_TARGET_GR716) {
                i2cmst_init(GR716_I2CMST_DRV_ALL);
        } else {
                i2cmst_autoinit();
        }

        puts("");
        puts("");
        puts(DESCRIPTION);

        const int dev_count = i2cmst_dev_count();
        int ret;

        printf("INFO: i2cmst_dev_count() -> %d\n", dev_count);
        if (dev_count < 1) {
                printf("INFO: no I2CMST device found\n");
                return 1;
        }
        for (int i = 0; i < dev_count; i++) {
                struct i2cmst_priv *dev = NULL;
                const struct drv_devreg *devreg = NULL;

                puts("");
                devreg = i2cmst_get_devreg(i);
                printf(
                        "i2cmst%d: addr=%p, interrupt=%d\n",
                        i,
                        (void *) devreg->addr,
                        devreg->interrupt
                );

                dev = i2cmst_open(i);
                assert(dev);

                scandev(dev);

                ret = i2cmst_close(dev);
                assert(DRV_OK == ret);
        }

        return 0;
}

