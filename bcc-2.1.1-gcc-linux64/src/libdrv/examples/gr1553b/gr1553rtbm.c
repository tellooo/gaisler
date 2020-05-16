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

/*
 * sample application using the GR1553BC MIL-1553B driver
 *
 *  COPYRIGHT (c) 2018
 *  Cobham Gaisler
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <bcc/bcc.h>

#define TICKS_PER_SEC 100
#define US_TO_TICKS(t) ((t)/10000)
#define SLEEP_TICKS(ticks) do { \
	uint32_t epoch = bcc_timer_get_us(); \
	while ( US_TO_TICKS(bcc_timer_get_us() - epoch) < (ticks) ); \
	/* printf("TICK: %lu\n", US_TO_TICKS(bcc_timer_get_us())); */ \
} while (0)

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define __bsp_gr716__ 1
#else
 #define __bsp_gr716__ 0
#endif

/* Enable/Disable RT Event Log printout */
#define EVLOG_PRINTOUT
/* Enable/disable printout of the RAW EventLog value */
/*#define EVLOG_PRINTOUT_RAW*/

  /* No translation of Data buffers needed */
  #define TRANSLATE(adr) (uint16_t *)(adr)
  /* Let driver dynamically allocate the descriptor table */
#if __bsp_gr716__
  char ev_table_base[1024]    __attribute__((section(".data.ext"),aligned(1024)));
  char sa_table_base[16*32]   __attribute__((section(".data.ext"),aligned(512)));
  char bd_table_base[32*1024] __attribute__((section(".data.ext"),aligned(16)));
  char bd_sw_base[4*1024]     __attribute__((section(".data.ext")));
  #define EV_TABLE_BASE &ev_table_base[0]
  #define SA_TABLE_BASE &sa_table_base[0]
  #define BD_TABLE_BASE &bd_table_base[0]
  #define BD_SW_BASE    &bd_sw_base[0]
#else
  #define EV_TABLE_BASE NULL
  #define SA_TABLE_BASE NULL
  #define BD_TABLE_BASE NULL
  #define BD_SW_BASE    NULL
#endif
  #define RT_DATA_HW_ADR &RT_data
  #define RT_DATA_CPU_ADR RT_DATA_HW_ADR
  /* Dynamically allocate (BM) LOGGING BASE ADDRESS */
#if __bsp_gr716__
  char bm_log_base[128*1024] __attribute__((section(".data.ext"),aligned(8)));
  #define BM_LOG_BASE &bm_log_base[0]
#else
  #define BM_LOG_BASE NULL
#endif

#define DEBUG 0
#if defined(DEBUG) && DEBUG
#define printk(...) iprintf(__VA_ARGS__)
#else
#define printk(...)
#endif

#include "bm_logger.h"

int rt_init_list(int *err);
int rt_init(void);
int rt_loop();
int rt_stop();
void *rt = NULL;
bm_logger_t bm;

int gr1553rtbm_test(void)
{
	int retval = 0;

	if ( rt_init() ) {
		printf("Failed to initialize RT\n");
		retval = -1;
		goto end;
	}

	if ( bm_init(&bm, BM_LOG_BASE) ) {
		printf("Failed to initialize RT\n");
		retval = -2;
		goto end;
	}

	if ( rt_loop() ) {
		printf("Failed to execute RT\n");
		retval = -3;
		goto end;
	}

end:
	rt_stop();
	bm_stop(bm);
	return retval;
}

#include <drv/gr1553rt.h>
#include "pnp1553.h"

void rt_sa3_rx_isr(struct gr1553rt_list *list, unsigned int ctrl,
			int entry_next, void *data);
void rt_sa3_tx_isr(struct gr1553rt_list *list, unsigned int ctrl,
			int entry_next, void *data);

struct gr1553rt_cfg rtcfg =
{
	.rtaddress = 5,

	/* Mode code config: let all pass and be logged */
	.modecode = 0x2aaaaaaa,

	/* Highest time resolution */
	.time_res = 0,

	/* 512Byte SubAddress table, use malloc() */
	.satab_buffer = SA_TABLE_BASE,

