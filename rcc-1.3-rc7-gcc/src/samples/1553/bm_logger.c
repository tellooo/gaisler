#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <grlib/gr1553bm.h>
#include <rtems.h>

#include "ethsrv.h"
#include "bm_logger.h"

/* Compressed log */
struct compressed_log {
	unsigned int *base;
	unsigned int *head;
	unsigned int *tail;
	unsigned int *end;
	uint64_t time;
	uint64_t lastlogtime;
};

struct bm_logger {
	void *bm;
	int entry_cnt;
	struct gr1553bm_config bmcfg;
	struct compressed_log compressed;
	bm_eth_server_t eth;

	struct gr1553bm_entry entries[256];
	int nentries_log[5000];
};

#define COMPRESSED_LOG_SIZE 0x200000    /* 2Mb */
#define COMPRESSED_LOG_CNT  (COMPRESSED_LOG_SIZE/4)

int compressed_log_copy(
	unsigned int dst,
	struct gr1553bm_entry *src,
	int nentries,
	void *data
	);

/* Add a number of words to "compressed" Log */
void compressed_log_add(struct compressed_log *log, unsigned int *words, int cnt)
{
	int i;

	for (i=0; i<cnt; i++) {
		*log->head = words[i];
		log->head++;
		if ( log->head >= log->end )
			log->head = log->base;
		if ( log->head == log->tail ) {
			log->tail++;
			if ( log->tail >= log->end )
				log->tail = log->base;
		}
	}
}

int compressed_log_take(struct compressed_log *log, unsigned int *words, int max)
{
	int i;

	for (i=0; i<max; i++) {
		if ( log->tail == log->head ) {
			/* log empty */
			break;
		}
		words[i] = *log->tail;
		log->tail++;
		if ( log->tail >= log->end )
			log->tail = log->base;
	}

	return i;
}

/* Add an Control entry to log.
 *
 * CTRL is 29-bits
 */
void compressed_log_add_ctrl(struct compressed_log *log, int ctrl)
{
	unsigned int word = ctrl | 0xe0000000;
	compressed_log_add(log, &word, 1);
}

int dummy(void)
{
	static int i=0;
	return i++;
}

/* Copy Data To a larger software buffer
 *
 * dst       Argument not used
 * src       Source DMA buffer start
 * nentires  Must process the number of entries.
 */
int compressed_log_copy(
	unsigned int dst,
	struct gr1553bm_entry *src,
	int nentries,
	void *data
	)
{
	unsigned int words[3];
	int wc;
	struct bm_logger *bm = data;
	uint64_t currtime, logtime64 = 0, ll_time64, time64;
	unsigned int time24, logtime24;
	int cnt=0;

	/* Sample Current Time */
	gr1553bm_time(bm->bm, &currtime);
	time64 = currtime & ~0x00ffffff;
	time24 = currtime &  0x00ffffff;

	/* Get LastLog Time, but ignore lowest 13 bits */
	ll_time64 = bm->compressed.time & ~0x1fff;

	/* We know that the current time must be
	 * more recent than the time in the logs
	 * since the log length has been prepared
	 * before calling gr1553bm_time() here.
	 *
	 * We also know that this function is
	 * called so often that 24-bit time may
	 * only have wrapped once against current
	 * time sample.
	 */

	for (cnt = 0; cnt < nentries; cnt++) {
		/* Compress One entry */

		/*** Calculate Exact time of one LOG ENTRY ***/

		logtime24 = bm->compressed.time & 0x00ffffff;
		/* Calculate Most significant bits in time */
		if ( logtime24 < time24 ) {
			/* No Wrap occurred */
			logtime64 = time64;
		} else {
			/* Wrap occurred.  */
			logtime64 = time64 - 0x1000000;
		}
		/* Set lowest 24-bits */
		logtime64 |= logtime24;

/* FOR DEBUGGING WHEN BUFFER HAS BEEN FULL.
 *
 * WE SHOULD WRITE ERROR CONTROL WORD HERE.
 */
		if ( bm->compressed.lastlogtime > logtime64 ) {
			/* Stop BM logging */
			*(volatile unsigned int *)0x800005c4 = 0;
			printf("LOG LAST TIME WAS PRIOR:\n");
			printf("  Last: 0x%llx\n", bm->compressed.lastlogtime);
			printf("  Now:  0x%llx:\n", logtime64);
			printf("  time64:  0x%llx:\n", time64);
			printf("  time24:  0x%x:\n", time24);
			printf("  currtime:  0x%llx:\n", currtime);
			printf("  ll_time64:  0x%llx:\n", ll_time64);
			printf("  cnt: %d\n", cnt);
			printf("  words[i-4]: 0x%08x\n", (unsigned int)(src-2)->time);
			printf("  words[i-3]: 0x%08x\n", (unsigned int)(src-2)->data);
			printf("  words[i-2]: 0x%08x\n", (unsigned int)(src-1)->time);
			printf("  words[i-1]: 0x%08x\n", (unsigned int)(src-1)->data);
			printf("  words[i+0]: 0x%08x\n", (unsigned int)src->time);
			printf("  words[i+1]: 0x%08x\n", (unsigned int)src->data);
			dummy();
			asm volatile("ta 0x1\n\t");
		}
		bm->compressed.lastlogtime = logtime64;

		/* Do we need to write down time in a longer format? We
		 * Compare with the time of the last recoded entry.
		 *
		 * The extra 30 bits of time is written
		 * if the 13-bit time is overflown since last
		 * sample.
		 */
		if ( (logtime64 & ~0x1fff) > ll_time64 ) {
			/* Write down long time before data entry
			 * The 2 MSB bits must be 0b10
			 */
			ll_time64 = logtime64 & ~0x1fff;

			/* Get 30-bit MSB Time from 64-bit time */
			words[0] = 0x80000000 | ((logtime64>>13) & 0x3fffffff);
			wc = 1;
		} else {
			wc = 0;
		}

		if ( (src->data & (0x3<<17)) != 0 ) {
			/* Error of some kind. Log error
			 * The 3 MSB bits must be 0b110
			 */
			words[wc] = 0xc0000000 | ((src->data >> 17) & 0x3);
			wc++;
		}
		/* Log Transfer
		 * The MSB bit must be 0
		 */
		words[wc] =
			((logtime64 & 0x1fff)<<18) |	/* 13-bit time */
			((src->data & 0x80000)>>2) |	/* Bus bit */
			(src->data & 0x1ffff);		/* DATA and WTP */
		wc++;

		/* Write Words to buffer */
		compressed_log_add(&bm->compressed, &words[0], wc);

		src++;
	}

	if (nentries) {
		/* Save complete last log time */
		bm->compressed.time = logtime64;
	}

	return 0;
}

