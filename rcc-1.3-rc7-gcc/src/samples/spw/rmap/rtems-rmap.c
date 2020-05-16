/*
 * RTEMS SpaceWire RMAP Protocol example. The
 * example communicates with a GRLIB target including a GRSPW2 core using RMAP and SpW Interrupts. 
 * This software demonstrates to following:
 *  - Initiator GRSPW2 initialization using GRSPW_PKT driver
 *  - Initiator GRSPW2 link control and DMA handling using GRSPW_PKT driver
 *  - RMAP communication to Target using GRSPW_PKT driver and RMAP-ASYNC stack
 *
 * The local and remote hardware required:
 *  - GRSPW2
 *  - Initiator SpWX connector must be connected to SpWX connector of Target (choosen in configuration)
 *
 * The application consists of two threads:
 *
 *  INIT. Initialization and start task. Make basic initialization and
 *        creates the below task:
 *
 *  TA01. Example Task
 *
 * The Example task does the following:
 * 1) Start SPW GRSPW core to be used
 * 2) Set up asynchronous RMAP stack
 * 3) Initialize target (TODO)
 * 4) Check that RMAP transmission is working
 * 5) Initialize data arrays (used to transfer and receive data)
 * 6) Perform write and read cycle using transmission type 1
 * 7) Perform write and read cycle using transmission type 2
 *
 * There are two types of transmission
 * Type1: Perform a complete data transmission, either RMAP_READ or RMAP_WRITE, 
 * sending each packet one by one (including command and reply). 
 * Type2: Perform a complete data transmission, either RMAP_READ or RMAP_WRITE, 
 * preparing all packets and then sending them at once. Please note that this 
 * method does not include any mechanism that deals with the case in which the 
 * number of packets to send exceeds the maximum RMAP packets.
 * 
 * This example is based on the SPWTDP example.
 *
 * The example can be configured with grmon by setting a breakpoint in <rmap_conf_label>
 * and changing global variables. Those variables are print at the begining of the example.
 * remote_base_address: Base address on Remote. This is the address to which the data is copied. 
 * remote_dst_address: SpW Destination address. 
 * remote_dst_key: SpW Destination Key
 * source_src_address: SpW Source address
 * rmap_dev_idx: This parameter chooses which initiator GRSPW2 core is used
 * route_entry { nr: Number of addresses in dstadr array,  dstadr[16]: Path Addresses }
 * routetab[ROUTE_TO]: Route entry from source to destiny.
 * routetab[ROUTE_FROM]: Route entry from destiny to source
 * path_addressing: Use path addressing (!=0) or not (0)
 * rmap_pkt_size: Size of data in RMAP packets
 * rmap_data_bytes: Data bytes to send over RMAP
 *
 *  COPYRIGHT (c) 2016
 *  Cobham Gaisler.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 * Author: Javier Jalle, Cobham Gaisler
 * Contact: support@gaisler.com
 *
 */

/****** CONFIGURATION OF RMAP EXAMPLE *****
 *
 * Remote Settings:
 */
#define R_RMAP_DSTADR 0xfe
#define R_RMAP_DSTKEY 0x00
#define R_RMAP_BASEADR 0x00100000
unsigned int remote_base_address = R_RMAP_BASEADR; /* Base address on Remote. This is the address to which the data is copied. */
unsigned int remote_dst_address = R_RMAP_DSTADR; /* SpW Destination address. */
unsigned int remote_dst_key = R_RMAP_DSTKEY; /* SpW Destination Key */

/* Source settings. */
#define S_RMAP_SRCADR 0x20
unsigned int source_src_address = S_RMAP_SRCADR; /* SpW Source address */

/* GRSPW device used to communicate with target. Note only SpW0 tested. */
#define RMAP_SPW_DEV_IDX 0 
int rmap_dev_idx = RMAP_SPW_DEV_IDX; /* This parameter chooses which initiator GRSPW2 core is used */
#define DEVS_MAX 1 /* Number of GRSPW2 cores in the initiator */

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
	unsigned char nr;		/* Number of addresses in dstadr array */
	unsigned char dstadr[16];	/* Path Addresses */
};

#define SPW_PORT_INI 1
#define SPW_PORT_TAR 1
#define AMBA_PORT_INI 9 /* For DEV_IDX=0, AMBA_PORT=9 */ 
#define AMBA_PORT_TAR 5 
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

