/*
 * RTEMS SpaceWire packet library demonstration application.
 *
 * This applicayion runs in one of three modes:
 *
 *   MODE_1 - No Router, SpW cable between SpW0 and SpW1 ports
 *
 *   MODE_2 - On-chip Router, solely between AMBA ports
 *
 *   MODE_3 - On-Chip Router, SpW cable between SpW0 and SpW1 ports
 *
 * The on-chip router is autodetectd, if not present only MODE_1 is available,
 * if router present ROUTE_TRAFIC_EXTERNAL selects MODE_2 (undefined) or
 * MODE_3 (defined).
 */

/* GRSPW Interface used for TX */
#ifndef TX_DEV
  #define TX_DEV 0
#endif
/* GRSPW Interface used for RX (must not be same as TX_DEV) */
#ifndef RX_DEV
  #define RX_DEV 1
#endif

/* Node ID number of RX and TX SpW nodes */
#define TX_DEV_NODEID 0x1
#define RX_DEV_NODEID 0x2

/* ROUTER CONFIG: Send Data from AMBA Port -> AMBA Port, or from AMBA port ->
 * SpW port -> SpW Port -> AMBA Port. This requires SpW port 0 connected to
 * Spw Port 1 using SpW Cable. It also requires the routing table to be
 * modified.
 * */
/*#define ROUTE_TRAFIC_EXTERNAL*/

/* Define INCLUDE_PCI_DRIVERS to include PCI host and PCI board drivers with
 * SpaceWire interfaces
 */
/*#define INCLUDE_PCI_DRIVERS*/


/*** END OF CONFIGURATION OPTIONS - start of test application ***/

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

/* Configure PCI Library to auto configuration. This can be substituted with
 * a static configuration by setting PCI_LIB_STATIC, see pci/. Static
 * configuration can be generated automatically by print routines in PCI
 * library.
 */
#define RTEMS_PCI_CONFIG_LIB
/*#define CONFIGURE_PCI_LIB PCI_LIB_STATIC*/
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO

#include <rtems/confdefs.h>

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

#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPW_ROUTER /* SpaceWire Router  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW2     /* SpaceWire Packet driver */

#ifdef INCLUDE_PCI_DRIVERS
/*#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF*//* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#endif

/*******************************************/

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "../../config.c"

rtems_task test_app(rtems_task_argument ignored);

rtems_id tid;

rtems_task Init(
  rtems_task_argument ignored
)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	drvmgr_print_topo();
	rtems_task_wake_after(4);

	/* Run SpaceWire Test application */
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '1' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid);

	rtems_task_start(tid, test_app, 0);
	rtems_task_suspend( RTEMS_SELF );
}

#include <grlib/grspw_pkt.h>
#include "grspw_pkt_lib.h"

void app_tc_isr(void *data, int tc);
int tc_irqs;

#define PKT_SIZE 32

struct grspw_device {
	/* GRSPW Device layout - must be the same as 'struct grspw_dev' */
	void *dh;
	void *dma[4];
	int index;
	struct grspw_hw_sup hwsup;

	/* Test structures */
	struct grspw_config cfg;
	struct route_entry *routetab;
};
#define DEV(device) ((struct grspw_dev *)(device))

static struct grspw_device devs[2];

struct grspw_config dev_configs[2] = 
{
	/*** GRSPW[0] ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = TX_DEV_NODEID,
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
		.rmap_cfg = 0,		/* Disable RMAP */
		.rmap_dstkey = 0,	/* No RMAP DESTKEY needed when disabled */
		.tc_cfg = TCOPTS_EN_TX,	/* Enable TimeCode Transmission */
		.tc_isr_callback = NULL,/* No TimeCode ISR */
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
	},

	/*** GRSPW[1] ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = RX_DEV_NODEID,
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
		.rmap_cfg = 0,		/* Disable RMAP */
		.rmap_dstkey = 0,	/* No RMAP DESTKEY needed when disabled */
		.tc_cfg = TCOPTS_EN_RX,	/* Enable TimeCode Reception. And generate IRQ upon reception */
		.tc_isr_callback = app_tc_isr,/* TimeCode ISR */
		.tc_isr_arg = &devs[1],	/* TimeCode ISR Argument */
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
	},
};

