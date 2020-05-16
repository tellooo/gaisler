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
 * This example performs SPI transfers in loopback on device 0.
 */

/*
 * 0: scan devices using ambapp
 * 1: use GR716 static driver tables
 */
#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

#include <assert.h>
#include <stdio.h>

#include <drv/spi.h>
#include <drv/gr716/spi.h>
#include <drv/regs/spictrl-regs.h>
#include <drv/nelem.h>

void spi004(void)
{
        struct spi_priv *dev0;
        int ret;
        struct spi_config cfg;
        uint32_t event;
        volatile struct spictrl_regs *REGSA;

        if (CFG_TARGET_GR716) {
                spi_init(GR716_SPI_DRV_ALL);
        } else {
                spi_autoinit();
        }

        REGSA = (void *) spi_get_devreg(0)->addr;
        /* Open */
        dev0 = spi_open(0);
        assert(dev0);

        /* Use default config with 125 KHz */
        cfg = SPI_CONFIG_DEFAULT;
        cfg.freq = 125 * 1000;

        ret = spi_config(dev0, &cfg);

        assert(DRV_OK == ret);
        /* Enable loopback */
        REGSA->mode |= SPICTRL_MODE_LOOP;

        /* Start */
        ret = spi_start(dev0);
        assert(DRV_OK == ret);

        uint32_t txbuf[64];
        uint32_t rxbuf[64];
        for (size_t i = 0; i < NELEM(txbuf); i++) {
                txbuf[i] = 0xdeadbe00 + i;
                rxbuf[i] = 0xc0de1200 + i;
        }

        /* Write nothing */
        ret = spi_write(dev0, txbuf, 0);
        assert(0 == ret);

        /* Write one word with slave 1 is selected */
        const int SELINDEX = 1;
        spi_slave_select(dev0, (1 << SELINDEX));
        ret = spi_write(dev0, txbuf, 1);
        spi_slave_select(dev0, 0);

        assert(1 == ret);
        /* Wait for it to end up at RX */
        { volatile int busy = 100000; while (busy--); }

        /* Check event */
        event = spi_get_event(dev0);
        assert((SPICTRL_EVENT_NE | SPICTRL_EVENT_NF) == event);

        /* Read nothing */
        ret = spi_read(dev0, rxbuf, 0);

        assert(0 == ret);

        /* Read one word */
        ret = spi_read(dev0, rxbuf, 1);

        assert(1 == ret);
        /* RX empty again */
        assert(txbuf[0] == rxbuf[0]);

        /* Write very much */
        ret = spi_write(dev0, NULL, 256);

        /*
         * At least 17 words shall be written. May be more than 17 depending on
         * execution speed vs SPI speed.
         */
        assert(17 <= ret);
        /* Stop */
        ret = spi_stop(dev0);

        assert(DRV_OK == ret);

        /* Close */
        ret = spi_close(dev0);
        assert(DRV_OK == ret);
}

int main(void)
{
        puts("SPI example begin");
        spi004();
        puts("PASS");
        puts("SPI example end");
        return 0;
}