/* Set up BM to log eveything */

int bm_init(bm_logger_t *bm, void* log_base, const char *ip, int port, int wait)
{
	int status;
	*bm = calloc(1, sizeof(struct bm_logger));
	if (*bm == NULL ) {
		printf("Failed to allocate BM LOG[%d]\n", 0);
		return -1;
	}
	/* Aquire BM device */
	(*bm)->bm = gr1553bm_open(0);
	if ( !(*bm)->bm ) {
		printf("Failed to open BM[%d]\n", 0);
		free(*bm);
		*bm = NULL;
		return -1;
	}

	if (ip) {
		(*bm)->compressed.base = (unsigned int *)malloc(COMPRESSED_LOG_SIZE);
		if ( (*bm)->compressed.base == NULL ) {
			/* Failed to allocate log buffer */
			gr1553bm_close((*bm)->bm);
			free(*bm);
			*bm = NULL;
			return -2;
		}
		(*bm)->compressed.head = (*bm)->compressed.base;
		(*bm)->compressed.tail = (*bm)->compressed.base;
		(*bm)->compressed.end = (*bm)->compressed.base + COMPRESSED_LOG_CNT;
		(*bm)->compressed.time = 0;
		(*bm)->compressed.lastlogtime = 0;

		/* Add initialial entry in log (START) */
		compressed_log_add_ctrl(&(*bm)->compressed, 0);
	}

	(*bm)->bmcfg.time_resolution = 0;	/* Highest time resoulution */
	(*bm)->bmcfg.time_ovf_irq = 1;	/* Let IRQ handler update time */
	(*bm)->bmcfg.filt_error_options = 0xe; /* Log all errors */
	(*bm)->bmcfg.filt_rtadr = 0xffffffff;/* Log all RTs and Broadcast */
	(*bm)->bmcfg.filt_subadr= 0xffffffff;/* Log all sub addresses */
	(*bm)->bmcfg.filt_mc = 0x7ffff;	/* Log all Mode codes */
	(*bm)->bmcfg.buffer_size = 16*1024;/* 16K buffer */
	(*bm)->bmcfg.buffer_custom = (void *)log_base;	/* Let driver allocate dynamically or custom adr */
	if (ip) {
		(*bm)->bmcfg.copy_func = compressed_log_copy; /* Custom Copying to compressed buffer */
		(*bm)->bmcfg.copy_func_arg = *bm_log;
	} else {
		(*bm)->bmcfg.copy_func = NULL;	/* Standard Copying */
		(*bm)->bmcfg.copy_func_arg = NULL;
	}
	(*bm)->bmcfg.dma_error_isr = NULL;	/* No custom DMA Error IRQ handling */
	(*bm)->bmcfg.dma_error_arg = NULL;


	/* Register standard IRQ handler when an error occur */
	if ( gr1553bm_config((*bm)->bm, &(*bm)->bmcfg) ) {
		printf("Failed to configure BM driver\n");
		gr1553bm_close((*bm)->bm);
		free(*bm);
		*bm = NULL;
		return -3;
	}

	if (ip) {
		/* Set up and start Ethernet server */
		status = server_setup(
				&(*bm)->eth, ip, port, wait,
				(server_get_func)compressed_log_take, &(*bm)->compressed);
		if (status) {
			printf("Failed setting up Ethernet\n");
			gr1553bm_close((*bm)->bm);
			free(*bm);
			*bm = NULL;
			return -4;
		}
	}

	/* Start BM Logging as configured */
	status = gr1553bm_start((*bm)->bm);
	if ( status ) {
		printf("Failed to start BM: %d\n", status);
		gr1553bm_close((*bm)->bm);
		free(*bm);
		*bm = NULL;
		return -4;
	}

	if (ip) {
		uint64_t time1553;
		unsigned int words[2];

		gr1553bm_time((*bm)->bm, &time1553);

		/* Add initialial time entry in log */
		words[0] = 0x80000000 | ((time1553>>13) & 0x3fffffff);
		words[1] = (time1553 & 0x1fff) << 18; /* Empty Data */
		compressed_log_add(&(*bm)->compressed, &words[0], 2);
	}

	return 0;
}

void bm_stop(bm_logger_t bm)
{
	if (bm) {
		gr1553bm_stop(bm->bm);
		if (bm->eth) {
			server_stop(bm->eth);
		}
		printf("BM Entries: %d\n", bm->entry_cnt);
		gr1553bm_close(bm->bm);
		free(bm);
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
