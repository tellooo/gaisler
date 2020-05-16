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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <drv/gr1553bm.h>

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define __bsp_gr716__ 1
#else
 #define __bsp_gr716__ 0
#endif

#include "bm_logger.h"


struct bm_logger {
	void *bm;
	int entry_cnt;
	struct gr1553bm_config bmcfg;

	struct gr1553bm_entry entries[256];
	int nentries_log[5000];
};


/* Set up BM to log eveything */
static struct bm_logger *bm_alloc(void)
{
#if __bsp_gr716__
	static struct bm_logger bm_static __attribute__((section(".data.ext")));
	return &bm_static;
#else
	return calloc(1, sizeof(struct bm_logger));
#endif
}

static void bm_free(struct bm_logger *bm)
{
#if __bsp_gr716__
	/* Do nothing */
#else
	free(bm);
#endif
}

int bm_init(bm_logger_t *bm, void* log_base)
{
	int status;
	*bm = bm_alloc();
	if (bm == NULL ) {
		printf("Failed to allocate BM LOG[%d]\n", 0);
		return -1;
	}

	/* Aquire BM device */
	(*bm)->bm = gr1553bm_open(0);
	if (!(*bm)->bm ) {
		printf("Failed to open BM[%d]\n", 0);
		bm_free(*bm);
		*bm = NULL;
		return -1;
	}

	(*bm)->bmcfg.time_resolution = 0;	/* Highest time resoulution */
	(*bm)->bmcfg.time_ovf_irq = 1;	/* Let IRQ handler update time */
	(*bm)->bmcfg.filt_error_options = 0xe; /* Log all errors */
	(*bm)->bmcfg.filt_rtadr = 0xffffffff;/* Log all RTs and Broadcast */
	(*bm)->bmcfg.filt_subadr= 0xffffffff;/* Log all sub addresses */
	(*bm)->bmcfg.filt_mc = 0x7ffff;	/* Log all Mode codes */
	(*bm)->bmcfg.buffer_size = 16*1024;/* 16K buffer */
	(*bm)->bmcfg.buffer_custom = (void *)log_base;	/* Let driver allocate dynamically or custom adr */
	(*bm)->bmcfg.copy_func = NULL;	/* Standard Copying */
	(*bm)->bmcfg.copy_func_arg = NULL;
	(*bm)->bmcfg.dma_error_isr = NULL;	/* No custom DMA Error IRQ handling */
	(*bm)->bmcfg.dma_error_arg = NULL;


	/* Register standard IRQ handler when an error occur */
#if __bsp_gr716__
	if ( gr1553bm_config_init((*bm)->bm, &(*bm)->bmcfg)) {
		gr1553bm_close((*bm)->bm);
		printf("Failed to configure BM driver\n");
		bm_free(*bm);
		*bm = NULL;
		return -3;
	}
#else
	if (gr1553bm_config_alloc((*bm)->bm, &(*bm)->bmcfg) ) {
		printf("Failed to configure BM driver\n");
		gr1553bm_close((*bm)->bm);
		bm_free(*bm);
		*bm = NULL;
		return -3;
	}
#endif
	/* Start BM Logging as configured */
	status = gr1553bm_start((*bm)->bm);
	if ( status ) {
		printf("Failed to start BM: %d\n", status);
		gr1553bm_close((*bm)->bm);
		bm_free(*bm);
		*bm = NULL;
		return -4;
	}

	return 0;
}

void bm_stop(bm_logger_t bm)
{
	if (bm) {
		printf("BM Entries: %d\n", bm->entry_cnt);
		gr1553bm_close(bm->bm);
		bm_free(bm);
	}
}

int bm_count(bm_logger_t bm)
{
	if (bm) {
		return bm->entry_cnt;
	}
	return 0;
}

/* Handle BM LOG (empty it) */
int bm_log(bm_logger_t bm)
{
	int nentries, max, tot;
	int index, count;
	nentries = 10000000; /* check that is overwritten */
	if ( gr1553bm_available(bm->bm, &nentries) ) {
		printf("Failed to get number of available BM log entries\n");
		return -2;
	}

	count = sizeof(bm->nentries_log)/sizeof(bm->nentries_log[0]);
	index = nentries > count ? nentries : count-1;
	bm->nentries_log[index]++;

	tot = 0;
	do {
		max = 128;
		if ( gr1553bm_read(bm->bm, &bm->entries[0], &max) ) {
			printf("Failed to read BM log entries\n");
			return -3;
		}
		tot += max;

		if (bm->bmcfg.copy_func == NULL) {
		/* Handle Copied BM Log here.
		 */
		} else {
		 /* When doing custom copy, the copy function has
		  * already handled the data. Nothing to do.
		  */
		}

		/* Read as long as the BM driver fills our buffer */
	} while ( max == 128 );

	if ( tot < nentries ) {
		printf("BM Failed to read all entries: %d, %d\n", nentries, max);
		return -4;
	}

	/* Update stats */
	bm->entry_cnt += tot;
	/*printf("BM Entries: %d (time: %llu)\n", entry_cnt, time1553);*/
	/*printf("BM Entries: %d\n", entry_cnt);*/

	return 0;
}
