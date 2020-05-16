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
"This program reads and writes to an EEPROM connected to I2CMST\n";

static const int EEPROM_ADDR = 0x50;
/* Requires EEPROM write buffer of at least 8 byte */
static const int NWRITE = 8;

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drv/osal.h>
#include <drv/nelem.h>
#include <drv/drvret.h>

#include <drv/i2cmst.h>
#include <drv/gr716/i2cmst.h>

int dosimple(
        struct i2cmst_priv *dev,
        int uflags,
        int devaddr,
        uint16_t memaddr,
        uint8_t *payload,
        int length
)
{
        struct i2cmst_packet pkt;
        struct i2cmst_list pkts;
        int ret;
        pkt.next = NULL;
        pkt.flags = uflags;
        pkt.slave = devaddr;
        pkt.addr = memaddr;
        pkt.length = length;
        pkt.payload = payload;

        pkts.head = &pkt;
        pkts.tail = &pkt;
        i2cmst_request(dev, &pkts);
        assert(DRV_OK == ret);

        struct i2cmst_packet *ptr = NULL;
        while (ptr == NULL) {
                i2cmst_reclaim(dev, &pkts);
                ptr = pkts.head;
        }

        uint32_t flags = ptr->flags;
        if (0) {
                /* Verify */
                assert(flags & I2CMST_FLAGS_FINISHED);
                printf("  flags=0x%02x  ", (unsigned) flags);
                printf("%10s", flags & I2CMST_FLAGS_FINISHED ? "FINISHED" : "-");
                printf("%10s", flags & I2CMST_FLAGS_RETRIED  ? "RETRIED " : "-");
                printf("%10s", flags & I2CMST_FLAGS_ERR ? "ERR" : "-");
                printf("%10s", flags & I2CMST_FLAGS_READ ? "READ" : "-");
                printf("%10s", flags & I2CMST_FLAGS_ADDR ? "ADDR" : "-");
                puts("");
        }
        return flags & I2CMST_FLAGS_ERR;
}

void hexdump(uint8_t *payload, int length)
{
        for (int i = 0; i < length; i++) {
                printf("[%2d] = 0x%02x  ", i, payload[i]);
                if (isprint(payload[i])) {
                        printf("%c", payload[i]);
                }
                puts("");
        }
}

int i2cwrite(
        struct i2cmst_priv *dev,
        int devaddr,
        uint8_t *buf,
        int length
)
{
        return dosimple(dev, 0, devaddr, 0, buf, length);
}

int i2cread(
        struct i2cmst_priv *dev,
        int devaddr,
        uint8_t *buf,
        int length
)
{
        return dosimple(dev, I2CMST_FLAGS_READ, devaddr, 0, buf, length);
}

unsigned char buf[100];
static int eepromdev(struct i2cmst_priv *dev)
{
        int ret;
        const int ten_bit_addr = 0;
        const int retries = 10;
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

        printf("Reading 20 bytes from EEPROM\n");
        buf[0] = 0;
        ret = i2cwrite(dev, EEPROM_ADDR, buf, 1);
        ret = i2cread(dev, EEPROM_ADDR, buf, 20);
        printf("Contents of first 20 bytes of EEPROM:\n");
        hexdump(buf, 20);

        printf("Writing %d bytes to EEPROM, starting at address 0x00:\n", NWRITE);
        buf[0] = 0;
        for (int i = 1; i < (NWRITE+1); i++) {
                buf[i] = 'a' + i-1;
        }
        ret = i2cwrite(dev, EEPROM_ADDR, buf, NWRITE+1);
        printf("done..\n");

        printf("Reading the first 20 bytes of the EEPROM:\n");
        buf[0] = 0;
        ret = i2cwrite(dev, EEPROM_ADDR, buf, 1);
        ret = i2cread(dev, EEPROM_ADDR, buf, 20);
        hexdump(buf, 20);

        ret = i2cmst_stop(dev);
        assert(DRV_OK == ret);

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

                eepromdev(dev);

                ret = i2cmst_close(dev);
                assert(DRV_OK == ret);
        }

        return 0;
}

