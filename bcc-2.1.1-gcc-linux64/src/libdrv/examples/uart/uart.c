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
 * This example prints a message on the second UART.
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

/*
 * 0: no FIFO debug. Connect a terminal to the external UART pins.
 * 1: APBUART debug FIFO. use with GRMON:
 *      grmon3> foward enable uart1
 */
#ifndef  CFG_FIFO_DEBUG
 #define CFG_FIFO_DEBUG 0
#endif

#ifndef  CFG_UART_INDEX
 #define CFG_UART_INDEX 1
#endif

#ifndef  CFG_UART_BAUD
 #define CFG_UART_BAUD 38400
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <drv/apbuart.h>
#include <drv/gr716/apbuart.h>

void sendmsg(struct apbuart_priv *dev, const char *str)
{
        while (*str) {
                while (0 == apbuart_outbyte(dev, *str));
                str++;
        }
}

void writemsg(struct apbuart_priv *dev, const char *str)
{
        size_t n = 0;
        size_t count;

        count = strlen(str);
        while (n < count) {
                n += apbuart_write(dev, (const uint8_t *) &str[n], count-n);
        }
}

#define TXFIFOBUFSZ 32
#define RXFIFOBUFSZ 32
uint8_t txfifobuf[TXFIFOBUFSZ];
uint8_t rxfifobuf[RXFIFOBUFSZ];

int main(void)
{
        int ret;
        struct apbuart_priv *dev1;
        struct apbuart_config cfg = { 0 };

        if (CFG_TARGET_GR716) {
                apbuart_init(GR716_APBUART_DRV_ALL);
        } else {
                apbuart_autoinit();
        }

        ret = apbuart_dev_count();
        printf("INFO: apbuart_dev_count() -> %d\n", ret);
        if (ret == 0) {
                printf("ERROR: to few devices\n");
                return 1;
        }
        for (int i = 0; i < ret; i++) {
                const struct drv_devreg *devreg;

                devreg = apbuart_get_devreg(i);
                printf(
                        "apbuart%d: addr=%p, interrupt=%d\n",
                        i,
                        (void *) devreg->addr,
                        devreg->interrupt
                );
        }

        dev1 = apbuart_open(CFG_UART_INDEX);
        if (!dev1) {
                printf("ERROR: error opening apbuart%d\n", CFG_UART_INDEX);
                return 1;
        }

        if (CFG_FIFO_DEBUG) {
                apbuart_set_debug(dev1, 1);
        }

        cfg.baud = CFG_UART_BAUD;
        cfg.parity = APBUART_PAR_NONE;
        cfg.flow = 0;
        cfg.mode = APBUART_MODE_NONINT;

        apbuart_config(dev1, &cfg);
        sendmsg(dev1, "hello world\n\r");
        apbuart_drain(dev1);

        apbuart_close(dev1);

        dev1 = apbuart_open(CFG_UART_INDEX);
        if (!dev1) {
                printf("ERROR: error opening device\n");
                return 1;
        }

        if (CFG_FIFO_DEBUG) {
                apbuart_set_debug(dev1, 1);
        }

        cfg.baud = CFG_UART_BAUD;
        cfg.parity = APBUART_PAR_NONE;
        cfg.flow = 0;
        cfg.mode = APBUART_MODE_INT;
        cfg.txfifobuf = txfifobuf;
        cfg.txfifobuflen = sizeof (txfifobuf);
        cfg.rxfifobuf = rxfifobuf;
        cfg.rxfifobuflen = sizeof (rxfifobuf);

        apbuart_config(dev1, &cfg);
        writemsg(dev1, "HELLO WORLD using interrupt based apbuart_write()\n\r");
        apbuart_drain(dev1);

        apbuart_close(dev1);

        printf("INFO: end\n");

        return 0;
}

