/*
 * RTEMS SpaceWire Time Distribution Protocol (SPWTDP) core example. The
 * example communicates with a GRLIB target including a GRSPW2 and SPWTDP
 * core using RMAP, TimeCodes and SpW Interrupts. The local SPWTDP is operated
 * in initiator mode and the remote SPWTDP in target mode. This software
 * demonstrates to following:
 *  - Initiator SPWTDP initialization using SPWTDP driver
 *  - Initiator GRSPW2 initialization using GRSPW_PKT driver
 *  - Initiator GRSPW2 link control and DMA handling using GRSPW_PKT driver
 *  - RMAP communication to Target using GRSPW_PKT driver and RMAP-ASYNC stack
 *  - Target SPWTDP initialization over RMAP
 *  - Target GRSPW2 initialization over RMAP
 *  - Basic Elapsed Time format handling
 *  - Rough startup synchonization between Initiator RTEMS System Clock
 *    and SPWTDP
 *  - Latency Calculation
 *
 * The local and remote hardware required:
 *  - GRSPW2 with Distributed Interrupt Support
 *  - SPWTDP connected to GRSPW2
 *  - Initiator SpW0 connector must be connected to SpW1 connector of Target
 *  - SPWTDP of Initiator and Target use the same configuration
 *  - Maximum 64-bits fine time resolution
 *  - Latency value calculated is less than two octet of fine time
 *
 * The application consists of four threads:
 *
 *  INIT. Initialization and start task. Make basic initialization and
 *        creates the below tasks:
 *
 *  TA01. Example Task
 *
 *  TA02. Link monitor task. Prints out whenever a SpaceWire link switch
 *        from run-state to any other state or vice versa.
 *
 *  TA03. SpaceWire DMA task. Handles reception and transmission of SpaceWire
 *        packets on all SpaceWire devices.
 *
 * Other RTEMS objects:
 *
 *  SEM0. DMA semaphore to protect DMA RX and TX packet lists. The DMA task
 *        responds to changes and handles the DMA RX/TX operations that the
 *        example task TA01 makes. The TA01 task generates RMAP requests into
 *        DMA TX queue and processes RX queue for incomming RMAP responses.
 *
 */

/****** CONFIGURATION OF SPWTDP EXAMPLE *****
 *
 * Remote SpW-TDP Settings:
 */
#define R_SPWTDP_DSTADR 0xfe /* SpW Destination address */
#define R_SPWTDP_DSTKEY 0x00 /* SpW Destination Key */
//#define R_SPWTDP_BASEADR 0x80100400 /* Base address of SPWTDP on Remote */
#define R_SPWTDP_BASEADR 0xffa0c000 /* Base address of SPWTDP on Remote */
#define R_SPWTDP_SPWDEV  0 /* SpaceWire device connector */
#define R_SPWTDP_MAPPING 6 /* Mapping of Time-Code to T-Field */
//#define R_SPW_BASEADR    0x80000a00 /* SpaceWire device 1 base address */
#define R_SPW_BASEADR    0xff90d000 /* SpaceWire device 1 base address */

//#define GR740_MASTER
//#define FPGA_SLAVE
//#define FPGA_MASTER
//#define GR740_SLAVE

/* GRSPW device used to communicate with target. Note only SpW0 tested. */
#define DEV_IDX 0
#define RMAP_SPW_DEV_IDX DEV_IDX

/* Number of latency calculations */
//#define LATENCY_COUNT 128
#define LATENCY_COUNT 64
//#define LATENCY_COUNT 2

/* READ/WRITE Target SPWTDP over RMAP end demonstration */
#undef TOGGLE_TGT_ME

/* Enable to debug SpW Traffic */
#ifdef DEBUG_SPW
#define DEBUG_SPW_TX
#define DEBUG_SPW_RX
#endif

/* Add print outs for latency calculation. Warning! this might make the
 * calculation invlaid since printf() may take too long time. Works using a
 * Ethernet debuglink and doing -u UART tunneling with GRMON2.
 */
#undef DEBUG_LATENCY

/* Debug option to see when the Datation ET are sampled localy for Latency
 * calculation. A GPTIMER timer incatnce is programmed and used for the
 * sampling.
 */
#undef DEBUG_ET_SAMPLING_TIMES
/* Base address of GPTIMER timer used to sample time. Could use
 * GPTIMER->timer[2] to be more Plug&Play able */
#define TIMER_BASE 0x80000320

/* Define INCLUDE_PCI_DRIVERS to include PCI host and PCI board drivers with
 * SpaceWire interfaces
 */
/*#define INCLUDE_PCI_DRIVERS*/

#include <rtems.h>

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

/* Set some values if someone should modify the example. The shared IRQ layer
 * need one semaphore.
 */
#define CONFIGURE_MAXIMUM_TASKS             8
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_DRIVERS 32
#define CONFIGURE_MAXIMUM_PERIODS             1

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES    RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT
#define CONFIGURE_EXTRA_TASK_STACKS         (40 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MICROSECONDS_PER_TICK     RTEMS_MILLISECONDS_TO_MICROSECONDS(2)


/* Configure Driver manager */
#if defined(RTEMS_DRVMGR_STARTUP) && defined(LEON3) /* if --drvmgr was given to configure */
/* Add Timer and UART Driver for this example */
#ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#endif
#ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#endif
#endif

#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW2     /* SpaceWire Packet driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPW_ROUTER /* SpaceWire Router  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPWTDP     /* SpaceWire TDP driver */

#ifdef INCLUDE_PCI_DRIVERS
/* Configure PCI Library to auto configuration. This can be substituted with
 * a static configuration by setting PCI_LIB_STATIC, see pci/. Static
 * configuration can be generated automatically by print routines in PCI
 * library.
 */
#define RTEMS_PCI_CONFIG_LIB
/*#define CONFIGURE_PCI_LIB PCI_LIB_STATIC*/
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO

/*#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF*//* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#endif

/*******************************************/

#ifdef LEON2
/* PCI support for AT697 */
#define CONFIGURE_DRIVER_LEON2_AT697PCI
/* AMBA PnP Support for GRLIB-LEON2 */
/*#define CONFIGURE_DRIVER_LEON2_AMBAPP*/
#endif

#include <rtems/confdefs.h>
#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* no network */
#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "../../config.c"
#include "spwtdp_lib.h"

extern int router_setup_custom(void);

rtems_task test_app(rtems_task_argument ignored);
rtems_id tid, tid_link, tid_dma;
rtems_id dma_sem;

rtems_task Init(
		rtems_task_argument ignored
		)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	/*drvmgr_print_topo();*/
	rtems_task_wake_after(4);

	/* Create Tasks for example application */
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '1' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid);
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '2' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid_link);
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '3' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid_dma);

	/* Device Semaphore created with count = 1 */
	if (rtems_semaphore_create(rtems_build_name('S', 'E', 'M', '0'), 1,
				RTEMS_FIFO | RTEMS_SIMPLE_BINARY_SEMAPHORE | \
				RTEMS_NO_INHERIT_PRIORITY | RTEMS_LOCAL | \
				RTEMS_NO_PRIORITY_CEILING, 0, &dma_sem) != RTEMS_SUCCESSFUL) {
		printf("Failed creating Semaphore\n");
		exit(0);
	}

	/* Configure SPW router */
	printf("Setting up SpaceWire router\n");
	if (router_setup_custom()){
		printf("Failed router initialization.\n");
	}

	rtems_task_start(tid, test_app, 0);
	rtems_task_suspend(RTEMS_SELF);
}

#include <grlib/grspw_pkt.h>
#include "grspw_pkt_lib.h"

#include "rmap.h"


int nospw = 0;
int tasks_stop = 0;
rtems_task link_ctrl_task(rtems_task_argument unused);
rtems_task dma_task(rtems_task_argument unused);

#define PKT_SIZE 256

struct grspw_device {
	/* GRSPW Device layout - must be the same as 'struct grspw_dev' */
	void *dh;
	void *dma[4];
	int index;
	struct grspw_hw_sup hwsup;

	/* Test structures */
	struct grspw_config cfg;
	int run;

	/* RX and TX lists with packet buffers */
	struct grspw_list rx_list, rx_buf_list, tx_list, tx_buf_list;
	int rx_list_cnt, rx_buf_list_cnt, tx_list_cnt, tx_buf_list_cnt;
};
#define DEV(device) ((struct grspw_dev *)(device))

#define DEVS_MAX 32
static struct grspw_device devs[DEVS_MAX];