	/* Event Log config */
	.evlog_buffer = EV_TABLE_BASE,
	.evlog_size = 1024,

	/* Tranfer descriptors config */
	.bd_count = 1024,
	.bd_buffer = BD_TABLE_BASE,
	.bd_sw_buffer = BD_SW_BASE,
};


/* SUBADDRESS 1, BUS STATE BROADCASTED FROM BC:
 *  RX: WAIT/STARTUP/RUNNING/SHUTDOWN  (FROM BC)
 *  TX: DOWN/UP/DOWN              (RT INDICATE RUNNING STATUS TO BC)
 */
struct gr1553rt_list *sa1rx_list = NULL, *sa1tx_list = NULL;
struct gr1553rt_list_cfg sa1_cfg =
{
	.bd_cnt = 1,
};

/* SUBADDRESS 2, Plug&Play information:
 *  TX: VENDOR|DEVICE|VERSION|CLASS|STRING
 *  RX: NOTHING
 */
struct gr1553rt_list *sa2tx_list = NULL;
struct gr1553rt_list_cfg sa2_cfg =
{
	.bd_cnt = 1,
};


/* SUBADDRESS 3, BC<->RT Data transfers. ~1kB/sec:
 *
 *  RX: BC Transfer Data in 64byte block.
 *  TX: Received BC data is copied here.
 */
struct gr1553rt_list *sa3tx_list = NULL;
struct gr1553rt_list *sa3rx_list = NULL;

struct gr1553rt_list_cfg sa3_cfg =
{
	.bd_cnt = 16, /* two per major frame */
};

#if __bsp_gr716__
char sa1rx_base[GR1553RT_LIST_SIZE(1)]  __attribute__((section(".data.ext")));
char sa1tx_base[GR1553RT_LIST_SIZE(1)]  __attribute__((section(".data.ext")));
char sa2tx_base[GR1553RT_LIST_SIZE(1)]  __attribute__((section(".data.ext")));
char sa3rx_base[GR1553RT_LIST_SIZE(16)] __attribute__((section(".data.ext")));
char sa3tx_base[GR1553RT_LIST_SIZE(16)] __attribute__((section(".data.ext")));
#endif

int rxbuf_curr = 0;

/* All RT Data buffers */
struct rt_data_buffer {
	/* Startup/Shutdown parameters the BC control */
	unsigned short bus_status; /* Wait */
	unsigned short rt_status; /* Down */

	/* Information about RT device that BC read */
	struct pnp1553info pnp1553_info;

	/* Transfer buffers */
	uint16_t rxbufs[16][32];
	uint16_t txbufs[16][32];
};

/* Initial State of RT Data */
struct rt_data_buffer RT_data_startup =
{
	.bus_status = 0,
	.rt_status = 0,
	.pnp1553_info =
	{
		.vendor = 0x0001,
		.device = 0x0001,
		.version = 0,
		.class = 0,
		.subadr_rx_avail = 0x0003,
		.subadr_tx_avail = 0x0007,
		.desc = "GAISLER RTEMS DEMO1",
	},
};

#ifndef AMBA_OVER_PCI
	/* Data Buffers the RT Hardware Access */
#if __bsp_gr716__
	struct rt_data_buffer RT_data __attribute__((section(".data.ext")));
#else
	struct rt_data_buffer RT_data;
#endif
#endif
/* Pointer to RT_Data structure. For AMBA-over-PCI the pRT_data pointer
 * should be automatically be calculated.
 */
struct rt_data_buffer *pRT_data;
struct rt_data_buffer *pRT_data_hw;

