/*
 * Copyright 2018 Cobham Gaisler AB
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*  GR1553B BM driver usage example
 *
 *  COPYRIGHT (c) 2018
 *  Cobham Gaisler
 *
 *
 *  OVERVIEW
 *  --------
 *  Logger of 1553 bus, does not filter out anything, reads from a DMA area
 *  and compresses it into a larger log area. The larger log area can be read
 *  from a TCP/IP service using the linux_client.c example. The Linux
 *  application output it into a textfile for later processing.
 *
 *  The example is configured from config_bm.h and affects all applications
 *  using the BM: rtems-gr1553bm, rtems-gr1553bcbm rtems-gr1553rtbm
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <bcc/bcc.h>
#define TICKS_PER_SEC 100
#define US_TO_TICKS(t) ((t)/10000)
#define SLEEP_TICKS(ticks) do { \
	uint32_t epoch = bcc_timer_get_us(); \
	while ( US_TO_TICKS(bcc_timer_get_us() - epoch) < (ticks) ); \
} while (0)

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define __bsp_gr716__ 1
#else
 #define __bsp_gr716__ 0
#endif

  /* Bus Monitor (BM) LOGGING BASE ADDRESS : Dynamically allocated */
#if __bsp_gr716__
  char bm_log_base[128*1024] __attribute__((section(".data.ext"),aligned(8)));
  #define BM_LOG_BASE &bm_log_base[0]
#else
  #define BM_LOG_BASE NULL
#endif

#include "bm_logger.h"

bm_logger_t bm;

int bm_loop(void)
{
	int time;
	printf("Starting Copy of Log\n");

	time = TICKS_PER_SEC * 12; /* seconds */
	while ( time ) {
		/* Handle Log buffer */
		if ( bm_log(bm) ) {
			printf("BM Log failed\n");
			return -2;
		}

                if (bm_count(bm)) {
			--time;
		}

		/* Sleep one tick (up to 10ms) */
               SLEEP_TICKS(1);
	}

	return 0;
}

int gr1553bm_test(void)
{
	int retval = 0;

	if ( bm_init(&bm, BM_LOG_BASE) ) {
		printf("Failed to initialize BM\n");
		retval = -1;
		goto end;
	}

	if ( bm_loop() ) {
		printf("Failed to log BM\n");
		retval = -1;
		goto end;
	}

end:
	bm_stop(bm);
	return retval;
}

#include <drv/gr1553b.h>
#if __bsp_gr716__
#include <drv/gr716/gr1553b.h>
#endif

void __bcc_init70(void) {
        int ret;
        ret = bcc_timer_tick_init();
        if (BCC_OK != ret) {
            exit(EXIT_FAILURE);
        }
}

int main(void)
{
#if __bsp_gr716__
	gr1553_register(GR716_GR1553B_DRV_ALL[0]);
#else
	gr1553_autoinit();
#endif
        return gr1553bm_test();
}