extern int router_setup_custom(void);

/* SpaceWire parameters */
#define SPW_PROT_ID 155

/* SpaceWire destinations supported in routing table */
#define ROUTE_MAX 256

/* mark where we must correct routing table for different number of SpW ports
 * when the SpceWire router is present.
 */
#define ROUTER_SPWLINK_NO 0
int router_spwlink_no = -1; /* by default disabled (NO ROUTER) */

/* SpaceWire Routing table entry */
struct route_entry {
	unsigned char dstadr[16];	/* 0 terminates array */
};

/* Routing table when the SpW router is in design. We want to route one SpW
 * packet from AMBA port0 to AMBA port1. The address of AMBA port 0 and 1
 * depends on the number of SpaceWire ports, which is autodetected and the
 * below table is updated on the locations marked by ROUTER_SPWLINK_NO.
 *
 * The below table is made for a router with header-deletion and path
 * addressing.
 */
struct route_entry router_present_routetab[ROUTE_MAX] =
{
	/* DST */  /* {PATH/LOGICAL ADDRESS} */
#ifndef ROUTE_TRAFIC_EXTERNAL
/* from AMBA port -> AMBA port (no external interface) */
	/* 0x00 */ {{0, 0}},
	/* 0x01 */ {{1+ROUTER_SPWLINK_NO+0, 0x1, 0}},
	/* 0x02 */ {{1+ROUTER_SPWLINK_NO+1, 0x2, 0}},
	/* 0x03 */ {{1+ROUTER_SPWLINK_NO+2, 0x3, 0}},
	/* 0x04 */ {{1+ROUTER_SPWLINK_NO+3, 0x4, 0}},
	/* 0x05 */ {{1+ROUTER_SPWLINK_NO+4, 0x5, 0}},
	/* 0x06 */ {{1+ROUTER_SPWLINK_NO+5, 0x6, 0}},
	/* 0x07 */ {{1+ROUTER_SPWLINK_NO+6, 0x7, 0}},
	/* 0x08 */ {{1+ROUTER_SPWLINK_NO+7, 0x8, 0}},
#else
/* from AMBA port -> SpW port -> SpW port -> AMBA port (via external SpW interface)
 * requires SpW cable between SpW port0 and SpW port1
 */
	/* 0x00 */ {{0, 0, 0}},
	/* 0x01 */ {{1, 1+ROUTER_SPWLINK_NO+0, 0x1, 0}},
	/* 0x02 */ {{2, 1+ROUTER_SPWLINK_NO+1, 0x2, 0}},
	/* 0x03 */ {{1, 1+ROUTER_SPWLINK_NO+2, 0x3, 0}},
	/* 0x04 */ {{2, 1+ROUTER_SPWLINK_NO+3, 0x4, 0}},
	/* 0x05 */ {{1, 1+ROUTER_SPWLINK_NO+4, 0x5, 0}},
	/* 0x06 */ {{2, 1+ROUTER_SPWLINK_NO+5, 0x6, 0}},
	/* 0x07 */ {{1, 1+ROUTER_SPWLINK_NO+6, 0x7, 0}},
	/* 0x08 */ {{2, 1+ROUTER_SPWLINK_NO+7, 0x8, 0}},
#endif
};


/* Assuming direct connect (no SpW network) */
struct route_entry std_routetab[ROUTE_MAX] =
{
	/* DST */  /* {PATH/LOGICAL ADDRESS} */
	/* 0x00 */ {{0, 0}},
	/* 0x01 */ {{1, 0}},
	/* 0x02 */ {{2, 0}},
	/* 0x03 */ {{3, 0}},
	/* 0x04 */ {{4, 0}},
};

struct route_entry *routetab = &std_routetab[0];

/* SpaceWire packet payload (data) content layout */
struct pkt_hdr {
	unsigned char addr;
	unsigned char protid;
	unsigned char resv1; /* 0 or -1, stops packet */
	unsigned char resv2; /* Zero for now */
	unsigned int data[(4*8-4)/4];
};