int rt_init_list(int *err)
{
	int i;
	int irq, next;

	/*** SUBADDRESS 1 - BUS STATUS ***/

	/* Make driver allocate list description */
#if __bsp_gr716__
	sa1rx_list = (struct gr1553rt_list *)&sa1rx_base[0];
	sa1tx_list = (struct gr1553rt_list *)&sa1tx_base[0];
	if ( (*err = gr1553rt_list_init(rt, &sa1rx_list, &sa1_cfg)) ) {
		return -1;
	}
	if ( (*err = gr1553rt_list_init(rt, &sa1tx_list, &sa1_cfg)) ) {
		return -2;
	}
#else
	sa1rx_list = NULL;
	sa1tx_list = NULL;
	if ( (*err = gr1553rt_list_alloc(rt, &sa1rx_list, &sa1_cfg)) ) {
		return -1;
	}
	if ( (*err = gr1553rt_list_alloc(rt, &sa1tx_list, &sa1_cfg)) ) {
		return -2;
	}
#endif
	/* Setup descriptors to receive STATUS WORD and transmit current
	 * RT status.
	 *
	 * RX and TX: circular ring with one descriptor.
	 */
	if ( (*err = gr1553rt_bd_init(sa1rx_list, 0, 0, &pRT_data_hw->bus_status, 0)) )
		return -3;
	if ( (*err = gr1553rt_bd_init(sa1tx_list, 0, 0, &pRT_data_hw->rt_status, 0)) )
		return -4;

	/*** SUBADDRESS 2 - PnP READ ***/
#if __bsp_gr716__
	sa2tx_list = (struct gr1553rt_list *)&sa2tx_base[0];
	if ( (*err = gr1553rt_list_init(rt, &sa2tx_list, &sa2_cfg)) ) {
		return -10;
	}
#else
	sa2tx_list = NULL;
	if ( (*err = gr1553rt_list_alloc(rt, &sa2tx_list, &sa2_cfg)) ) {
		return -10;
	}
#endif
	if ( (*err = gr1553rt_bd_init(sa2tx_list, 0, 0,
	                         (uint16_t *)&pRT_data_hw->pnp1553_info, 0)) )
		return -11;

	/*** SUBADDRESS 3 - BC<->RT DATA Transfer ***/

#if __bsp_gr716__
	sa3tx_list = (struct gr1553rt_list *)&sa3tx_base[0];
	sa3rx_list = (struct gr1553rt_list *)&sa3rx_base[0];
	if ( (*err = gr1553rt_list_init(rt, &sa3tx_list, &sa3_cfg)) ) {
		return -20;
	}
	if ( (*err = gr1553rt_list_init(rt, &sa3rx_list, &sa3_cfg)) ) {
		return -21;
	}
#else
	sa3tx_list = NULL;
	sa3rx_list = NULL;
	if ( (*err = gr1553rt_list_alloc(rt, &sa3tx_list, &sa3_cfg)) ) {
		return -20;
	}
	if ( (*err = gr1553rt_list_alloc(rt, &sa3rx_list, &sa3_cfg)) ) {
		return -21;
	}
#endif
	/* Install IRQ handlers for RX/TX (only RX is used) */
	if ( gr1553rt_irq_sa(rt, 3, 1, rt_sa3_tx_isr, rt) ) {
		return -22;
	}
	if ( gr1553rt_irq_sa(rt, 3, 0, rt_sa3_rx_isr, rt) ) {
		return -22;
	}
	/* Transmit And Receive list */
	for ( i=0; i<16; i++) {
		/* Next Descriptor, with wrap-around */
		next = i + 1;
		if ( next == 16 )
			next = 0;

		/* Enable RX IRQ when last transfer within an Major
		 * frame is received. We have up 25ms to prepare
		 * the data the BC will request.
		 */
		irq = 0;
		if ( i & 1 )
			irq = GR1553RT_BD_FLAGS_IRQEN;

		if ( (*err = gr1553rt_bd_init(sa3tx_list, i, 0,
                             &pRT_data_hw->txbufs[i][0], next)) ) {
			return -30;
		}

		if ( (*err = gr1553rt_bd_init(sa3rx_list, i, irq,
                              &pRT_data_hw->rxbufs[i][0], next)) ) {
			return -31;
		}
	}

	return 0;
}