/****** Packet configuration 
 *
 * Choose the packet size here. There are two packet sizes important for this example: 
 * i) MAX_SPW_PKT_SIZE defines the size of the SPW packets. This parameters is used to allocate an array that holds MAX_SPW_PACKETS, 
 *      which is used as a packet buffer. Be careful not to exceed the available memory with this parameters, since a wrong confiration
 *      will lead to an internal error on RTEMS boot malloc initialization.
 * ii) rmap_pkt_size: This is the size of the data for each RMAP packet. The maximum packet size is MAX_SPW_PKT_SIZE - 32 to allow space for RMAP headers.
 *
 * In addition:
 * iii) rmap_data_bytes: This parameter defines the size of data to be send over RMAP.
 * 
 * */
#define MAX_SPW_PKT_SIZE 1*1024
#define MAX_SPW_PACKETS 16
int rmap_pkt_size = MAX_SPW_PKT_SIZE - 32;
//int rmap_pkt_size = 20*1024;
int rmap_data_bytes = 4*1024;
//int rmap_data_bytes = 16*20*1024;

/****** Data configuration 
 *
 * Writedata: the data to be written to the TARGET 
 * Readdata: holds the data read from TARGET
 * data_initialize(): Set each value in writedata to an unique value and each value in readdata to zero.
 */
#define MAX_DATA 8*1024 
#define MAX_DATA_BYTES MAX_DATA*4 
unsigned int writedata[MAX_DATA];
unsigned int readdata[MAX_DATA];
/* Used to initialize data arrays */
int data_initialize();


/****** DEBUG OPTIONS */
/* Enable to debug SpW Traffic */
/*#define DEBUG_SPW*/
/*#define PRINT_GRSPW_RESET_CFG*/
/*#define DBG_PROGRESS_BAR*/
#ifdef DEBUG_SPW
 #define DEBUG_SPW_TX
 #define DEBUG_SPW_RX
#endif