struct grspw_config dev_def_cfg =
{
	.adrcfg =
	{
		.promiscuous = 1, /* Detect all packets */
		.def_addr = 32, /* updated bu dev_init() */
		.def_mask = 0,
		.dma_nacfg =
		{
			/* Since only one DMA Channel is used, only
			 * the default Address|Mask is used.
			 */
			{
				.node_en = 0,
				.node_addr = 0,
				.node_mask = 0,
			},
			{
				.node_en = 0,
				.node_addr = 0,
				.node_mask = 0,
			},
			{
				.node_en = 0,
				.node_addr = 0,
				.node_mask = 0,
			},
			{
				.node_en = 0,
				.node_addr = 0,
				.node_mask = 0,
			},
		},
	},
	//.rmap_cfg = 0, /* Disable RMAP */
	.rmap_cfg = 1, /* Enable RMAP */
	.rmap_dstkey = 0, /* No RMAP DESTKEY needed when disabled */
	.tc_cfg = TCOPTS_EN_TX,/* Enable TimeCode */
	.tc_isr_callback = NULL,/* TimeCode ISR */
	.tc_isr_arg = NULL,	/* No TimeCode ISR Argument */
	.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
	.chan =
	{
		{
			.flags = DMAFLAG_NO_SPILL,
			.rxmaxlen = PKT_SIZE,
			.rx_irq_en_cnt = 0, /* Disable RX IRQ generation */
			.tx_irq_en_cnt = 0, /* Disable TX IRQ generation */
		},
		/* The other 3 DMA Channels are unused */

	},
	.iccfg =
	{
		.tomask = 0,
		.aamask = 0,
		.scaler = 0,
		.isr_reload = (1 << 31),
		.ack_reload = (1 << 31),
	},
};

/****** SPW route configuration
 *
 * Please make sure to set up this correctly in order to route the RMAP packets to the destination. The actual route assumes the following:
 * 1-> INITIATOR-BOARD SPW port labeled as SPW_PORT_INI is connected to SPW_PORT_TAR in TARGET-BOARD.
 * 2-> Both boards use the first AMBA_PORT, i.e. AMBA_PORT_INI and AMBA_PORT_TAR.
 * 3-> The route from initiator to target has two routing points (where the routers are). *
 * INIIATOR-BOARD-->AMBA_PORT_INI-->ROUTER_INI-->SPW_PORT_INI-->CABLE-->SPW_PORT_TAR-->ROUTER_TAR-->AMBA_PORT_TAR-->TARGET-BOARD
 * Those two routing points are: Choose i) SPW_PORT_INI in ROUTER_INI and ii) AMBA_PORT_TAR in ROUTER_TAR, therefore the route is SPW_PORT_INI, AMBA_PORT_TAR.
 * 4-> The route back from target to initiator has two routing points (where the routers are). *
 * TARGET-BOARD-->AMBA_PORT_TAR-->ROUTER_TAR-->SPW_PORT_TAR-->CABLE-->SPW_PORT_INI-->ROUTER_INI-->AMBA_PORT_INI-->INITIATOR-BOARD
 * Those two routing points are: Choose i) SPW_PORT_TAR in ROUTER_TAR and ii) AMBA_PORT_INI in ROUTER_INI, therefore the route is SPW_PORT_TAR, AMBA_PORT_INI.
 *
 * 5-> Make sure that the NOCRC_FLAG is chosen according to the ROUTE SIZE. In the example above, the route has 2 addresses so NOCRC_FLAG has to be 2, in this case,
 * NOCRC_FLAG = TXPKT_FLAG_NOCRC_LEN2. Maximum ROUTE size is 16
 * */


/*  Path Table */
struct route_entry {
	unsigned char nr; /* Number of addresses in dstadr array */
	unsigned char dstadr[16]; /* Path Addresses */
};

#if defined(GR740_MASTER)
#define SPW_PORT_INI 1
#define AMBA_PORT_INI 9 /* For DEV_IDX=0, AMBA_PORT=9 */ 
#elif defined(FPGA_MASTER)
#define SPW_PORT_INI 1
#define AMBA_PORT_INI 5 /* For DEV_IDX=0, AMBA_PORT=5 */ 
#else
#error "Wrong master"
#endif
#if defined(GR740_SLAVE)
#define SPW_PORT_TAR 1
#define AMBA_PORT_TAR 9 /* For DEV_IDX=0, AMBA_PORT=9 */ 
#elif defined(FPGA_SLAVE)
#define SPW_PORT_TAR 1
#define AMBA_PORT_TAR 5 /* For DEV_IDX=0, AMBA_PORT=5 */ 
#else
#error "Wrong master"
#endif

#define NOCRC_FLAG TXPKT_FLAG_NOCRC_LEN2
/* Here the routes are defined */
#define ROUTE_TO_SIZE 2
#define ROUTE_FROM_SIZE 2
#define ROUTE_MAX 2
#define ROUTE_TO 0
#define ROUTE_FROM 1
struct route_entry routetab[ROUTE_MAX] = {
	/* routetab[entry]         {NUMADD, {ROUTEADD1, ROUTEADD2, ...}}, */
	/* routetab[ROUTE_TO]   */ {ROUTE_TO_SIZE, {SPW_PORT_INI, AMBA_PORT_TAR, 0}},
	/* routetab[ROUTE_FROM] */ {ROUTE_FROM_SIZE, {SPW_PORT_TAR, AMBA_PORT_INI, 0}}
};
int path_addressing=1;
/* If path addressing is not used, a proper router configuration is needed to route the src and dst logical addresses to their correct paths.
 * Such functionality is not included in this example. */

/* SpaceWire packet payload (data) content layout */
struct pkt_hdr {
	unsigned char addr;
	unsigned char protid;
	unsigned int data[(PKT_SIZE-2)/4];
};

struct spwpkt {
	struct grspw_pkt p;
	unsigned long long data[PKT_SIZE/8+1]; /* 32 bytes of data - 2byte data-header (8 extra bytes to avoid truncated bad packets)*/
	unsigned long long hdr[256/8+2]; /* Max header size and extra room for RMAP stack */
};

/* All packet buffers used by application */
struct spwpkt pkts[DEVS_MAX][16];

void init_pkts(void)
{
	struct spwpkt *pkt;
	int i, j;

	memset(&pkts[0][0], 0, sizeof(pkts));

	for (i = 0; i < DEVS_MAX; i++) {
		grspw_list_clr(&devs[i].rx_list);
		grspw_list_clr(&devs[i].tx_list);
		grspw_list_clr(&devs[i].tx_buf_list);
		devs[i].rx_list_cnt = 0;
		devs[i].tx_list_cnt = 0;
		devs[i].rx_buf_list_cnt = 0;
		devs[i].tx_buf_list_cnt = 0;
		for (j = 0, pkt = &pkts[i][0]; j < 16; j++, pkt = &pkts[i][j]) {
			pkt->p.pkt_id = (i << 8)+ j; /* unused */
			pkt->p.data = &pkt->data[0];
			pkt->p.hdr = &pkt->hdr[0];
			if (j < 8) {
				/* RX buffer */

				/* Add to device RX list */
				grspw_list_append(&devs[i].rx_buf_list, &pkt->p);
				devs[i].rx_buf_list_cnt++;
			} else {
				/* TX buffer */
				pkt->p.dlen = 0;
				pkt->p.hlen = 0;

				/* Add to device TX list */
				grspw_list_append(&devs[i].tx_buf_list, &pkt->p);
				devs[i].tx_buf_list_cnt++;
			}
		}
	}
}