struct rt_sas_config {
	unsigned int mask;
	unsigned int opts;
	struct gr1553rt_list **rxlist;
	struct gr1553rt_list **txlist;
} rtsa_cfg[32] =
{
/* SEE HW MANUAL FOR BIT DEFINITIONS
 *
 * Mode code:      all give IRQ and is logged.
 *
 * Non-mode codes: are not logged or IRQed by default, only
 *                 when explicitly defined by descriptor config.
 */
	/* 00 */ {0xffffffff, 0x00000, NULL, NULL},	/* Mode code - ignored */
	/* 01 */ {0xffffffff, 0x38181, &sa1rx_list, &sa1tx_list}, /* Startup/shutdown sub address */
	/* 02 */ {0xffffffff, 0x39090, NULL, &sa2tx_list}, /* PnP: VENDOR|DEVICE|VERSION|CLASS... Limit to 32 byte */
	/* 03 */ {0xffffffff, 0x38080, &sa3rx_list, &sa3tx_list}, /* 64byte Data transfers */
	/* 04 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 05 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 06 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 07 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 08 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 09 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 10 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 11 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 12 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 13 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 14 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 15 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 16 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 17 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 18 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 19 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 20 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 21 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 22 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 23 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 24 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 25 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 26 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 27 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 28 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 29 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 30 */ {0xffffffff, 0x00000, NULL, NULL},
	/* 31 */ {0xffffffff, 0x1e0e0, NULL, NULL},	/* Mode code - ignored */
};

void rt_init_sa(void)
{
	int i;

	for (i=0; i<32; i++) {
		gr1553rt_sa_setopts(rt, i, rtsa_cfg[i].mask, rtsa_cfg[i].opts);
		if ( rtsa_cfg[i].rxlist )
			gr1553rt_sa_schedule(rt, i, 0, *rtsa_cfg[i].rxlist);
		if ( rtsa_cfg[i].txlist )
			gr1553rt_sa_schedule(rt, i, 1, *rtsa_cfg[i].txlist);
	}
}

int rt_irq_cnt=0;

unsigned int rt_isr(struct gr1553rt_list *list, int entry, void *data)
{
	(void)entry;
	(void)data;
	rt_irq_cnt++;

	if ( list == sa3rx_list ) {

	} else if ( list == sa3tx_list ) {

	}

	/* Default action is to clear the DATA-VALID flag and
	 * TIME, BC, SZ, RES
	 */
	return 0x83ffffff;
}

/* ERROR IRQ (DMA Error or RT Table access error) */
void rt_err_isr(int err, void *data)
{
	(void)err;
	(void)data;
	printk("ERROR IRQ: 0x%x\n", err);
}

/* Mode Code Received */
void rt_mc_isr(int mcode, unsigned int entry, void *data)
{
	(void)mcode;
	(void)entry;
	(void)data;
	printk("MC%d IRQ: 0x%08x\n", mcode, entry);
}

int rt_init(void)
{
	int status, err;

	pRT_data = RT_DATA_CPU_ADR;
	pRT_data_hw = RT_DATA_HW_ADR;

	/* Initialize default values of data buffers */
	memcpy(pRT_data, &RT_data_startup, sizeof(struct rt_data_buffer));

	rxbuf_curr = 0;

	/* Print List:
	 *   gr1553bc_show_list(list, 0);
	 */

	/* Aquire RT device */
	rt = gr1553rt_open(0);
	if ( !rt ) {
		printf("Failed to open RT[%d]\n", 0);
		return -1;
	}

	/* Configure driver before setting up lists */
#if __bsp_gr716__
	if ( gr1553rt_config_init(rt, &rtcfg) ) {
		printf("Failed to configure RT driver\n");
		return -1;
	}
#else
	if ( gr1553rt_config_alloc(rt, &rtcfg) ) {
		printf("Failed to configure RT driver\n");
		return -1;
	}
#endif
	/* Assign Error IRQ handler */
	if ( gr1553rt_irq_err(rt, rt_err_isr, rt) ) {
		printf("Failed to register ERROR IRQ function\n");
		return -1;
	}

	/* Assign ModeCode IRQ handler */
	if ( gr1553rt_irq_mc(rt, rt_mc_isr, rt) ) {
		printf("Failed to register ERROR IRQ function\n");
		return -1;
	}

	/* Set up lists and schedule them on respective RT subaddress.
	 * Also, register custom IRQ handlers on some transfer
	 * descriptors.
	 */
	if ( (status=rt_init_list(&err)) != 0 ) {
		printf("Failed to init lists: %d : %d\n", status, err);
		return -1;
	}

	/* Set up configuration options per RT sub-address */
	rt_init_sa();

	/* Start communication */
	status = gr1553rt_start(rt);
#if 0
	/* Register standard IRQ handler when an error occur */
	if ( gr1553bc_irq_setup(bc, bc_isr, bc) ) {
		printf("Failed to register standard IRQ handler\n");
		return -2;
	}

	/* Start previously created BC list */
	status = gr1553bc_start(bc, list, NULL);
	if ( status ) {
		printf("Failed to start BC: %d\n", status);
		return -3;
	}

	printf("            MAJOR MINOR SLOT\n");
	while ( 1 ) {
		if ( gr1553bc_indication(bc, &mid) ) {
			printf("Error getting current MID\n");
			gr1553bc_show_list(list, 0);
			return -1;
		}

		printf("INDICATION: %02x %02x %02x\n",
			GR1553BC_MAJID_FROM_ID(mid),
			GR1553BC_MINID_FROM_ID(mid),
			GR1553BC_SLOTID_FROM_ID(mid));

		if ( bm_log() ) {
			printf("BM Log failed\n");
			return -2;
		}
		rtems_task_wake_after(10);
		/*sleep(1);*/
	}
#endif
	return 0;
}