struct spwpkt {
	struct grspw_pkt p;
	unsigned long long data[4]; /* 32 bytes of data - 4byte data-header */
	unsigned long long hdr[2]; /* up to 16byte header (path address) */
};

/* All packet buffers used by application */
struct spwpkt pkts[16];

int idx2devno[2] = {TX_DEV, RX_DEV};

/* Initialize packet header, and Data payload header */
void pkt_init_hdr(struct grspw_pkt *pkt, struct route_entry *route)
{
	int i;
	struct pkt_hdr *pkt_hdr = (struct pkt_hdr *)pkt->data;
	unsigned char *hdr = pkt->hdr;

	/* If path addressing we put non-first Destination Addresses in 
	 * header. route->dstadr[0] is always non-zero.
	 */
	i = 0;
	while ( route->dstadr[i+1] != 0 ) {
		hdr[i] = route->dstadr[i];
		i++;
	}
	/* Put last address in pkthdr->addr */
	pkt->hlen = i;
	pkt_hdr->addr = route->dstadr[i];
	pkt_hdr->protid = SPW_PROT_ID;
	pkt_hdr->resv1 = pkt_hdr->resv2 = 0;
}

void init_pkts(void)
{
	struct spwpkt *pkt;
	int i;

	memset(&pkts[0], 0, sizeof(pkts));
	for (i = 0, pkt = &pkts[0]; i < 16; i++, pkt = &pkts[i]) {
		pkt->p.pkt_id = i;
		pkt->p.data = &pkt->data[0];
		pkt->p.hdr = &pkt->hdr[0];
		if (i < 8) {
			/* RX Packets */
		} else {
			/* TX Packets */
			pkt->p.dlen = 32;
			memset(pkt->p.data+4, i, 32-4);
			pkt_init_hdr(&pkt->p, &routetab[RX_DEV_NODEID]);
		}
	}
}