int dev_init(int idx)
{
	struct grspw_device *dev = &devs[idx];
	int i, ctrl, clkdiv, tc, stscfg;
	unsigned int icctrl;

	printf(" Initializing SpaceWire device %d\n", idx);

	memset(dev, 0, sizeof(struct grspw_device));

	dev->index = idx;
	dev->dh = grspw_open(idx);
	if (dev->dh == NULL) {
		printf("Failed to open GRSPW device %d\n", idx);
		return -1;
	}
	grspw_hw_support(dev->dh, &dev->hwsup);
#ifdef PRINT_GRSPW_RESET_CFG
	grspw_config_read(DEV(dev), &dev->cfg);
	printf("\n\n---- DEFAULT CONFIGURATION FROM DRIVER/HARDWARE ----\n");
	grspw_cfg_print(&dev->hwsup, &dev->cfg);
#endif
	dev->cfg = dev_def_cfg;
	dev->cfg.adrcfg.def_addr = 32 + idx;
	dev->cfg.tc_isr_arg = dev;
	tc = TCOPTS_EN_TX | TCOPTS_EN_RX | TCOPTS_EN_RXIRQ;
	grspw_tc_ctrl(dev->dh, &tc);

	if (grspw_cfg_set(DEV(dev), &dev->cfg)) {
		grspw_close(dev->dh);
		return -1;
	}
#ifdef PRINT_GRSPW_RESET_CFG
	printf("\n\n---- APPLICATION CONFIGURATION ----\n");
	grspw_cfg_print(&dev->hwsup, &dev->cfg);
	printf("\n\n");
#endif

	/* Make sure the required SpaceWire Interrupt support is available */
	if (dev->hwsup.irq == 0) {
		printf(" dev(%d) does not have SPW-IRQ support!\n", dev->index);
		return -1;
	}

	/* Enable CTRL.TF to make sure that GRSPW separate Time Codes from
	 * SpaceWire interrupt-codes.*/

	icctrl = -1;
	//grspw_ic_ctrl(DEV(dev), &icctrl);
	icctrl |= ICOPTS_EN_FLAGFILTER;
	//grspw_ic_ctrl(DEV(dev), &icctrl);

	/* This will result in an error if only one port available */
	if (dev->hwsup.nports < 2) {
		int port = 1;
		if ( grspw_port_ctrl(dev->dh, &port) == 0 ) {
			printf("Succeeded to select port1, however only one PORT on dev %d!\n", dev->index);
			return -1;
		}
	}

	/* Try to bring link up at fastest clockdiv but do not touch
	 * start-up clockdivisor */
	clkdiv = -1;
	grspw_link_ctrl(dev->dh, NULL, NULL, &clkdiv);
	ctrl = LINKOPTS_ENABLE | LINKOPTS_AUTOSTART | LINKOPTS_START;
	clkdiv &= 0xff00;
	clkdiv |= 0x13;
	stscfg = LINKSTS_MASK;
	grspw_link_ctrl(dev->dh, &ctrl, &stscfg, &clkdiv);

	if ( (dev->hwsup.hw_version >> 16) == GAISLER_SPW2_DMA )
		printf(" NOTE: running on SPW-ROUTER DMA SpaceWire link (no link-state available)\n");
	else
		printf(" After Link Start: %d\n", (int)grspw_link_state(dev->dh));
	dev->run = 0;

	grspw_stats_clr(dev->dh);

	for (i=0; i<dev->hwsup.ndma_chans; i++) {
		if (dev->dma[i])
			grspw_dma_stats_clr(dev->dma[i]);
	}

	grspw_list_clr(&dev->rx_list);
	grspw_list_clr(&dev->rx_buf_list);
	grspw_list_clr(&dev->tx_list);
	grspw_list_clr(&dev->tx_buf_list);
	dev->rx_list_cnt = dev->rx_buf_list_cnt = 0;
	dev->tx_list_cnt = dev->tx_buf_list_cnt = 0;

	return 0;
}

int dev_check_started(int idx)
{
	struct grspw_device *dev = &devs[idx];
	if ( (dev->hwsup.hw_version >> 16) == GAISLER_SPW2_DMA ){
		printf("NOTE: running on SPW-ROUTER DMA SpaceWire link (no link-state available)\n");
		return 1;
	}else{
		if (grspw_link_state(dev->dh) != SPW_LS_RUN){
			return 0;
		}
	}
	return 1;
}

void dev_enable_spwirq(int idx)
{
	struct grspw_device *dev = &devs[idx];
	unsigned int icctrl;
	icctrl = -1;
	grspw_ic_ctrl(dev->dh, &icctrl);
	icctrl |= ICOPTS_EN_RX | ICOPTS_EN_TX;
	grspw_ic_ctrl(dev->dh, &icctrl);
}

int dev_dma_close_all(int idx)
{
	struct grspw_device *dev = &devs[idx];
	int i, rc;
	for (i=0; i<dev->hwsup.ndma_chans; i++) {
		if (dev->dma[i]) {
			rc = grspw_dma_close(dev->dma[i]);
			if (rc)
				return rc;
			dev->dma[i] = NULL;
		}
	}
	return 0;
}

void dev_cleanup(int idx)
{
	struct grspw_device *dev = &devs[idx];

	if (dev->dh == NULL)
		return;

	/* Stop all DMA activity first */
	grspw_stop(DEV(dev));
	/* wait for other tasks to be thrown out from driver */
	rtems_task_wake_after(4);

	/* close all DMA channels */
	if (dev_dma_close_all(idx)) {
		printf("FAILED to close GRSPW%d DMA\n", idx);
	}

	if (grspw_close(dev->dh)) {
		printf("FAILED to close GRSPW%d\n", idx);
	}
	dev->dh = NULL;
}

void pktlist_set_flags(struct grspw_list *lst, unsigned short flags)
{
	struct grspw_pkt *pkt = lst->head;
	while (pkt) {
		pkt->flags = flags;
		pkt = pkt->next;
	}
}

int dma_process(struct grspw_device *dev);

struct grspw_device *rmap_dev;
void *rmap_stack;

#include <grlib/spwtdp.h>
struct spwtdp_regs spwtdp;
int remote_fine, remote_coarse;

int remote_spwtdp_init(void);
int remote_spwtdp_reset(void);
int remote_spwtdp_read(unsigned int ofs, int len);
int remote_spwtdp_write(unsigned int ofs, int len);


int parse_precision(unsigned short preamble, int *coarse, int *fine)
{
	int coarse_precision, fine_precision;

	if (preamble & 0x80) {
		puts("Pfield second extension set: unknown format");
		return -1;
	}
	if (!((preamble & 0x7000) == 0x2000 || (preamble & 0x7000) == 0x1000)) {
		puts(" PField indicates not unsegmented code: unknown format");
		return -1;
	}
	/*
	   coarse_precision = 32;
	   fine_precision = 24;
	   */
	coarse_precision = ((preamble >> 10) & 0x3) + 1;
	if (preamble & 0x80)
		coarse_precision += (preamble >> 5) & 0x3;
	fine_precision = (preamble >> 8) & 0x3;
	if (preamble & 0x80)
		fine_precision += (preamble >> 2) & 0x7;
	if (coarse)
		*coarse = coarse_precision;
	if (fine)
		*fine = fine_precision;
	return 0;
}

int remote_spwtdp_init(void)
{
	unsigned short preamble;

	memset(&spwtdp, 0, sizeof(spwtdp));

	if (remote_spwtdp_reset())
		return -1;
	/* if (remote_spwtdp_read(0, sizeof(spwtdp))) */
	if (remote_spwtdp_read(0, 0x0cc)){ /* SPWTDP core in connected GR-RASTA system does not have the
										  external datation part of the spwtdp_regs, so we can only read till 0x0cc instead of 0x190=sizeof(spwtdp)*/
		return -2;
	}

	preamble = spwtdp.dat_ctrl & 0xffff;
	if (parse_precision(preamble, &remote_coarse, &remote_fine)) {
		puts("failed parsing PField of remote SPWTDP");
		return -3;
	}
	printf("Target Coarse precision: %d bits\n", remote_coarse*8);
	printf("Target Fine precision:   %d bits\n", remote_fine*8);

	/* Initialize basic initialization */
	spwtdp.conf[0] =
		(1 << 24) |                 /* Jitter Correction */
		(R_SPWTDP_MAPPING << 8) |   /* Mapping */
		(1 << 7) |                  /* TD=1 */
		(R_SPWTDP_SPWDEV << 4) |    /* SpW device */
		(1 << 3) |                  /* Mitigation Enable */
		(1 << 2) |                  /* Enable RX for Target */
		(0 << 0);                   /* Out of Reset */

	/* CFG1 and CFG2 */
#if defined(GR740_SLAVE)
	spwtdp.conf[1] = 72057594;
#elif defined(FPGA_SLAVE)
	spwtdp.conf[1] = 400319966;
#else
#error "Wrong slave"
#endif
	spwtdp.conf[2] = 0;

	if (remote_spwtdp_write(0, 3*sizeof(spwtdp.conf[0])))
		return -4;

	return 0;
}

/* Reset core and get out of reset */
int remote_spwtdp_reset()
{
	spwtdp.conf[0] = 1; /* Reset */
	if (remote_spwtdp_write(0, sizeof(spwtdp.conf[0])))
		return -1;

	spwtdp.conf[0] = 0; /* Stop Reset */
	if (remote_spwtdp_write(0, sizeof(spwtdp.conf[0])))
		return -2;

	return 0;
}

struct grspw_pkt *dma_alloc_tx_buf(struct grspw_device *dev)
{
	struct grspw_pkt *pkt;