int rt_stop(void)
{
	if (rt) {
		gr1553rt_close(rt);
		rt = NULL;
	}

	return 0;
}

int state = 0;
int init_state = 0;

int rt_startup(void)
{
	printf("RT Starting up: %04hx\n", pRT_data->bus_status);

	return 0;
}

int rt_run()
{
	printf("Bus in running mode: %04hx\n", pRT_data->bus_status);

	return 0;
}

int rt_shutdown()
{
	printf("Bus in shutdown mode: %04hx\n", pRT_data->bus_status);

	return 0;
}

int rt_init_state(void)
{
	switch ( init_state ) {
	default:
	case 0:	/* Wait for Startup Message */

		/* Check if BC wants us to startup */
		if ( (0x00ff & pRT_data->bus_status) > 0 ) {

			/* Try to startup RT */
			if ( rt_startup() ) {
				printf("RT Startup failed\n");
				return -1;
			}

			/* Step to next state */
			init_state = 1;

			/* Signal to BC we have started up */
			pRT_data->rt_status = 1;
		}
		break;

	case 1:
		/* Check if BC say all RTs are started up and ready to go */

		if ( (0x00ff & pRT_data->bus_status) > 1 ) {
			if ( rt_run() ) {
				printf("RT Run mode failed\n");
				return -1;
			}
			init_state = 1;
			return 1;
		}
		break;
	}

	return 0;
}

/* Check new Data has arrived from BC. In that case data is copied
 * to RT transmit.
 */
int bc_rt_data_transfer(void)
{
#if 0
	int curr;

	/* Do nothing, our ISR handles data copying */

	/* Get Received data by looking at the current descriptor address
	 *
	 */
	gr1553rt_indication(rt, 3, NULL, &curr);

	/* Have we received more data? */
	if ( curr == rxbuf_curr ) {
		return;
	}

	while ( curr != rxbuf_curr ) {

		/* Handle buffer 'rxbuf_curr' */
		memcpy(pRT_data->txbufs[rxbuf_curr], pRT_data->rxbufs[rxbuf_curr], 64);

		/* Next buffer will be */
		rxbuf_curr++;
		if ( rxbuf_curr >= 16 ) {
			rxbuf_curr = 0;
		}
	}
#endif

	if ( (0x00ff & pRT_data->bus_status) >= 3 ) {
		/* Shutdown */
		return 1;
	}
	return 0;
}