int dev_init(int idx)
{
	struct grspw_device *dev = &devs[idx];
	struct grspw_link_state ls;
	int i, ctrl, clkdiv, stscfg;

	memset(dev, 0, sizeof(struct grspw_device));

	dev->index = idx;
	dev->dh = grspw_open(idx2devno[idx]);
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

	if (grspw_cfg_set(DEV(dev), &dev_configs[dev->index])) {
		grspw_close(dev->dh);
		return -1;
	}
	printf("\n\n---- APPLICATION CONFIGURATION ----\n");
	dev->cfg = dev_configs[dev->index];
	grspw_cfg_print(&dev->hwsup, &dev->cfg);
	printf("\n\n");

	/* This will result in an error if only one port available */
	if (dev->hwsup.nports < 2) {
		int port = 1;
		if ( grspw_port_ctrl(dev->dh, &port) == 0 ) {
			printf("Succeeded to select port1, however only one PORT on dev %d!\n", dev->index);
			return -1;
		} else {
			printf("PORT TEST SUCCEEDED\n"); /* should ret error */
		}
	}

	printf("\n\nBefore Link Start:\n");
	grspw_link_state_get(DEV(dev), &ls);
	grspw_linkstate_print(&ls);

	/* Try to bring link up at fastest clockdiv but do not touch
	 * start-up clockdivisor */
	clkdiv = -1;
	stscfg = -1;
	grspw_link_ctrl(dev->dh, NULL, &stscfg, &clkdiv);
	ctrl = LINKOPTS_ENABLE | LINKOPTS_AUTOSTART | LINKOPTS_START;
	clkdiv &= 0xff00;
	stscfg &= LINKSTS_MASK; /* all errors handled by driver */
	grspw_link_ctrl(dev->dh, &ctrl, &stscfg, &clkdiv);

	printf("\n\nAfter Link Start:\n");
	grspw_link_state_get(DEV(dev), &ls);
	grspw_linkstate_print(&ls);

	if ( (dev->hwsup.hw_version >> 16) == GAISLER_SPW2_DMA ) {
		printf("\n\n###NOTE: running on SPW-ROUTER DMA SpaceWire link:\n");
		printf(    "         there is no Link-state available\n");
	}

	grspw_stats_clr(dev->dh);

	for (i=0; i<dev->hwsup.ndma_chans; i++) {
		if (dev->dma[i])
			grspw_dma_stats_clr(dev->dma[i]);
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

	grspw_close(dev->dh);
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

#include <grlib/grspw_router.h>
extern struct router_hw_info router_hw;
int router_present = 0;

rtems_task test_app(rtems_task_argument ignored)
{
	int i, tot, txtot, rc, sendmax;
	struct grspw_list lst, rx_list, tx_list;
	struct grspw_device *txdev, *rxdev;
	int cnt, rx_list_cnt, tx_list_cnt;
	struct grspw_pkt *pkt;
	int rx_ready, rx_sched, rx_recv, rx_hwcnt;
	int tx_send, tx_sched, tx_sent, tx_hwcnt;
	struct grspw_stats stats;
	int tc;

	/* Initialize two GRSPW AMBA ports */
	printf("Setting up SpaceWire router\n");
	if (router_setup_custom()) {
		printf("Failed router initialization, assuming that it does not exists\n");
	} else {
		/* on-chip router found */
		if (router_hw.nports_amba < 2) {
			printf("Error. Router with less than 2 AMBA ports not supported\n");
			exit(0);
		}
#ifdef ROUTE_TRAFIC_EXTERNAL
		if (router_hw.nports_spw < 2) {
			printf("Error. Router with less than 2 SpW ports not supported\n");
			exit(0);
		}
#endif
		router_present = 1;
		printf("Router found. Configuring routing table for %d SpWs\n",
			router_hw.nports_spw);
		/* select routing table. Configure routing table for
		 * number of AMBA ports
		 */
		routetab = &router_present_routetab[0];
		for (i=1; i<5; i++)
#ifndef ROUTE_TRAFIC_EXTERNAL
			routetab[i].dstadr[0] += router_hw.nports_spw;
#else
			routetab[i].dstadr[1] += router_hw.nports_spw;
#endif
	}

	memset(devs, 0, sizeof(devs));
	for (i=0; i<2; i++) {
		if (dev_init(i)) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
		fflush(NULL);
	}

	/* Check link status */
	if (router_present == 0) {
		rc = 0;
		printf("\n\nChecking that links are in run-state\n");
		for (i=0; i<2; i++) {
			if (grspw_link_state(devs[i].dh) != SPW_LS_RUN) {
				printf("### GRSPW%d: not in run-state\n", i);
				rc = 1;
			}
		}
		if (rc != 0)
			exit(0);
		printf("All links are in run-state\n");
	}

	/* Initialize GRSPW */
	init_pkts();

	printf("\n\nStarting SpW DMA channels\n");
	for (i = 0; i < 2; i++) {
		printf("Starting GRSPW%d: ", i);
		fflush(NULL);
		if (grspw_start(DEV(&devs[i]))) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
		printf("Started Successfully\n");
	}

	txdev = &devs[0];
	rxdev = &devs[1];
	tot = 0;
	txtot = 0;

	/* Fill lists with initial packets */
	rx_list_cnt = 8;
	grspw_list_clr(&rx_list);
	for (i = 0; i < 8;  i++)
		grspw_list_append(&rx_list, &pkts[i].p);
	tx_list_cnt = 8;
	grspw_list_clr(&tx_list);
	for (i = 8; i < 16;  i++)
		grspw_list_append(&tx_list, &pkts[i].p);

	printf("Starting Packet processing loop. Transfer 16 packets.\n");
	while (1) {
		/*** RX PART ***/

		/* Prepare receiver with packet buffers */
		if (rx_list_cnt > 0) {
			rc = grspw_dma_rx_prepare(rxdev->dma[0], 0, &rx_list,
								rx_list_cnt);
			if (rc != 0) {
				printf("rx_prep failed %d\n", rc);
				exit(0);
			}
			printf("Prepared %d RX packet buffers for future "
			       "reception\n", rx_list_cnt);
			grspw_list_clr(&rx_list);
			rx_list_cnt = 0;
		}

		/* Try to receive packets on receiver interface */
		grspw_list_clr(&lst);
		cnt = -1; /* as many packets as possible */
		rc = grspw_dma_rx_recv(rxdev->dma[0], 0, &lst, &cnt);
		if (rc != 0) {
			printf("rx_recv failed %d\n", rc);
			exit(0);
		}
		if (cnt > 0)
			printf("Recevied %d packets\n", cnt);
		for (pkt = lst.head; pkt; tot++, pkt = pkt->next) {
			if ((pkt->flags & RXPKT_FLAG_RX) == 0) {
				printf("PKT[%d] not received.. buf ret\n", tot);
				continue;
			} else if (pkt->flags &
			           (RXPKT_FLAG_EEOP | RXPKT_FLAG_TRUNK)) {
				printf("PKT[%d] RX errors:", tot);
				if (pkt->flags & RXPKT_FLAG_TRUNK)
					printf(" truncated");
				if (pkt->flags & RXPKT_FLAG_EEOP)
					printf(" EEP");
				printf(" (0x%x)\n", pkt->flags);
				continue;
			}
			printf(" PKT[%d] of length %d bytes\n", tot, pkt->dlen);
		}

		/* Reuse packet buffers by moving packets to rx_list */
		if (cnt > 0) {
			grspw_list_append_list(&rx_list, &lst);
			rx_list_cnt += cnt;
		}

		if (tot >= 16)
			break;


		/*** TX PART ***/

		/* Reclaim already transmitted buffers */
		cnt = -1; /* as many packets as possible */
		grspw_list_clr(&lst);
		rc = grspw_dma_tx_reclaim(txdev->dma[0], 0, &lst, &cnt);
		if (rc != 0) {
			printf("tx_reclaim failed %d\n", rc);
			exit(0);
		}
		/* put sent packets in end of send queue for retransmission */
		if (cnt > 0) {
			printf("Reclaimed %d TX packet buffers\n", cnt);
			/* Clear transmission flags */
			pkt = lst.head;
			while (pkt) {
				if ((pkt->flags & TXPKT_FLAG_TX) == 0)
					printf(" One Packet TX failed\n");
				else if (pkt->flags & TXPKT_FLAG_LINKERR)
					printf(" One Packet with TX errors\n");
				pkt = pkt->next;
			}

			grspw_list_append_list(&tx_list, &lst);
			tx_list_cnt += cnt;
		}

		/* Transmit sendmax packets */
		if (txtot > 32) /* limit to 32 packets transmission */
			continue;

		sendmax = (tot & 0x3) + 1;
		grspw_list_clr(&lst);
		cnt = grspw_list_take_head_list(&tx_list, &lst, sendmax);
		if (cnt > 0) {
			tx_list_cnt -= cnt;
			rc = grspw_dma_tx_send(txdev->dma[0], 0, &lst, cnt);
			if (rc != 0) {
				printf("tx_send failed %d\n", rc);
				exit(0);
			}
			printf("Sent %d packets\n", cnt);
			txtot += cnt;
		}
	}

	printf("\n\nTransmitted %d and received %d packets.\n", txtot, tot);
	printf("TRANSFER EXAMPLE COMPLETED.\n\n");

	/* Send time-code 0x16, 0x17, 0x18. GRSPW devices has already been
	 * setup to TX and RX time-codes via the configuration structure.
	 * The time-code interrupt is also demonstrated.
	 */
	printf("\n\nDemonstrating Time-code interface\n");
	tc_irqs = 0;
	/* 1. Set TimeCode on Transmitter */
	tc = 0x15; /* TimeCode incremented before transmission */
	grspw_tc_time(txdev->dh, &tc);
	/* 2. Read Current TimeCode on receiver */
	tc = -1;
	grspw_tc_time(rxdev->dh, &tc);
	printf("RX Current Time-code (before send): 0x%02x\n", tc);
	/* 3. Verify that TimeCode support is enabled */
	tc = -1;
	grspw_tc_ctrl(rxdev->dh, &tc);
	if ((tc & TCOPTS_EN_RX) == 0) {
		printf("Time-code RX not enabled on Receiver\n");
		exit(0);
	}
	tc = -1;
	grspw_tc_ctrl(txdev->dh, &tc);
	if ((tc & TCOPTS_EN_TX) == 0) {
		printf("Time-code TX not enabled on Transmitter\n");
		exit(0);
	}
	/* 4. Enable interrupt generation on TimeCode reception.
	 * NOTE that since time-code is not old time-code + 1, the GRSPW
	 * will not generare a tick-out. So it is expected that only two
	 * ISR calls will be made, one for 0x17 and one for 0x18.
	 */
	tc = TCOPTS_EN_RX | TCOPTS_EN_RXIRQ;
	grspw_tc_ctrl(rxdev->dh, &tc);
	/* 5. Generate three TimeCodes and verify that they have arrived */
	grspw_tc_tx(txdev->dh);
	rtems_task_wake_after(4);
	tc = -1;
	grspw_tc_time(rxdev->dh, &tc);
	if (tc != 0x16) {
		printf("Time-code 0x16 was not received (0x%x). Which can be expected "
		       "since time-code value not previous time-code + 1. Continuing\n", tc);
	} else
		printf("TASK: TimeCode 0x%02x was received\n", tc);
	if (tc_irqs != 0) {
		/* this might happen if the time-code register already
		 * contained 0x15, which is not likely */
		printf("TASK: time-code resulted in tick-out on first\n");
		tc_irqs = 0; /* skip first tick-out */
	}
	grspw_tc_tx(txdev->dh);
	rtems_task_wake_after(4);
	tc = -1;
	grspw_tc_time(rxdev->dh, &tc);
	if (tc != 0x17) {
		printf("TimeCode 0x17 was not received. Aborting. 0x%x\n", tc);
		exit(0);
	}
	if (tc_irqs != 1) {
		printf("TimeCode ISR not called once: %d\n", tc_irqs);
		exit(0);
	
	}
	printf("TASK: TimeCode 0x%02x was received\n", tc);
	grspw_tc_tx(txdev->dh);
	rtems_task_wake_after(4);
	tc = -1;
	grspw_tc_time(rxdev->dh, &tc);
	if (tc != 0x18) {
		printf("TimeCode 0x18 was not received. Aborting. 0x%x\n", tc);
		exit(0);
	}
	if (tc_irqs != 2) {
		printf("TimeCode ISR not called twice: %d\n", tc_irqs);
		exit(0);
	}
	printf("TASK: Time-code 0x%02x was received\n", tc);

	printf("\n\nTime-code EXAMPLE COMPLETED. Printing driver stats\n\n");

	/* Print statistics */
	for (i=0; i<2; i++) {
		if (i == 0)
			printf("\n\n--- GRSPW TX Device ---\n");
		else
			printf("\n\n--- GRSPW RX Device ---\n");
		grspw_dma_rx_count(devs[i].dma[0], &rx_ready, &rx_sched, &rx_recv, &rx_hwcnt);
		grspw_dma_tx_count(devs[i].dma[0], &tx_send, &tx_sched, &tx_sent, &tx_hwcnt);
		printf(" DRVQ RX_READY: %d\n", rx_ready);
		printf(" DRVQ RX_SCHED: %d\n", rx_sched);
		printf(" DRVQ RX_RECV: %d\n", rx_recv);
		printf(" DRVQ RX_HWCNT: %d\n", rx_hwcnt);
		printf(" DRVQ TX_SEND: %d\n", tx_send);
		printf(" DRVQ TX_SCHED: %d\n", tx_sched);
		printf(" DRVQ TX_SENT: %d\n", tx_sent);
		printf(" DRVQ TX_HWCNT: %d\n", tx_hwcnt);

		grspw_stats_get(DEV(&devs[i]), &stats);
		grspw_stats_print(DEV(&devs[i]), &stats);
	}

	for ( i=0; i<2; i++)
		dev_cleanup(i);

	printf("\n\nEXAMPLE COMPLETED.\n\n");
	exit(0);
}

/* Interrupt handler for TimeCode reception on RXDEV */
void app_tc_isr(void *data, int tc)
{
	struct grspw_device *dev = data;

	printk("TCISR: received 0x%02x, 0x%08x\n", tc, (unsigned int) dev);
	tc_irqs++;
}