	/* Allocate SpW packet buffer to hold RMAP request */
	rtems_semaphore_obtain(dma_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
	pkt = dev->tx_buf_list.head;
	if (pkt == NULL) /* no free packets left */
		goto out;
	dev->tx_buf_list.head = pkt->next;
	if (pkt->next == NULL)
		dev->tx_buf_list.tail = NULL;
	dev->tx_buf_list_cnt--;

out:
	rtems_semaphore_release(dma_sem);
	return pkt;
}

int dma_sched_tx_pkt(struct grspw_device *dev, struct grspw_pkt *pkt)
{
	/* Put TX packet into transmission queue */
	rtems_semaphore_obtain(dma_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
	grspw_list_append(&dev->tx_list, pkt);
	dev->tx_list_cnt++;
	rtems_semaphore_release(dma_sem);

	return 0;
}

/* Function that will generate path addressing.
 *
 * \param cookie   Identifies RMAP stack, NOT USED.
 * \param dir      Direction (0=to dst, 1=from dst)
 * \param srcadr   Source address (the SpW address of the interface used)
 * \param dstadr   Destination address to translate. Note that it is actually
 *                 a "unsigned short" given here, so this is rather a Node-ID.
 * \param buf      Where path address is put
 * \param len      On calling it indicates max length. On return it must be set
 *                 reflect the number of bytes was written to buf.
 */
int rmap_route_func1(void *cookie, int dir, int srcadr, int dstadr, void *buf, int *len)
{
	unsigned char *data;
	unsigned char *route;
	int tot, nr, i;
	unsigned char adr;
	struct route_entry *r;

	data = buf;
	tot = 0;

	/* printf("ROUTE FUNC: cookie: 0x%08x, dir=%d, srcadr=0x%02x, dstadr=0x%02x, buf: 0x%08x, len=%d\n",(unsigned int) cookie, dir, srcadr, dstadr, (unsigned int) buf, *len); */

	if ( dir == 0 ) {
		/* Route from this Node to Destination */
		adr = dstadr;
		r = &routetab[ROUTE_TO];
	} else {
		/* Route from Destination to this node */
		adr = srcadr;
		r = &routetab[ROUTE_FROM];

		nr = r->nr;
		/* Pad with zeros up to a 4byte boundary  */
		while ( nr & 0x3 ) {
			*data++ = 0;
			tot++;
			nr++;
		}
	}

	/* Copy path address until we reach adress zero */
	route = &r->dstadr[0];
	for (i=0; i<r->nr; i++) {
		*data++ = *route++;
	}
	tot += r->nr;

	/* Add SRC/DST address */
	*data = adr & 0xff;
	*len = tot + 1;

	return 0;
}

/* GRMON triggering point, set breakpoint here */
int grmon_clk_bp1(int v0, int v1)
{
	return v0+v1; /* just do something */
}

/* Sync local cache copy of remote registers by reading from target */
int rmap_tgt_read(unsigned int address, void *data, int len, int dstadr)
{
	struct rmap_command_read cmd;
	struct rmap_command *pcmd;
	struct grspw_pkt *txpkt, *rxpkt, *prevpkt;
	struct rmap_spw_pkt rmappkt;
	int i, j, found, rc;
	static int cnt = 0;

	cnt++;

	/* Allocate SpW packet buffer to hold RMAP request */
	txpkt = dma_alloc_tx_buf(rmap_dev);
	if (txpkt == NULL) {
		printf(" No free transmit buffers available %d\n", cnt);
		rtems_semaphore_release(dma_sem);
		return -2;
	}

	/* Describe RMAP request command */
	cmd.type = RMAP_CMD_RI;
	cmd.dstadr = dstadr;
	cmd.dstkey = R_SPWTDP_DSTKEY;
	cmd.address = address;
	cmd.length = len;
	cmd.datalength = 0;
	cmd.data = data; /* let RMAP stack copy data */

	/* Let RMAP stack generate RMAP SpaceWire packet into TX buffer */
	rmappkt.options = 0;
	rmappkt.hdr = txpkt->hdr;
	rmappkt.data = txpkt->data;
	rc = rmap_send_async(rmap_stack, (struct rmap_command *)&cmd, &rmappkt);
	if (rc < 0) {
		printf("rmap_send_async failed %d\n", rc);
		return -3;
	}
	if (rc == 0) {
		/* No wait is expected to return directly */
		puts("rmap_send_async indicates no response");
		return -4;
	}
	/* update TXpkt with RMAP stack output */
	txpkt->dlen = rmappkt.dlen;
	txpkt->hlen = rmappkt.hlen;
	txpkt->flags = 0;
	if (rmappkt.options & PKT_OPTION_HDR_CRC)
		txpkt->flags |= TXPKT_FLAG_HCRC;
	if (rmappkt.options & PKT_OPTION_DATA_CRC)
		txpkt->flags |= TXPKT_FLAG_DCRC;
	/* Put NONCRC part into flags. TODO: Improve this to use actual length of the path addressing (not hard-coded). */
	if (path_addressing)
		txpkt->flags |= NOCRC_FLAG;

	/* Put TX packet into transmission queue */
	if (dma_sched_tx_pkt(rmap_dev, txpkt)) {
		printf("write(): Failed scheduling TX packet\n");
		return -5;
	}

	/* Wait for response */
	found = 0;
	for (i = 0; i<20 && !found; i++) {
		/* sleep one tick */
		rtems_task_wake_after(1);

		rtems_semaphore_obtain(dma_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
		rxpkt = rmap_dev->rx_list.head;
		prevpkt = NULL;
		found = 0;
		for (j=0; j<rmap_dev->rx_list_cnt; j++) {
			/* search for our specific RMAP response
			 * Tell the RMAP stack about the Data and Header
			 * CRC results.
			 */
			rmappkt.options = 0;
			if ((rxpkt->flags & RXPKT_FLAG_DCRC) == 0)
				rmappkt.options |= PKT_OPTION_DATA_CRC;
			if ((rxpkt->flags & RXPKT_FLAG_HCRC) == 0)
				rmappkt.options |= PKT_OPTION_HDR_CRC;
			rmappkt.hdr = rxpkt->hdr;
			rmappkt.data = rxpkt->data;
			pcmd = (struct rmap_command *)&cmd;
			if (rmap_recv_async(rmap_stack, &rmappkt, &pcmd) == 0) {
				found = 1;
				break;
			}
			prevpkt = rxpkt;
			rxpkt = rxpkt->next;
		}
		if (found) {
			/* We found our packet.
			 *
			 * 1. Remove it from rx list
			 * 2. put it back into rx_buf list
			 */
			if (prevpkt == NULL)
				rmap_dev->rx_list.head = rxpkt->next;
			else
				prevpkt->next = rxpkt->next;
			if (rxpkt->next == NULL)
				rmap_dev->rx_list.tail = prevpkt;
			rmap_dev->rx_list_cnt--;

			grspw_list_append(&rmap_dev->rx_buf_list, rxpkt);
			rmap_dev->rx_buf_list_cnt++;

			/* Process the RMAP command */

			if (cmd.datalength != len)
				printf("Request and response length differs\n");
		}

		rtems_semaphore_release(dma_sem);
	}

	if (!found) {
		/* Remove command from RMAP async wait state */
		rmap_cancel_async(rmap_stack, (struct rmap_command *)&cmd);

		printf("Timeout on RMAP response\n");
		/* handle error here ... */

		return -6;
	}

	if (cmd.status != 0)
		return -7;

	/* The callers data buffer has now been updated */

	return 0;
}

int rmap_tgt_write(unsigned int address, void *data, int len, int dstadr)
{
	struct rmap_command_write cmd;
	struct rmap_command *pcmd;
	struct grspw_pkt *txpkt, *rxpkt, *prevpkt;
	struct rmap_spw_pkt rmappkt;
	int i, j, found, rc;
	static int cnt = 0;

	cnt++;

	/* Allocate SpW packet buffer to hold RMAP request */
	txpkt = dma_alloc_tx_buf(rmap_dev);
	if (txpkt == NULL) {
		printf("write(): No free transmit buffers available %d\n", cnt);
		rtems_semaphore_release(dma_sem);
		return -2;
	}

	/* Describe RMAP request command */
	cmd.type = RMAP_CMD_WIA;
	cmd.dstadr = dstadr;
	cmd.dstkey = R_SPWTDP_DSTKEY;
	cmd.address = address;
	cmd.length = len;
	cmd.data = txpkt->data; /* Data to be analysed (not copied) */

	/* Let RMAP stack generate RMAP SpaceWire packet into TX buffer */
	rmappkt.options = 0;
	rmappkt.hdr = txpkt->hdr;
	/* Copy data to SpW buffer (we could implement zero-copy here...) */
	memcpy(txpkt->data, data, len);

	rc = rmap_send_async(rmap_stack, (struct rmap_command *)&cmd, &rmappkt);
	if (rc < 0) {
		printf("write(): rmap_send_async failed %d\n", rc);
		return -3;
	}
	if (rc == 0) {
		/* No wait is expected to return directly */
		puts("write(): rmap_send_async indicates no response");
		return -4;
	}
	/* update TXpkt with RMAP stack output */
	txpkt->dlen = rmappkt.dlen;
	txpkt->hlen = rmappkt.hlen;
	txpkt->flags = 0;
	if (rmappkt.options & PKT_OPTION_HDR_CRC)
		txpkt->flags |= TXPKT_FLAG_HCRC;
	if (rmappkt.options & PKT_OPTION_DATA_CRC)
		txpkt->flags |= TXPKT_FLAG_DCRC;
	/* Put NONCRC part into flags */
	if (path_addressing)
		txpkt->flags |= NOCRC_FLAG;

	/* Put TX packet into transmission queue */
	if (dma_sched_tx_pkt(rmap_dev, txpkt)) {
		printf("write(): Failed scheduling TX packet\n");
		return -5;
	}

	/* Wait for response */
	found = 0;
	for (i = 0; i<20 && !found; i++) {
		/* sleep one tick */
		rtems_task_wake_after(1);

		rtems_semaphore_obtain(dma_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
		rxpkt = rmap_dev->rx_list.head;
		prevpkt = NULL;
		found = 0;
		for (j=0; j<rmap_dev->rx_list_cnt; j++) {
			/* search for our specific RMAP response
			 * Tell the RMAP stack about the Data and Header
			 * CRC results.
			 */
			rmappkt.options = 0;
			if ((rxpkt->flags & RXPKT_FLAG_DCRC) == 0)
				rmappkt.options |= PKT_OPTION_DATA_CRC;
			if ((rxpkt->flags & RXPKT_FLAG_HCRC) == 0)
				rmappkt.options |= PKT_OPTION_HDR_CRC;
			rmappkt.hdr = rxpkt->hdr;
			rmappkt.data = rxpkt->data;
			pcmd = (struct rmap_command *)&cmd;
			if (rmap_recv_async(rmap_stack, &rmappkt, &pcmd) == 0) {
				found = 1;
				break;
			}
			prevpkt = rxpkt;
			rxpkt = rxpkt->next;
		}
		if (found) {
			/* We found our packet.
			 *
			 * 1. Remove it from rx list
			 * 2. put it back into rx_buf list
			 */
			if (prevpkt == NULL)
				rmap_dev->rx_list.head = rxpkt->next;
			else
				prevpkt->next = rxpkt->next;
			if (rxpkt->next == NULL)
				rmap_dev->rx_list.tail = prevpkt;
			rmap_dev->rx_list_cnt--;

			grspw_list_append(&rmap_dev->rx_buf_list, rxpkt);
			rmap_dev->rx_buf_list_cnt++;
		}

		rtems_semaphore_release(dma_sem);
	}

	if (!found) {
		/* Remove command from RMAP async wait state */
		rmap_cancel_async(rmap_stack, (struct rmap_command *)&cmd);

		printf("write(): Timeout on RMAP response\n");
		/* handle error here ... */

		return -5;
	}

	/* Process the RMAP command */
	if (cmd.status != 0) {
		printf("write(): RMAP write failed with %d\n", cmd.status);
		return -7;
	}

	/* The callers data buffer has now been updated */


	return 0;
}

/* Sync local cache copy of remote registers by reading from target */
int remote_spwtdp_read(unsigned int ofs, int len)
{
	int status;

	if ((ofs+len) > sizeof(spwtdp))
		return -1;

	/* Sync Read */
	status = rmap_tgt_read((unsigned int)R_SPWTDP_BASEADR + ofs,
			((void *)&spwtdp) + ofs, len, R_SPWTDP_DSTADR);
	if (status != 0) {
		printf("Failed executing RMAP read from SPWTDP Target: %d\n",
				status);
	}
	return status;
}

/* Sync local cache copy of remote registers by writing to target */
int remote_spwtdp_write(unsigned int ofs, int len)
{
	int status;

	if ((ofs+len) > sizeof(spwtdp))
		return -1;

	status = rmap_tgt_write((unsigned int)R_SPWTDP_BASEADR + ofs,
			((void *)&spwtdp) + ofs,
			len,
			R_SPWTDP_DSTADR);
	if (status != 0) {
		printf("Failed executing RMAP write to SPWTDP Target: %d\n",
				status);
	}
	return status;
}

void *tdp;
volatile unsigned int gists=0;

void tdp_isr(unsigned int ists, void *data)
{
	//printk("TDP_ISR: 0x%02x\n", ists);
	gists |= ists;
}

#ifdef DEBUG_ET_SAMPLING_TIMES
unsigned int tmr_log[1024];
int tmr_cnt;

void log_tmr()
{
	if (tmr_cnt == 0) {
		*(volatile unsigned int *)TIMER_BASE = 0xffffffff;
		*(volatile unsigned int *)(TIMER_BASE + 0x4) = 0xffffffff;
		*(volatile unsigned int *)(TIMER_BASE + 0x8) = 0x55;
	}
	tmr_log[tmr_cnt] = 0xffffffff - (*(volatile unsigned int *)TIMER_BASE);
	tmr_cnt++;
	if (tmr_cnt >= 1024)
		tmr_cnt = 0;
}
#endif

/* Read the remote ET and create a spwtdp_time_t value
*/
int remote_read_et_all(unsigned int *ctrl, spwtdp_time_t * val)
{
	unsigned int sample[6];
	int i;

	if (val==NULL){
		return -1;
	}

	for (i = 0; i < 6; i++) {
		sample[i] = ctrl[i];
	}

	val->preamble=sample[0]&0xffff;
	unsigned int *buffer = (unsigned int *) val->data;
	for (i = 0; i < 5; i++) {
		buffer[i]=sample[i+1];
	}

	return 0;
}

/* For Latency calculation. Moved to a global context so that they can be
 * inpected from GRMON mem commands.
 */
spwtdp_time_t i_tsrx, i_tstx, t_tsrx, t_tstx;
spwtdp_time_t i_diff, t_diff;
spwtdp_time_t total, min, max;
spwtdp_time_t result_diff[LATENCY_COUNT];
unsigned long long total_coarse, total_fine, latency_fine;

rtems_task test_app(rtems_task_argument ignored)
{
	int i, ret;
	struct rmap_config rmap_cfg;
	int status;

	unsigned short preamble;
	uint8_t coarse, fine;
	rtems_interrupt_level level;
	unsigned int tmp;

	/* Initialize two GRSPW AMBA ports */
	nospw = grspw_dev_count();
	if (nospw < 1) {
		printf("Found no SpaceWire cores, aborting\n");
		exit(0);
	}
	if (nospw > DEVS_MAX) {
		printf("Limiting to %d SpaceWire devices\n", DEVS_MAX);
		nospw = DEVS_MAX;
	}

	memset(devs, 0, sizeof(devs));
	for (i=0; i<nospw; i++) {
		if (dev_init(i)) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
		fflush(NULL);
	}

	/* Initialize GRSPW packets */
	init_pkts();

	printf("\n\nStarting SpW DMA channels\n");
	for (i = 0; i < nospw; i++) {
		printf("Starting GRSPW%d: ", i);
		fflush(NULL);
		if (grspw_start(DEV(&devs[i]))) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
		printf("DMA Started Successfully\n");
	}

	/* Make sure the SpaceWire link is in run-state before proceeding */
	if (dev_check_started(DEV_IDX) == 0) {
		printf("Link to SPWTDP target is not in run-state\n");
		exit(0);
	}



	/* Get on-chip SPWTDP */
	int dev_no=0;
	tdp = spwtdp_open(dev_no);
	if (!tdp) {
		puts("No on-chip SPWTDP device present. Aborting");
		exit(0);
	}
	printf("using SPWTDP%d at 0x%08x\n", dev_no, (unsigned int)tdp);
	ret=spwtdp_reset(tdp);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when reseting SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	printf("Local SPWTDP: reset.\n");
	//TODOspwtdp_clr_stats(tdp);
	/* Install ISR but do not enable IRQ until later */
	ret=spwtdp_isr_register(tdp, tdp_isr, NULL);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when registering ISR on SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	printf("Local SPWTDP: ISR registered.\n");

	/* Start SpW DMA and SpW link-status tasks and wait */
	fflush(NULL);
	rtems_task_start(tid_link, link_ctrl_task, 0);
	rtems_task_start(tid_dma, dma_task, 0);
	rtems_task_wake_after(12);

	/* Set up asynchronos RMAP stack */
	rmap_dev = &devs[RMAP_SPW_DEV_IDX];
	if (path_addressing){
		rmap_cfg.route_func = rmap_route_func1; /* Function that defines the routing */
	}else{
		rmap_cfg.route_func = NULL;
	}
	rmap_cfg.spw_adr = rmap_dev->cfg.adrcfg.def_addr;
	rmap_cfg.tid_msb = 0xff; /* only communication with one SpW RMAP target */
	if (rmap_dev->hwsup.rmap_crc)
		rmap_cfg.drv_cap = DRV_CAP_HDR_CRC | DRV_CAP_DATA_CRC;
	else
		rmap_cfg.drv_cap = 0;
	rmap_cfg.thread_safe = 0; /* only one sender */
	rmap_cfg.drv = NULL; /* not needed in async mode */
	rmap_cfg.max_rx_len = rmap_dev->cfg.chan[0].rxmaxlen - 32; /* adjust for RMAP header */
	rmap_cfg.max_tx_len = rmap_dev->cfg.chan[0].rxmaxlen;
	rmap_stack = rmap_async_init(&rmap_cfg, 16);
	if (!rmap_stack) {
		printf("RMAP stack initialization failed\n");
		exit(0);
	}

	/*** Init SpW-TDPs ***/

	/* CFG0 register setup:
	 *  - Core has already been resetted
	 *  - Select TDP over CUC
	 *  - Select SpW0 for SpaceWire transmissions (IRQ and TC)
	 *  - Mapping generate 64 TimeCodes per Second (2^6)
	 */
	ret = spwtdp_initiator_conf(tdp,6,SPWTDP_TDP_ENABLE);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when configuring initiator SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	printf("Local SPWTDP: Initiator configured.\n");


	/* Leave CFG1 and CFG2 to reset values */
	/* GR740 CFG1 and CFG2 */
#if defined(GR740_MASTER)
	ret = spwtdp_freq_setup(tdp,72057594,0,0);
#elif defined(FPGA_MASTER)
	ret = spwtdp_freq_setup(tdp,400319966,0,0);
#else
#error "Wrong master"
#endif
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when configuring frequency SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	printf("Local SPWTDP: Frequency configured.\n");

	/* CFG3 register setup:
	 *  - INRX=4, INTX=5 - this is to match Target
	 *  - Disable Acknowledge on Interrupts
	 *  - STM Mask to do timeStamping at every 4th TC, i.e. 16 times/second
	 */
	ret = spwtdp_initiator_int_conf(tdp, 0x3, 0x4, 0x5);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when configuring interrupt SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	printf("Local SPWTDP: SPW Interrupt configured.\n");

	/* CFG0 register setup:
	 *  - Enable Trasmitter
	 */
	ret = spwtdp_initiator_enable(tdp);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when enabling initiator SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	printf("Local SPWTDP: Initiator enabled.\n");

	ret = spwtdp_precision_get(tdp,&fine,&coarse);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when getting precission SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	printf("Coarse precision: %d bits\n", coarse*8);
	printf("Fine precision:   %d bits\n", fine*8);

	/* Command Control register:
	 *  - NC/IS initialization (will set datation when ET written)
	 *  - init Preamble as CCSDS 301.0-B-4:
	 *    * 32bit coarse time, 64-bit fractional time
	 *    * 1958 Jan 1 - epoch
	 *
	 * examples:
	 *   - 32+32 bits: 0x9e08 == 0x9c10
	 *   - 32+48 bits: 0x9e10 == 0x9c18
	 *   - 32+64 bits: 0x9d1c == 0x9f14
	 */
	//TODO preamble tdp_regs->cmd_ctrl = (1<<31) | preamble;
	spwtdp_time_t cmd_et;

	/* Sync datation with RTEMS System Timer overflow. This approach
	 * is dangerous but is an example what basically could be needed. We
	 * wait for the next tick and then we write the ET so the datation is
	 * initialized.
	 * Instead of writing all zeros to the datation unit we should get
	 * the time from RTEMS and convert that into the correct time format...
	 * but this is just an example.
	 */
	rtems_interrupt_disable(level);
	/* init as much as possible of ET */
	for (i=0;i<SPWTDP_TIME_DATA_LENGTH;i++){
		cmd_et.data[i]=0;
	}
	cmd_et.preamble=0x2f00;

	/* Wait for last 1us before tick */
	while (LEON3_Timer_Regs->timer[0].value > 0) {
		asm volatile ("nop\n\t"::); /* wait 8 clocks */
		asm volatile ("nop\n\t"::);
		asm volatile ("nop\n\t"::);
		asm volatile ("nop\n\t"::);
		asm volatile ("nop\n\t"::);
		asm volatile ("nop\n\t"::);
		asm volatile ("nop\n\t"::);
		asm volatile ("nop\n\t"::);
	}
	asm volatile ("nop\n\t"::); /* add an additional small delay */
	asm volatile ("nop\n\t"::);
	asm volatile ("nop\n\t"::);
	asm volatile ("nop\n\t"::);
	/* this should write ET to datation */
	ret=spwtdp_initiator_cmd_et_set(tdp, cmd_et);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when writing CMDET SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}

	/* use GRMON here to set a breakpoint on grmon_clk_bp() to see the
	 * initialization accuracy... used to debug.
	 */
	grmon_clk_bp1(0, LEON3_Timer_Regs->timer[0].value);

	rtems_interrupt_enable(level); /* let the tick be handled */

	/* At this point the Initiator sends 64 SpW timecodes per second.
	 * the targets will be initialized within one second can wait for
	 * the next SpW Time Code of 0 (fine time = 0)
	 */

	/*** Target Initialization ***/

	/* Enable TimeCode and Interrupt on remote SpW device end */
	tmp = 0;
	if (rmap_tgt_read(R_SPW_BASEADR, &tmp, 4, R_SPWTDP_DSTADR)) {
		printf("Failed reading Target SpW Ctrl register\n");
		exit(0);
	}
	tmp |= 0x01802;
	if (rmap_tgt_write(R_SPW_BASEADR, &tmp, 4, R_SPWTDP_DSTADR)) {
		printf("Failed writing Target SpW Ctrl register\n");
		exit(0);
	}
	/* clr status register */
	tmp = 0xffffffff;
	if (rmap_tgt_write(R_SPW_BASEADR+0x04, &tmp, 4, R_SPWTDP_DSTADR)) {
		printf("Failed clearing Target SpW Status register\n");
		exit(0);
	}

	/* Init SPWTDP */

	status = remote_spwtdp_init();
	if (status) {
		printf("Remote initialization failed: %d\n", status);
		exit(0);
	}

	/* Check if Initiator and Target have same hardware setup */
	if (remote_coarse != coarse || remote_fine != fine) {
		puts("This example is made for equal precision of counters");
		exit(0);
	}

	/* CFG3 register setup:
	 *  - INRX=5, INTX=4 - this is to match Initiator
	 *  - Disable Acknowledge on Interrupts
	 *  - STM Mask to do timeStamping at every 4th TC, i.e. 16 times/second
	 */
	spwtdp.conf[3] = (0x03<<16) | (0<<10) | (5<<5) | (4<<0);

	/* Set up next time for coarse time increment */
	preamble = 0x2f00;
	spwtdp.cmd_ctrl = (1<<31) | (1<<30) | (0<<16) | preamble;
	spwtdp.cmd_et[0] = 0;
	spwtdp.cmd_et[1] = 0;
	spwtdp.cmd_et[2] = 0;
	spwtdp.cmd_et[3] = 0;
	spwtdp.cmd_et[4] = 0;
	/* next coarse time in the relevant octet depending on the length of
	 * the coarse time
	 */
	((unsigned char *)spwtdp.cmd_et)[coarse - 1] = 1;

	/* Sync (changed parts of) local cached copy to Target register using
	 * RMAP
	 */
	remote_spwtdp_write(0x0c, 0x38 - 0xc);

	/* Resync to see that written registers have an effect */
	/* remote_spwtdp_read(0, sizeof(spwtdp)); */
	remote_spwtdp_read(0, 0x0cc); /* SPWTDP core in connected GR-RASTA system does not have the
									 external datation part of the spwtdp_regs, so we can only read till 0x0cc instead of 0x190=sizeof(spwtdp)*/
	if (((spwtdp.cmd_ctrl >> 31) & 0x1) != 0x1) {
		printf("Target ET init failed1: 0x%08x\n", spwtdp.cmd_ctrl);
		exit(0);
	}

	/* Wait for next tick for coarse time, then check that target has been
	 * initialized
	 */
	spwtdp_time_t dat_et;
	ret=spwtdp_dat_et_get(tdp, &dat_et);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when getting DATET SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
#ifdef DEBUG_ET_SAMPLING_TIMES
	log_tmr();
#endif
	while (dat_et.data[coarse-1] == 0) {
		rtems_task_wake_after(1);
		ret=spwtdp_dat_et_get(tdp, &dat_et);
		if (ret!=SPWTDP_ERR_OK) {
			printf("Failure when getting DATET SPWTDP(ret=%d). Aborting", ret);
			exit(0);
		}
#ifdef DEBUG_ET_SAMPLING_TIMES
		log_tmr();
#endif
	}

	/* Resync to see that Target ET has been initialized */
	/* remote_spwtdp_read(0, sizeof(spwtdp)); */
	remote_spwtdp_read(0, 0x0cc); /* SPWTDP core in connected GR-RASTA system does not have the
									 external datation part of the spwtdp_regs, so we can only read till 0x0cc instead of 0x190=sizeof(spwtdp)*/

	if (((spwtdp.cmd_ctrl >> 31) & 0x1) != 0x0) {
		printf("Target ET init failed2: 0x%08x\n", spwtdp.cmd_ctrl);
		exit(0);
	}
	if (((unsigned char *)spwtdp.dat_et)[coarse - 1] != 1) {
		printf("Target Datation init failed: 0x%08x 0x%08x 0x%08x 0x%08x 0x%04x 0x%08x\n",
				spwtdp.cmd_ctrl, spwtdp.dat_et[0],
				spwtdp.dat_et[1], spwtdp.dat_et[2],
				spwtdp.dat_et[3], spwtdp.dat_et[4]);
		exit(0);
	}

	puts("successful init");
	/* TODO: Code to disable router timers */
	volatile unsigned int * tpresc = (volatile unsigned int *) 0xff880a14;
	*tpresc = 0xfa;
	volatile unsigned int * isrtrld = (volatile unsigned int *) 0xff880a30;
	*isrtrld = 0x0;
	volatile unsigned int * rtrcfg = (volatile unsigned int *) 0xff880a00;
	unsigned int rtrcfg_val = 0x00000702;
	volatile unsigned int * pctrl = (volatile unsigned int *) 0xff880804;
#ifdef GR740_MASTER
	*pctrl = 0x2700802e;
#elif defined(FPGA_MASTER)
	*pctrl = 0x1300802e;
#else
#error "Wrong master"
#endif
#ifdef GR740_SLAVE
	unsigned int pctrl_val = 0x2700802e;
#elif defined(FPGA_SLAVE)
	unsigned int pctrl_val = 0x1300802e;
#else
#error "Wrong slave"
#endif
	volatile unsigned int * pctrl2 = (volatile unsigned int *) 0xff880984;
	unsigned int pctrl2_val = 0xc000de00;
	/* Now in the remote */
	if (rmap_tgt_write((unsigned int) tpresc, (void *) tpresc, 4, R_SPWTDP_DSTADR)) {
		printf("Failed writing Target Router tpresc register\n");
		exit(0);
	}
	if (rmap_tgt_write((unsigned int) isrtrld, (void *) isrtrld, 4, R_SPWTDP_DSTADR)) {
		printf("Failed writing Target Router isrtrld register\n");
		exit(0);
	}
	if (rmap_tgt_write((unsigned int) rtrcfg, &rtrcfg_val, 4, R_SPWTDP_DSTADR)) {
		printf("Failed writing Target Router CFGSTS register\n");
		exit(0);
	}
	if (rmap_tgt_write((unsigned int) pctrl, &pctrl_val, 4, R_SPWTDP_DSTADR)) {
		printf("Failed writing Target Router PCTRL port 1 register\n");
		exit(0);
	}
	if (rmap_tgt_write((unsigned int) pctrl2, &pctrl2_val, 4, R_SPWTDP_DSTADR)) {
		printf("Failed writing Target Router PCTRL2 port 1 register\n");
		exit(0);
	}
	asm volatile("grmon_test_label:");


	puts("Disabled router timers");
	printf("Start latency calculation for %d samples\n", LATENCY_COUNT);

	/*** Make latency calculations ***/

	/* Enable RX/TX Distributed Interrupts locally */
	dev_enable_spwirq(DEV_IDX);

	/* Enable distributed interrupts on SpW device used by SpWTDP */
	tmp = (0x3 << 16);
	if (rmap_tgt_write(R_SPW_BASEADR+0xa0, &tmp, 4, R_SPWTDP_DSTADR)) {
		printf("Failed writing Target SpW 0xa0 register\n");
		exit(0);
	}
	/* Set up every TimeCode of value=0bxxxx10 a Interrupts is generated */
	//TODO check with anand tdp_regs->ts_tx_ctrl = (2<<16) | preamble;
	ret=spwtdp_initiator_tstx_conf(tdp, 2);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when configuring TSTX SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}

	/* Enable Latency Calculation of the Target before initiator */
	spwtdp.conf[0] |= 1<<16;
	remote_spwtdp_write(0, 4);

	/* Start latency calculation by setting LE bit */
	ret = spwtdp_initiator_conf(tdp,6,SPWTDP_TDP_ENABLE|SPWTDP_LATENCY_ENABLE);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when configuring initiator SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	ret = spwtdp_interrupt_unmask(tdp,SPWTDP_IRQ_DIT|SPWTDP_IRQ_DIR);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when configuring initiator SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	/* Make calculations*/
	for (i=0; i<LATENCY_COUNT; i++) {
		/* Wait for ISR status variable */
		while ( gists != 0x00000030 )
		{}
		/* Read the Time Stamp TX and RX values of initiator */
		ret = spwtdp_tsrx_et_get(tdp,&i_tsrx);
		if (ret!=SPWTDP_ERR_OK) {
			printf("Failure when getting TSRXET SPWTDP(ret=%d). Aborting", ret);
			exit(0);
		}
#ifdef DEBUG_ET_SAMPLING_TIMES
		log_tmr();
#endif
		ret = spwtdp_tstx_et_get(tdp,&i_tstx);
		if (ret!=SPWTDP_ERR_OK) {
			printf("Failure when getting TSTXET SPWTDP(ret=%d). Aborting", ret);
			exit(0);
		}
#ifdef DEBUG_ET_SAMPLING_TIMES
		log_tmr();
#endif
		/* Reinit ISR status variable */
		gists = 0;

		remote_spwtdp_read(0x60, 0x98-0x60);

		/* Read the Time Stamp TX and RX values of target*/
		remote_read_et_all((unsigned int *)&spwtdp.ts_rx_ctrl, &t_tsrx);
		remote_read_et_all((unsigned int *)&spwtdp.ts_tx_ctrl, &t_tstx);

		/* Do latency calculations:
		 * ((i.TsRX-i.TsTX) - (t.TsTX-t.TsRX)) / 2
		 */

		spwtdp_et_sub(&i_tsrx, &i_tstx, &i_diff);
		spwtdp_et_sub(&t_tstx, &t_tsrx, &t_diff);
		spwtdp_et_sub(&i_diff, &t_diff, &result_diff[i]);

#ifdef DEBUG_LATENCY
		/*tdp_regs->conf[0] &= ~(1<<16);*/
		spwtdp_et_print(&result_diff[i],"Result");
		/*exit(0);*/
#endif
	}

	/* Disable latency calculation by clearing LE bit */
	ret = spwtdp_initiator_conf(tdp,6,SPWTDP_TDP_ENABLE|SPWTDP_LATENCY_DISABLE);
	if (ret!=SPWTDP_ERR_OK) {
		printf("Failure when configuring initiator SPWTDP(ret=%d). Aborting", ret);
		exit(0);
	}
	spwtdp.conf[0] &= ~(1<<16);
	remote_spwtdp_write(0, 4);

	memset(&total, 0, sizeof(spwtdp_time_t));
	memset(&max, 0, sizeof(spwtdp_time_t));
	memset(&min, 0xff, sizeof(spwtdp_time_t));
	total.preamble=0x2f00;
	max.preamble=0x2f00;
	min.preamble=0x2f00;
	for (i=0; i<LATENCY_COUNT; i++) {
		spwtdp_et_add(&result_diff[i], &total, &total);

		if (spwtdp_et_cmp(&max, &result_diff[i]) > 0)
			memcpy(&max, &result_diff[i], sizeof(spwtdp_time_t));

		if (spwtdp_et_cmp(&min, &result_diff[i]) < 0)
			memcpy(&min, &result_diff[i], sizeof(spwtdp_time_t));

	}

	spwtdp_et_print(&total, "TOTAL");
	spwtdp_et_print(&min, "MIN SAMPLE");
	spwtdp_et_print(&max, "MAX SAMPLE");

	total_coarse = total_fine = 0;
	if (spwtdp_et_to_uint(&total, &total_coarse, &total_fine)) {
		puts("fine time must be limited to 64 bits for this example");
		exit(0);
	}
	if (total_coarse > 0) {
		puts("Example assumes average to be covered by fine time only. Aborting");
		exit(0);
	}
	latency_fine = total_fine / (LATENCY_COUNT);
	printf("AVERAGE FINE TIME LATENCY: %llu\n", latency_fine);
	latency_fine = latency_fine / 2;
	printf("AVERAGE FINE TIME LATENCY / 2: %llu\n", latency_fine);

	/* Update Target with latency detected values
	 *
	 * For now we assume that latency is convered by the 2 least
	 * significant byte of fine time.
	 */
	printf("Writing the latency %llu to target registers\n", latency_fine);
	if (latency_fine > 65025) {
		printf("Too big a latency value for this example to work\n");
		exit(0);
	}
	for (i = 0; i < fine + coarse; i++) {
		((unsigned char *)spwtdp.lat_et)[i] = 0;
		if (latency_fine < 256) {
			if (i == (fine + coarse - 1))
				((unsigned char *)spwtdp.lat_et)[i] = latency_fine & 0xff;
		}
		if (latency_fine > 255) {
			if (i == (fine + coarse - 1))
				((unsigned char *)spwtdp.lat_et)[i] = latency_fine % 0x100;
			if (i == (fine + coarse - 2))
				((unsigned char *)spwtdp.lat_et)[i] = latency_fine / 0x100;
		}
	}
	remote_spwtdp_write(0xa4, 5*4);

	/* Resync the register from target to see the last values */
	/* remote_spwtdp_read(0, sizeof(spwtdp)); */
	remote_spwtdp_read(0, 0x0cc); /* SPWTDP core in connected GR-RASTA system does not have the
									 external datation part of the spwtdp_regs, so we can only read till 0x0cc instead of 0x190=sizeof(spwtdp)*/

	printf("Example successfully completed\n");


#ifdef TOGGLE_TGT_ME
	/* Enable this to sync Remote Taget SPWTDP registers over to local
	 * initiator memory every 10 RTEMS system clock ticks. The Mitigation
	 * bit of the target is toggled every 100 ticks just as an example
	 * how to write the CONF[0] Register over RMAP.
	 */
	memset(&spwtdp, 0, sizeof(spwtdp));
	i = 0;
	while (1) {
		/* status = remote_spwtdp_read(0, sizeof(spwtdp)); */
		status = remote_spwtdp_read(0, 0x0cc); /* SPWTDP core in connected GR-RASTA system does not have the
												  external datation part of the spwtdp_regs, so we can only read till 0x0cc instead of 0x190=sizeof(spwtdp)*/
		if (status != 0)
			printf("Error in spwtdp_read() Status=%d\n", status);
#if 0 /* example how to toggle Mitigation Enable bit */
		if (i == 10) {
			printf("CONF[0] before: 0x%x\n", spwtdp.conf[0]);
			i = 0;
			if (spwtdp.conf[0] & 0x8)
				spwtdp.conf[0] &= ~0x8;
			else
				spwtdp.conf[0] |= 0x8;
			status = remote_spwtdp_write(0,4);/* Sync only CONF[0]*/
			if (status != 0)
				printf("Error in spwtdp_write() Status=%d\n", status);
		}
#endif
		rtems_task_wake_after(10);
		i++;
	}
#endif

	/*	for ( i=0; i<nospw; i++)
		dev_cleanup(i);
		tasks_stop = 1;
		rtems_task_wake_after(8);*/

	printf("\n\nEXAMPLE COMPLETED.\n\n");
	exit(0);
}

rtems_task link_ctrl_task(rtems_task_argument unused)
{
	int i, run;
	struct grspw_device *dev;
	spw_link_state_t state;

	printf("Started link control task\n");

	while (tasks_stop == 0) {
		for (i = 0; i < nospw; i++) {
			dev = &devs[i];
			if (dev->dh == NULL)
				continue;

			/* Check if link status has changed */
			state = grspw_link_state(dev->dh);
			run = 0;
			if (state == SPW_LS_RUN)
				run = 1;
			if ((run && dev->run == 0) || (run == 0 && dev->run)) {
				if (run)
					printf("GRSPW%d: link state entering run-state\n", i);
				else
					printf("GRSPW%d: link state leaving run-state\n", i);
				dev->run = run;
			}
		}
		rtems_task_wake_after(4);
	}

	printf(" Link control task shutdown\n");

	rtems_task_delete(RTEMS_SELF);
}

rtems_task dma_task(rtems_task_argument unused)
{
	int i;
	struct grspw_device *dev;

	printf("Started DMA control task\n");

	while (tasks_stop == 0) {
		rtems_semaphore_obtain(dma_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);

		for (i = 0; i < nospw; i++) {
			dev = &devs[i];
			if (dev->dh == NULL)
				continue;

			dma_process(dev);
		}
		rtems_semaphore_release(dma_sem);
		rtems_task_wake_after(1);
	}

	printf(" DMA task shutdown\n");

	rtems_task_delete(RTEMS_SELF);
}

int dma_process(struct grspw_device *dev)
{
	int cnt, rc, prnt_pkt;
	struct grspw_list lst;
	struct grspw_pkt *pkt;
	unsigned char *c;
#ifdef DEBUG_SPW_TX
	int i;
#endif

	/* Prepare receiver with packet buffers */
	if (dev->rx_buf_list_cnt > 0) {
		rc = grspw_dma_rx_prepare(dev->dma[0], 0, &dev->rx_buf_list,
				dev->rx_buf_list_cnt);
		if (rc != 0) {
			printf("rx_prep failed %d\n", rc);
			return -1;
		}
		/*printf("GRSPW%d: Prepared %d RX packet buffers for future "
		  "reception\n", dev->index, dev->rx_list_cnt);*/
		grspw_list_clr(&dev->rx_buf_list);
		dev->rx_buf_list_cnt = 0;
	}

	/* Try to receive packets on receiver interface */
	grspw_list_clr(&lst);
	cnt = -1; /* as many packets as possible */
	rc = grspw_dma_rx_recv(dev->dma[0], 0, &lst, &cnt);
	if (rc != 0) {
		printf("rx_recv failed %d\n", rc);
		return -1;
	}
	if (cnt > 0) {
#ifdef DEBUG_SPW_RX
		printf("GRSPW%d: Recevied %d packets\n", dev->index, cnt);
#endif
		for (pkt = lst.head; pkt; pkt = pkt->next) {
			prnt_pkt = 0;
			if ((pkt->flags & RXPKT_FLAG_RX) == 0) {
				printf(" PKT not received.. buf ret\n");
				continue;
			} else if (pkt->flags &
					(RXPKT_FLAG_EEOP | RXPKT_FLAG_TRUNK)) {
				printf(" PKT RX errors:");
				if (pkt->flags & RXPKT_FLAG_TRUNK)
					printf(" truncated");
				if (pkt->flags & RXPKT_FLAG_EEOP)
					printf(" EEP");
				printf(" (0x%x)", pkt->flags);
				prnt_pkt = 1;
			}
#ifdef DEBUG_SPW_RX
			else {
				printf(" PKT");
				prnt_pkt = 1;
			}
#endif
			if (prnt_pkt == 1) {
				c = (unsigned char *)pkt->data;
				printf(" of length %d bytes, 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x...\n",
						pkt->dlen, c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
			}
		}

		/* Reuse packet buffers by moving packets to rx_list */
		grspw_list_append_list(&dev->rx_list, &lst);
		dev->rx_list_cnt += cnt;
	}

	/*** TX PART ***/

	/* Reclaim already transmitted buffers */
	cnt = -1; /* as many packets as possible */
	grspw_list_clr(&lst);
	rc = grspw_dma_tx_reclaim(dev->dma[0], 0, &lst, &cnt);
	if (rc != 0) {
		printf("tx_reclaim failed %d\n", rc);
		exit(0);
	}
	/* put sent packets in end of send queue for retransmission */
	if (cnt > 0) {
		/*printf("GRSPW%d: Reclaimed %d TX packet buffers\n",
		  dev->index, cnt);*/
		/* Clear transmission flags */
		pkt = lst.head;
		while (pkt) {
			if ((pkt->flags & TXPKT_FLAG_TX) == 0)
				printf(" One Packet TX failed\n");
			else if (pkt->flags & TXPKT_FLAG_LINKERR)
				printf(" One Packet with TX errors\n");
			pkt = pkt->next;
		}

		grspw_list_append_list(&dev->tx_buf_list, &lst);
		dev->tx_buf_list_cnt += cnt;
	}

	/* Send packets in the tx_list queue */
	if (dev->tx_list_cnt > 0) {
#ifdef DEBUG_SPW_TX
		printf("GRSPW%d: Sending %d packets\n", dev->index,
				dev->tx_list_cnt);
		for (pkt = dev->tx_list.head; pkt; pkt = pkt->next) {
			printf(" PKT of length %d bytes,", pkt->hlen+pkt->dlen);
			for (i = 0; i < pkt->hlen+pkt->dlen && i < 8; i++) {
				if (i < pkt->hlen)
					c = i + (unsigned char *)pkt->hdr;
				else
					c = i - pkt->hlen + (unsigned char *)pkt->data;
				printf(" 0x%02x", *c);
			}
			printf("\n");
		}
#endif
		rc = grspw_dma_tx_send(dev->dma[0], 0, &dev->tx_list,
				dev->tx_list_cnt);
		if (rc != 0) {
			printf("tx_send failed %d\n", rc);
			exit(0);
		}
		dev->tx_list_cnt = 0;
		grspw_list_clr(&dev->tx_list);
	}
	return 0;
}