#ifdef EVLOG_PRINTOUT
void rt_process_evlog(void)
{
	unsigned int events[64], ev;
	int i, cnt, type, samc, bc, size, result, irq;
	char *type_str, *bc_str, *result_str, *irq_str;

	do {
		/* Get up to 64 events from Event log
		 *
		 * This method can be used to handle logged transmissions
		 * for non-interrupted or together with interrupted
		 * transmissions.
		 */
		cnt = gr1553rt_evlog_read(rt, events, 64);
		if ( cnt < 1 )
			break;

		/* Process the entries */
		for ( i=0; i<cnt; i++) {
			ev = events[i];

			/* Decode */
			irq = ev >> 31;
			type = (ev >> 29) & 0x3;
			samc = (ev >> 24) & 0x1f;
			bc =  (ev >> 9) & 0x1;
			size = (ev >> 3) & 0x3f;
			result =  ev & 0x7;

			bc_str = "";
			irq_str = "";
			result_str = "";

			if ( bc )
				bc_str = "BC ";
			if ( result != 0 )
				result_str = " ERROR";
			if ( irq )
				irq_str = "IRQ ";

			switch ( type ) {
				case 0:
					type_str = "TX";
					break;

				case 1:
					type_str = "RX";
					break;

				case 2:
					type_str = "MC";
					break;

				default:
					type_str = "UNKNOWN";
					break;
			}
#ifdef EVLOG_PRINTOUT_RAW
			printf("EV: %s%02x: %s%slen=%d%s (%08x)\n",
				type_str, samc, irq_str, bc_str, size,
				result_str, ev);
#else
			printf("EV: %s%02x: %s%slen=%d%s\n",
				type_str, samc, irq_str, bc_str, size,
				result_str);
#endif
		}

	} while ( cnt == 64 );
}
#endif

int rt_debug_majfrm = 0;
int rt_debug_blockno = 0;
int rt_sa3_irqs = 0;

void rt_sa3_rx_isr
	(
	struct gr1553rt_list *list,
	unsigned int ctrl,
	int entry_next,
	void *data
	)
{
	(void)list;
	(void)ctrl;
	(void)entry_next;
	(void)data;
	static int last_entry = 0;

	/* We have received two blocks of data: 128-bytes total, located
	 * in our receive buffers: rxbufs[MAJOR_FRAME][block].
	 *
	 * Copy received data to transmit buffers
	 */
	int majfrm = last_entry / 2;
	int blockno = last_entry & 1;
	unsigned int status;

	/* Re enable RX IRQ */
	status = GR1553RT_BD_FLAGS_IRQEN;
	gr1553rt_bd_update(sa3rx_list, last_entry+1, &status, NULL);

	memcpy(&pRT_data->txbufs[last_entry+0][0], &pRT_data->rxbufs[last_entry+0][0], 64);
	memcpy(&pRT_data->txbufs[last_entry+1][0], &pRT_data->rxbufs[last_entry+1][0], 64);

	last_entry += 2;
	if ( last_entry >= 16 )
		last_entry = 0;

	rt_debug_majfrm = majfrm;
	rt_debug_blockno = blockno;

	rt_sa3_irqs++;
}

/* Descriptors or SA-table do not have Interrupts enabled, so we will not
 * end up here.
 */
void rt_sa3_tx_isr
	(
	struct gr1553rt_list *list,
	unsigned int ctrl,
	int entry_next,
	void *data
	)
{
	(void)list;
	(void)ctrl;
	(void)entry_next;
	(void)data;
}

int rt_loop(void)
{
	int status;
	int quit = 0;

	state = 0;
	init_state = 0;

	printf("Switching to Wait Run State\n");
	while (!quit) {
		/* Answer BC's requests */
		switch ( state ) {
		default:
		case 0: /* Initial State */
			status = rt_init_state();
			if ( status < 0 ) {
				return -1;
			} else if ( status > 0 ) { /* Done with INIT - Switch? */
				printf("Switching to Communication State\n");
				state = 1;
			}
			break;

		case 1: /* Communication State */
			status = bc_rt_data_transfer();
#ifdef EVLOG_PRINTOUT
			rt_process_evlog();
#endif
			if (status == 1) {
				state = 2;
			}
			break;

		case 2: /* Shutdown state */
			rt_shutdown();
			quit = 1;
			break;
		}

		if ( bm_log(bm) ) {
			printf("BM Log failed\n");
			return -2;
		}

		/* Sleep 1 ticks */
		SLEEP_TICKS(1);
	}

	return 0;
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
        return gr1553rtbm_test();
}