/****** RTEMS configuration 
 *
 */

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
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_DRIVERS 32
#define CONFIGURE_MAXIMUM_PERIODS             1

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (40 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_INIT_TASK_ATTRIBUTES      RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT


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

/*******************************************/

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

#include "config.c"

#define GRMON_LABEL(x) asm volatile(#x":\nnop;\n")

extern int router_setup_custom(void);

#include <grlib/grspw_pkt.h>
#include "grspw_pkt_lib.h"

#include "rmap.h"

int nospw = 0;
int tasks_stop = 0;

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

static struct grspw_device devs[DEVS_MAX];

/* SpaceWire packet payload (data) content layout */
struct spwpkt {
	struct grspw_pkt p;
	unsigned long long data[MAX_SPW_PKT_SIZE/8+1]; /* 32 bytes of data - 2byte data-header (8 extra bytes to avoid truncated bad packets)*/
	unsigned long long hdr[256/8+2]; /* Max header size and extra room for RMAP stack */
};

/* All packet buffers used by application */
struct spwpkt pkts[DEVS_MAX][MAX_SPW_PACKETS];

rtems_task test_app(rtems_task_argument ignored);
rtems_id tid;

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
		.rmap_cfg = 0,		/* Disable RMAP */
		.rmap_dstkey = 0,	/* No RMAP DESTKEY needed when disabled */
		.tc_cfg = TCOPTS_EN_TX,/* Enable TimeCode */
		.tc_isr_callback = NULL,/* TimeCode ISR */
		.tc_isr_arg = NULL,	/* No TimeCode ISR Argument */
		.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
		.chan =
		{
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = MAX_SPW_PKT_SIZE,
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
                for (j = 0, pkt = &pkts[i][0]; j < MAX_SPW_PACKETS; j++, pkt = &pkts[i][j]) {
                        pkt->p.pkt_id = (i << 8)+ j; /* unused */
                        pkt->p.data = &pkt->data[0];
                        pkt->p.hdr = &pkt->hdr[0];
                        if (j < (MAX_SPW_PACKETS/2)) {
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

	printf("Initializing SpaceWire device %d\n", idx);

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
	dev->cfg.adrcfg.def_addr = source_src_address;
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
		printf(" NOTE: running on SPW-ROUTER DMA SpaceWire link (no link-state available)\n");
        return 1;
    }else{
	    if (grspw_link_state(dev->dh) != SPW_LS_RUN)
		    return 0;
    }
	return 1;
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

int dma_prepare_receive_packets(struct grspw_device *dev);
int dma_reclaim_transmit_packets(struct grspw_device *dev);
int dma_receive(struct grspw_device *dev);
int dma_transmit(struct grspw_device *dev);

struct grspw_device *rmap_dev;
void *rmap_stack;

struct grspw_pkt *dma_alloc_tx_buf(struct grspw_device *dev)
{
	struct grspw_pkt *pkt;

	/* Allocate SpW packet buffer to hold RMAP request */
	pkt = dev->tx_buf_list.head;
	if (pkt == NULL) /* no free packets left */
		goto out;
	dev->tx_buf_list.head = pkt->next;
	if (pkt->next == NULL)
		dev->tx_buf_list.tail = NULL;
	dev->tx_buf_list_cnt--;

out:
	return pkt;
}

int dma_sched_tx_pkt(struct grspw_device *dev, struct grspw_pkt *pkt)
{
	/* Put TX packet into transmission queue */
	grspw_list_append(&dev->tx_list, pkt);
	dev->tx_list_cnt++;

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

/*
 * Transform RMAP CRC header skip option RMAP library (rmap_spw_pkt->options)
 * to GRSPW packet driver flags (grspw_pkt->flags)
 */
static unsigned short rmap_options_to_pktflags(int options)
{
	const int PKT_OPTION_HDR_CRC_SKIPLEN_BIT = 8;
	return (options & PKT_OPTION_HDR_CRC_SKIPLEN_MASK) >> PKT_OPTION_HDR_CRC_SKIPLEN_BIT;
}

#define RMAP_READ 0
#define RMAP_WRITE 1
/* Send one RMAP command. This function puts one RMAP packet on the transmit list. */
int rmap_tgt_send(unsigned int address, void *data, int len, int dstadr, int ttype, struct rmap_command * pcmd)
{
    struct rmap_command_read * prcmd;
    struct rmap_command_write * pwcmd;
	struct grspw_pkt *txpkt;
	struct rmap_spw_pkt rmappkt;
	int rc;
	static int cnt = 0;

	cnt++;

	/* Allocate SpW packet buffer to hold RMAP request */
	txpkt = dma_alloc_tx_buf(rmap_dev);
	if (txpkt == NULL) {
		printf(" No free transmit buffers available %d\n", cnt);
		return -2;
	}

    if (ttype == RMAP_READ){
        /* Cast pointer */
        prcmd = (struct rmap_command_read *) pcmd;
    	/* Describe RMAP request command */
	    prcmd->type = RMAP_CMD_RI;
    	prcmd->dstadr = dstadr;
	    prcmd->dstkey = remote_dst_key;
    	prcmd->address = address;
	    prcmd->length = len;
    	prcmd->datalength = 0;
	    prcmd->data = data; /* let RMAP stack copy data */

    	/* Let RMAP stack generate RMAP SpaceWire packet into TX buffer */
	    rmappkt.options = 0;
    	rmappkt.hdr = txpkt->hdr;
	    rmappkt.data = txpkt->data;
    }else if (ttype == RMAP_WRITE){
        /* Cast pointer */
        pwcmd = (struct rmap_command_write *) pcmd;
	    /* Describe RMAP request command */
    	pwcmd->type = RMAP_CMD_WIA;
	    pwcmd->dstadr = dstadr;
    	pwcmd->dstkey = remote_dst_key;
    	pwcmd->address = address;
	    pwcmd->length = len;
    	pwcmd->data = txpkt->data; /* Data to be analysed (not copied) */

	    /* Let RMAP stack generate RMAP SpaceWire packet into TX buffer */
    	rmappkt.options = 0;
	    rmappkt.hdr = txpkt->hdr;
    	/* Copy data to SpW buffer (we could implement zero-copy here...) */
	    memcpy(txpkt->data, data, len);
    }else{
		printf(" Wrong transmission type %d\n", ttype);
		return -6;
    }

	rc = rmap_send_async(rmap_stack, pcmd, &rmappkt);
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
	/* Put NOCRC part into flags. */
	txpkt->flags |= rmap_options_to_pktflags(rmappkt.options);

	/* Put TX packet into transmission queue */
	if (dma_sched_tx_pkt(rmap_dev, txpkt)) {
		printf("write(): Failed scheduling TX packet\n");
		return -5;
	}
    return 0;
}

/* Receive one RMAP command reply. This function checks if the reply of a RMAP packet is on the reception list. */
int rmap_tgt_recv(struct rmap_command * pcmd)
{
	struct grspw_pkt *rxpkt, *prevpkt;
	struct rmap_spw_pkt rmappkt;
	int j, found;

	/* Check for response */
	found = 0;
    rxpkt = rmap_dev->rx_list.head;
    prevpkt = NULL;
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
        if (rmap_recv_async(rmap_stack, &rmappkt, &pcmd) == 0) {
            found=1;
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

        if (pcmd->status != 0)
		    return -7;

    	/* The callers data buffer has now been updated */
        return 0;
	}else{
        /* Nothing found */
        return 1;
    }

	return 0;
}

/* Perform a complete READ RMAP packet. That includes creating the packet, transmiting it and waiting for the reply. */
int rmap_tgt_read(unsigned int address, void *data, int len, int dstadr)
{
	struct rmap_command_read cmd;
	struct rmap_command *pcmd;
	struct grspw_pkt *txpkt, *rxpkt, *prevpkt;
	struct rmap_spw_pkt rmappkt;
	int j, found, rc;
	static int cnt = 0;

	cnt++;

	/* Allocate SpW packet buffer to hold RMAP request */
	txpkt = dma_alloc_tx_buf(rmap_dev);
	if (txpkt == NULL) {
		printf(" No free transmit buffers available %d\n", cnt);
		return -2;
	}

	/* Describe RMAP request command */
	cmd.type = RMAP_CMD_RI;
	cmd.dstadr = dstadr;
	cmd.dstkey = remote_dst_key;
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
	/* Put NOCRC part into flags. */
	txpkt->flags |= rmap_options_to_pktflags(rmappkt.options);

	/* Put TX packet into transmission queue */
	if (dma_sched_tx_pkt(rmap_dev, txpkt)) {
		printf("write(): Failed scheduling TX packet\n");
		return -5;
	}

    dma_prepare_receive_packets(&devs[rmap_dev_idx]);
    dma_transmit(&devs[rmap_dev_idx]);

	/* Wait for response */
    /* TODO: Implement timeout mechanism. This is an endless loop otherwise. */
	found = 0;
    while (!found){ 
        dma_receive(&devs[rmap_dev_idx]);

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

	}
    
    dma_reclaim_transmit_packets(&devs[rmap_dev_idx]);

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

/* Perform a complete WRITE RMAP packet. That includes creating the packet, transmiting it and waiting for the reply. */
int rmap_tgt_write(unsigned int address, void *data, int len, int dstadr)
{
	struct rmap_command_write cmd;
	struct rmap_command *pcmd;
	struct grspw_pkt *txpkt, *rxpkt, *prevpkt;
	struct rmap_spw_pkt rmappkt;
	int j, found, rc;
	static int cnt = 0;

	cnt++;

	/* Allocate SpW packet buffer to hold RMAP request */
	txpkt = dma_alloc_tx_buf(rmap_dev);
	if (txpkt == NULL) {
		printf("write(): No free transmit buffers available %d\n", cnt);
		return -2;
	}

	/* Describe RMAP request command */
	cmd.type = RMAP_CMD_WIA;
	cmd.dstadr = dstadr;
	cmd.dstkey = remote_dst_key;
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
	/* Put NOCRC part into flags. */
	txpkt->flags |= rmap_options_to_pktflags(rmappkt.options);

	/* Put TX packet into transmission queue */
	if (dma_sched_tx_pkt(rmap_dev, txpkt)) {
		printf("write(): Failed scheduling TX packet\n");
		return -5;
	}

    dma_prepare_receive_packets(&devs[rmap_dev_idx]);
    dma_transmit(&devs[rmap_dev_idx]);

	/* Wait for response */
    /* TODO: Implement timeout mechanism. This is an endless loop otherwise. */
	found = 0;
    while (!found){ 
        dma_receive(&devs[rmap_dev_idx]);

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

	}

    dma_reclaim_transmit_packets(&devs[rmap_dev_idx]);

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

/* Init remote target */
int remote_rmap_init()
{
	/* Perform remote target initialization here */
	/* TODO: Initialization is done using GRMON */
	return 0;
}

/* Perform a complete data transmission, either RMAP_READ or RMAP_WRITE, preparing all packets and then sending them at once. 
 * Please note that this method does not include any mechanism that deals with the case in which the number of packets to send exceeds the 
 * maximum RMAP packets.*/
int remote_rmap_transmission_v2(void * data, int len, int ttype)
{
    int offset=0;
	int status;
    int bytes=rmap_pkt_size;
    int tot_packets = 1 + ((len - 1) / rmap_pkt_size);
    struct rmap_command cmd[MAX_SPW_PACKETS/2];
    struct rmap_command * pcmd;
    int pkt_id = 0;

    if (tot_packets > (MAX_SPW_PACKETS/2)){
	    printf("Failed executing RMAP read due to max packets: %d (%d)\n", tot_packets, MAX_SPW_PACKETS/2);
        /* TODO: Implement mechanism to deal with the case when there are more packets to send than the actual size of the pkt list. */
	    return -1;
    }

    /* Create packets */
    while (offset < len){
        #ifdef DBG_PROGRESS_BAR
        int barWidth = 15;
        int i;
        float progress = (float) offset/ (float) len;
        printf("Create packets [");
        int pos = barWidth * progress;
        for (i = 0; i < barWidth; ++i) {
            if (i < pos) printf("=");
            else if (i == pos) printf(">");
            else printf(" ");
        }
        printf("] %d%%\r", (int) ( progress * 100.0));
        fflush(stdout);
        #endif
        // Packet to use
        pcmd = &cmd[pkt_id];
        if (offset + bytes >= len){
            bytes = len - offset;
        }
        status = rmap_tgt_send(remote_base_address + offset, data + offset, bytes, remote_dst_address, ttype, pcmd);
    	if (status != 0) {
	    	printf("Failed executing RMAP read send from Target: %d\n", status);
	        return status;
	    }
        offset += bytes;
        pkt_id++;
    }
	    
    /* Start transmission */
    dma_prepare_receive_packets(rmap_dev);
    if (dma_transmit(rmap_dev) != 0) {
		printf("Failed executing RMAP read dma transmit from Target: %d\n", status);
        return status;
	}

    /* Check received packets */
    for (pkt_id =0; pkt_id < tot_packets; pkt_id++){
        pcmd = &cmd[pkt_id];
        status = rmap_tgt_recv(pcmd);
        while (status > 0){
            dma_receive(rmap_dev);
            status = rmap_tgt_recv(pcmd);
	    }
        if (status < 0){
            break;
        }
    }
    
    dma_reclaim_transmit_packets(rmap_dev);
	return status;
}

/* Perform a complete data transmission, either RMAP_READ or RMAP_WRITE, sending each packet one by one (including command and reply). */ 
int remote_rmap_transmission_v1(void * data, int len, int ttype)
{
    int offset=0;
	int status=-1;
    int bytes=rmap_pkt_size;

    while (offset < len){
        #ifdef DBG_PROGRESS_BAR
        int barWidth = 15;
        int i;
        float progress = (float) offset/ (float) len;
        printf("Create packets [");
        int pos = barWidth * progress;
        for (i = 0; i < barWidth; ++i) {
            if (i < pos) printf("=");
            else if (i == pos) printf(">");
            else printf(" ");
        }
        printf("] %d%%\r", (int) ( progress * 100.0));
        fflush(stdout);
        #endif
        // Packet to use
        if (offset + bytes >= len){
            bytes = len - offset;
        }
        if (ttype == RMAP_READ){
    	    status = rmap_tgt_read(remote_base_address + offset, data + offset, bytes, remote_dst_address);
        }else if (ttype == RMAP_WRITE){
    	    status = rmap_tgt_write(remote_base_address + offset, data + offset, bytes, remote_dst_address);
        }else{
		    printf(" Wrong transmission type %d\n", ttype);
            status = -1;
        }
    	if (status != 0) {
	    	printf("Failed executing RMAP read send from Target: %d\n", status);
	        return status;
	    }
        offset += bytes;
    }
	    
	return status;
}

/* Check that transmited data is correct*/
int check_data( unsigned int * got, unsigned int * expected, int bytes)
{
	int status=0;
    int i;
    int len = bytes/8;
    for (i=0; i < len; i++){
        if (got[i] != expected[i]){
            status++;
        }
    }
	return status;
}

rtems_task Init(
  rtems_task_argument ignored
)
{
    int i;

	/* Initialize Driver manager and Networking, in config.c */
	system_init();

    GRMON_LABEL(rmap_conf_label);
	printf("#### To change configuration of this example, you can set a breakpoint\n#### on label <rmap_conf_label> and modify the following global variables:\n");
    printf("## remote_base_address: 0x%08x /* Base address on Remote. This is the address to which the data is copied. */\n", remote_base_address);
    printf("## remote_dst_address: 0x%02x /* SpW Destination address. */\n", remote_dst_address);
    printf("## remote_dst_key: 0x%02x/* SpW Destination Key */\n", remote_dst_key);
    printf("## source_src_address: 0x%02x /* SpW Source address */\n", source_src_address);
    printf("## rmap_dev_idx: %d /* This parameter chooses which initiator GRSPW2 core is used */\n", rmap_dev_idx);
    printf("## route_entry { nr; /* Number of addresses in dstadr array */, dstadr[16];/* Path Addresses */}\n");
    printf("## routetab[ROUTE_TO]={%d,{%d,%d,%d, ...}\n", routetab[ROUTE_TO].nr,  routetab[ROUTE_TO].dstadr[0],  routetab[ROUTE_TO].dstadr[1],  routetab[ROUTE_TO].dstadr[2]);
    printf("## routetab[ROUTE_FROM]={%d,{%d,%d,%d, ...}\n", routetab[ROUTE_FROM].nr,  routetab[ROUTE_FROM].dstadr[0],  routetab[ROUTE_FROM].dstadr[1],  routetab[ROUTE_FROM].dstadr[2]);
    printf("## path_addressing: %d /* Use path addressing (!=0) or not (0) */\n", path_addressing);
    printf("## rmap_pkt_size: %d\n", rmap_pkt_size);
    printf("## rmap_data_bytes: %d /* Data bytes to send over RMAP */\n", rmap_data_bytes);
    printf("#### RTEMS RMAP example\n\n");

	/* Print device topology */
	/*drvmgr_print_topo();*/
	rtems_task_wake_after(4);

	/* Create Tasks for example application */
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '1' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid);

	/* Configure SPW router */
	printf("Setting up SpaceWire router\n");
    if (router_setup_custom()){
        printf("Failed router initialization.\n");
    }

	/* Detect GRSPW AMBA ports */
	nospw = grspw_dev_count();
	if (nospw < 1) {
		printf("Found no SpaceWire cores, aborting\n");
		exit(0);
	}
	if (nospw > DEVS_MAX) {
		printf("Limiting to %d SpaceWire devices\n", DEVS_MAX);
		nospw = DEVS_MAX;
	}

	/* Initialize GRSPW AMBA port */
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

	rtems_task_start(tid, test_app, 0);
	rtems_task_suspend(RTEMS_SELF);
}

rtems_task test_app(rtems_task_argument ignored)
{
	struct rmap_config rmap_cfg;
	int status;
    int dev_id = rmap_dev_idx;

	unsigned int got, expected;
    struct timespec t0, t1, t2, t3;
	double writetime, readtime;

    printf("\nTest built for at least %d AMBA ports.\n"
            "Note that the SpW-links will not be used.\n"
            "There are %d GRSPW cores in the system.\n"
            "System clock: %u us / tick\n",
            nospw, grspw_dev_count(),
            (unsigned int) rtems_configuration_get_microseconds_per_tick());
    printf("\n\n");

	/* Starting SpW DMA channel */
    printf("Starting GRSPW%d: ", dev_id);
    fflush(NULL);
    if (grspw_start(DEV(&devs[dev_id]))) {
        printf("Failed to initialize GRSPW%d\n", dev_id);
        exit(0);
    }
    printf("DMA Started Successfully\n");

	/* Make sure the SpaceWire link is in run-state before proceeding */
	if (dev_check_started(dev_id) == 0) {
		printf("Link to target is not in run-state\n");
		exit(0);
	}


	/* Set up asynchronos RMAP stack */
	rmap_dev = &devs[dev_id];
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
	rmap_stack = rmap_async_init(&rmap_cfg, MAX_SPW_PACKETS/2);
	if (!rmap_stack) {
		printf("RMAP stack initialization failed\n");
		exit(0);
	}

	/* Init target */

	status = remote_rmap_init();
	if (status) {
		printf("Remote initialization failed: %d\n", status);
		exit(0);
	}

	/*** Check RMAP is working ***/

	got = 0;
    /* Read a value, */
	if (rmap_tgt_read(remote_base_address, &got, 4, remote_dst_address)) {
		printf("Failed reading Target\n");
		exit(0);
	}
    /* ... change that value, */
	expected = got ^ -1;
	if (rmap_tgt_write(remote_base_address, &expected, 4, remote_dst_address)) {
		printf("Failed writing Target\n");
		exit(0);
	}

    /* ... read it again */
	if (rmap_tgt_read(remote_base_address, &got, 4, remote_dst_address)) {
		printf("Failed reading Target\n");
		exit(0);
	}

    /* and check. */
    if (got != expected){
		printf("Failed RMAP write/read test\n");
		exit(0);
    }
	printf("RMAP connection is working\n");

    /* Initialize data array */
    /*  Check that sizes are correct */
    if (rmap_data_bytes > MAX_DATA_BYTES){
	    printf("Transmission bytes (%d) bigger than actual data size (%d). Please adjust sizes.\n", rmap_data_bytes, MAX_DATA_BYTES);
	    exit(0);
    }
	printf("Initializing data array\n");
    if (data_initialize()){
	    printf("Failed initializing data array\n");
	    exit(0);
    }

	printf("\n\nStarting transmission type 1: one packet at a time\n\n");
	/* Write data array to remote target using RMAP */
    rtems_clock_get_uptime(&t0);
    if (remote_rmap_transmission_v1( (void *) writedata, rmap_data_bytes, RMAP_WRITE)){
        printf("Failed RMAP write cycle\n");
        exit(0);
    }
    rtems_clock_get_uptime(&t1);

	writetime = (float)t1.tv_sec + (float)t1.tv_nsec/1000000000;
	writetime = writetime - (float)t0.tv_sec - (float)t0.tv_nsec/1000000000;
	printf("RMAP write cycle completed in %f seconds\n", writetime);

	/* Read data array from remote target using RMAP */
    rtems_clock_get_uptime(&t2);
	if (remote_rmap_transmission_v1( (void *) readdata, rmap_data_bytes, RMAP_READ)){
		printf("Failed RMAP read cycle\n");
    	exit(0);
    }
    rtems_clock_get_uptime(&t3);

	readtime = (float)t3.tv_sec + (float)t3.tv_nsec/1000000000;
	readtime = readtime - (float)t2.tv_sec - (float)t2.tv_nsec/1000000000;
	printf("RMAP read cycle completed in %f seconds\n", readtime);

    /* Print data rates */
    int tot_packets = 1 + ((rmap_data_bytes - 1) / rmap_pkt_size);
    printf("Data bytes sent: %d, Packet size: %d, Packet count: %d\n", rmap_data_bytes, rmap_pkt_size, tot_packets);
	printf("Wrote %d bytes to target in %f seconds. WRITE RATE: %f KiB/s\n", rmap_data_bytes, writetime, ((float)rmap_data_bytes)/(writetime*1024) );
	printf("Read %d bytes from target in %f seconds. READ RATE: %f KiB/s\n", rmap_data_bytes, readtime, ((float)rmap_data_bytes)/(readtime*1024) );
	printf("Wrote %d packets to target in %f seconds. WRITE RATE: %f pkt/s\n", tot_packets, writetime, ((float) tot_packets)/(writetime) );
	printf("Read %d packets from target in %f seconds. READ RATE: %f pkt/s\n", tot_packets, readtime, ((float) tot_packets)/(readtime) );
    
    /* Check that received data is correct */
    status = check_data( (void *) readdata, (void *) writedata, rmap_data_bytes);
    if (status) {
		printf("Target copy failed: got %d errors in data array\n", status);
	    printf("\n\nEXAMPLE 1 UNSUCCESSFULLY COMPLETED.\n\n");
	}else{
    	printf("\n\nEXAMPLE 1 SUCCESSFULLY COMPLETED.\n\n");
    }
	
	printf("\n\nStarting transmission type 2: all packets at a time\n\n");
	/* Write data array to remote target using RMAP */
    rtems_clock_get_uptime(&t0);
    if (remote_rmap_transmission_v2( (void *) writedata, rmap_data_bytes, RMAP_WRITE)){
        printf("Failed RMAP write cycle\n");
        exit(0);
    }
    rtems_clock_get_uptime(&t1);

	writetime = (float)t1.tv_sec + (float)t1.tv_nsec/1000000000;
	writetime = writetime - (float)t0.tv_sec - (float)t0.tv_nsec/1000000000;
	printf("RMAP write cycle completed in %f seconds\n", writetime);

    /* Read data array from remote target using RMAP */
    rtems_clock_get_uptime(&t2);
	if (remote_rmap_transmission_v2( (void *) readdata, rmap_data_bytes, RMAP_READ)){
		printf("Failed RMAP read cycle\n");
    	exit(0);
    }
    rtems_clock_get_uptime(&t3);

	readtime = (float)t3.tv_sec + (float)t3.tv_nsec/1000000000;
	readtime = readtime - (float)t2.tv_sec - (float)t2.tv_nsec/1000000000;
	printf("RMAP read cycle completed in %f seconds\n", readtime);

    /* Print data rates */
    tot_packets = 1 + ((rmap_data_bytes - 1) / rmap_pkt_size);
    printf("Data bytes sent: %d, Packet size: %d, Packet count: %d\n", rmap_data_bytes, rmap_pkt_size, tot_packets);
	printf("Wrote %d bytes to target in %f seconds. WRITE RATE: %f KiB/s\n", rmap_data_bytes, writetime, ((float)rmap_data_bytes)/(writetime*1024) );
	printf("Read %d bytes from target in %f seconds. READ RATE: %f KiB/s\n", rmap_data_bytes, readtime, ((float)rmap_data_bytes)/(readtime*1024) );
	printf("Wrote %d packets to target in %f seconds. WRITE RATE: %f pkt/s\n", tot_packets, writetime, ((float) tot_packets)/(writetime) );
	printf("Read %d packets from target in %f seconds. READ RATE: %f pkt/s\n", tot_packets, readtime, ((float) tot_packets)/(readtime) );
	
    /* Check that received data is correct */
    status = check_data( (void *) readdata, (void *) writedata, rmap_data_bytes);
    if (status) {
		printf("Target copy failed: got %d errors in data array\n", status);
	    printf("\n\nEXAMPLE 2 UNSUCCESSFULLY COMPLETED.\n\n");
	}else{
    	printf("\n\nEXAMPLE 2 SUCCESSFULLY COMPLETED.\n\n");
    }
	
    dev_cleanup(rmap_dev_idx);
	
	exit(0);
}

int dma_prepare_receive_packets(struct grspw_device *dev)
{
	int rc;

	/* Prepare receiver with packet buffers */
	while (dev->rx_buf_list_cnt > 0) {
		rc = grspw_dma_rx_prepare(dev->dma[0], 0, &dev->rx_buf_list,
							dev->rx_buf_list_cnt);
		if (rc != 0) {
			printf("rx_prep failed %d\n", rc);
			return -1;
		}
        #ifdef DEBUG_SPW_RX
		printf("GRSPW%d: Prepared %d RX packet buffers for future "
		 "reception\n", dev->index, dev->rx_list_cnt);
        #endif
		grspw_list_clr(&dev->rx_buf_list);
		dev->rx_buf_list_cnt = 0;
	}

    return 0;
}

int dma_receive(struct grspw_device *dev)
{
	int cnt, rc, prnt_pkt;
	struct grspw_list lst;
	struct grspw_pkt *pkt;
	unsigned char *c;
#ifdef DEBUG_SPW_RX
	int i;
#endif

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
#ifdef DEBUG_SPW_RX
				printf(" of length %d bytes", pkt->dlen);
                for (i=0; i < pkt->dlen && i < 16; i++){
				    printf(", 0x%02x",c[i]);
                }
                printf("\n");
#else
				printf(" of length %d bytes, 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x...\n",
					pkt->dlen, c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
#endif
			}
		}

		/* Reuse packet buffers by moving packets to rx_list */
		grspw_list_append_list(&dev->rx_list, &lst);
		dev->rx_list_cnt += cnt;
	}
    return 0;
}

int dma_reclaim_transmit_packets(struct grspw_device *dev)
{
	int cnt, rc;
	struct grspw_list lst;
	struct grspw_pkt *pkt;

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
    return 0;
}

int dma_transmit(struct grspw_device *dev)
{
	int rc;
#ifdef DEBUG_SPW_TX
	struct grspw_pkt *pkt;
	int i;
	unsigned char *c;
#endif

	/* Send packets in the tx_list queue */
	if (dev->tx_list_cnt > 0) {
#ifdef DEBUG_SPW_TX
			printf("GRSPW%d: Sending %d packets\n", dev->index,
				dev->tx_list_cnt);
			for (pkt = dev->tx_list.head; pkt; pkt = pkt->next) {
				printf(" PKT of length %d bytes,", pkt->hlen+pkt->dlen);
                printf(" HDR(%d bytes):", pkt->hlen);
				for (i = 0; i < pkt->hlen; i++){
					c = i + (unsigned char *)pkt->hdr;
					printf(" 0x%02x", *c);
                }
                if (pkt->dlen){
                    printf(" DATA(%d bytes):", pkt->dlen);
	    			for (i = 0; i < pkt->dlen && i < 16; i++) {
		    			c = i + (unsigned char *)pkt->data;
			    		printf(" 0x%02x", *c);
				    }
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

int data_initialize() {
    int i;
    for (i = 0; i < MAX_DATA; i++){
        writedata[i] = i;
        readdata[i] = 0;
    }
    return 0;
}
