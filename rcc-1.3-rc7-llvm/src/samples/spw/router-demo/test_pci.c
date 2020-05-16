/*
 * RTEMS SpaceWire packet library demonstration application modified for
 * NGMP research project - demonstrate PCI .
 *
 * The application requires the GRSPW Router. The application has been designed
 * to be able to run on several boards each with one or more SpaceWire nodes
 * on a SpaceWire network. In that case the routing table can easily be
 * updated to make packets travel off-chip onto a SpaceWire network.
 *
 * In the default configuration the data path of a SpaceWire packet is as
 * follows:
 *
 *  AMBAPort0 TX        ->  SpW0  ->  RASTA-IO Port 0 RX
 *  RASTA-IO Port 0 TX  ->  SpW0  ->  SpW1  ->  RASTA-IO Port 1 RX
 *  RASTA-IO Port 1 TX  ->  SpW1  -> AMBAPort1 RX
 *  AMBAPort1 TX        ->  SpW2  -> RASTA-IO Port 2 RX
 *  RASTA-IO Port 2 TX  ->  SpW2  -> AMBAPort0 RX
 *
 * In order to achive this both application's routing table (routetab,
 * initial_route) and cables must connected correctly. Make sure to
 * connect the three SpW cables in the following way:
 *  on-chip (router) SpW0 port <-> GR-RASTA-IO SpW0
 *  on-chip (router) SpW1 port <-> GR-RASTA-IO SpW1
 *  on-chip (router) SpW2 port <-> GR-RASTA-IO SpW2
 *
 * Device SpaceWire node ID:
 *  on-chip AMBA port0      - 0x1
 *  on-chip AMBA port1      - 0x2
 *  RASTA-IO AMBA/SpW port0 - 0x10
 *  RASTA-IO AMBA/SpW port1 - 0x11
 *  RASTA-IO AMBA/SpW port2 - 0x12
 *
 * This means that every packet will exercise 4 DMA on-chip channels and
 * 6 RASTA-IO DMA channles every time the packet travels one round-trip. The
 * number of round trips are counted and performance figures can be calculated
 * ontop of that.
 *
 * The packets are marked with a unique sequence number and contain 16-bit
 * word incremented data, this is to be able to verify packet RX/TX ordering
 * and data correctness. The packet sequence is verified for every received
 * packet, and optionally the data may also be verified for every reception.
 *
 * The test runs for LENGTH_SEC seconds, when the time has expired the test
 * stops sending on the "packet initiator" instead it collects all transmitted
 * packet by waiting for WAIT_SEC for all packets to be received at the
 * initiator. If the test is successful all packets the initiator has sent has
 * also been received, finaly the packet sequence and data content is verified
 * at the end.
 *
 * The option to only verify data at the end makes the test zero-copy, the
 * SpaceWire packet data is not read by the CPU. This makes it possible to
 * achieve high bitrates.
 *
 *
 * NGMP modifications
 * ------------------
 * In order to demonstrate L2-cache and coherence effects the test has been
 * modified so that MMU, IOMMU and L2-cache configuration is altered. To allow
 * this application to run on other platforms than NGMP without the required
 * extra hardware, the compile-time option TEST_NGMP can be undefined, or
 * defined for NGMP performance meassurements.
 *
 * The test can be configured to demonstrate different cache aspects, see below
 * description about the 8 different configurations determined by the dma_cfg
 * option. Note that it is assumed that the bootloader/GRMON sets up some
 * parts such as stack, enable/disable L2CACHE.
 *
 *
 * Configuration options
 * ---------------------
 * When the application is run from GRMON, GRMON can be used to change the
 * operation. For example the SpaceWire header and data buffers DMA location
 * can be altered, and the dma_cfg may be changed without rebuilding the
 * application:
 *
 * grmon> lo test_ngmp
 * grmon> wmem buf_hdr_start 0x01100000
 * grmon> wmem buf_data_start 0x01800000
 * grmon> wmem dma_cfg 8
 * grmon> stack 0x00ffffff
 * grmon> l2c disable
 * grmon> run
 *
 */

/* Link setup */

/* SpW links on SpaceWire Router determine which PATH addess the AMBA
 * targets has
 */
#define ROUTER_SPWLINK_NO 8
#define ROUTER_AMBA_PORTS 4

/* Number of SpaceWire Links to share the packet buffers with, each
 * SpaceWire Link have 2 Packet Queues per DMA Unit.
 */
#define SPW_LINKS_USED 5
#define SPW_DMA_CHANS_PER_LINK_USED 1 /* only 1 supported at this time */
#define SPW_DIRECTIONS_USED (SPW_LINKS_USED*SPW_DMA_CHANS_PER_LINK_USED*2)



/* Buffer setup */
#define SPW_BUF_KB_BLOCKS 1024		/* Number of kilobytes of buffer */
#define SPW_BUF_SIZE (SPW_BUF_KB_BLOCKS*1024) /* bytes */
#define HDR_SIZE 8
/*#define PKT_SIZE 128*/
#define PKT_SIZE 1024
/*#define PKT_SIZE 16384*/
#define PKT_DATA_SIZE (PKT_SIZE-1-1-1-1-4) /* - sizeof(pkt_hdr)... */
#define PKT_CNT  SPW_BUF_SIZE/PKT_SIZE/SPW_DIRECTIONS_USED
#define PKT_CNT_TOT PKT_CNT*SPW_DIRECTIONS_USED

/* Calculate buffer sizes */
#define BUF_HDR_SIZE (PKT_CNT_TOT * HDR_SIZE)
#define BUF_DATA_SIZE (PKT_CNT_TOT * PKT_SIZE)

/* Test setup */
/* Number of seconds to run Test */
/*#define TEST_LENGTH_SEC (60*60*12)*/
#define TEST_LENGTH_SEC (60*2)
/* Number of seconds to wait for all packets to be received */
#define TEST_END_WAIT_SEC 10
/* Skip RX check to save CPU and thereby get higher throughput figures
 * all packets are checked in the end any way...
 */
#define SKIP_CHECK_FOR_EVERY_RECEIVED_PACKET
/* SpaceWire Packet first sender (INITIATOR) device Index */
/* Skip TX check to save CPU and thereby get higher throughput figures
 * all packets are checked in the end any way...
 */
#define SKIP_CHECK_FOR_EVERY_TRANSMITTED_PACKET
/* SpaceWire Packet first sender (INITIATOR) device Index */
#define SPW_INITIATOR 0

/* Print GRSPW register content on boot prior to modification by SW */
#undef PRINT_GRSPW_RESET_CFG

/*
 * Interrupt configuration
 * =======================
 * This example shows how interrupt might be used together with the GRSPW
 * packet driver.
 *
 * Interrupts guarantees that all packets that have entered the SEND queue
 * will be transmitted, and all packet buffers in the RX-PREPARE READY queue
 * will be filled with data if packets are received. The GRSPW packet driver
 * has in internal "worker" task that process the descriptor table when
 * the interrupt handler detects that the descriptor table is becomming empty.
 * At least, this sample application has configured IRQs to happen every 124 or
 * 60 descriptor - minimum four descriptors remains.
 *
 * The task feeding the driver with packets exeutes in a rate monotonic
 * period of once every 2 ticks. Tick rate is set to 2 msec/tick. The example
 * uses the following assumsions:
 *  - 100MBit/s link ==> about 80MBit/s effective data transfer
 *  - 4 AMBA ports, 8 DMA units involved
 *    are required.
 *  - 1kB per packet
 *  - 1MB buffers => 128 packets per DMA channel
 *  - results in about 12.5ms between every 128packet being transmitted.
 *  - The task that feeds the driver will therefore be woken every 6ms
 *    to be sure to feed driver with buffers in time.
 *
 * When DMA generate interrupts, it is possible to use blocking I/O mode,
 * were the caller is blocked until a number of packets have been transmitted
 * or received. Enable blocking to demonstrate how blocking mode together
 * with a timeout can be used. One thread per DMA channel handles the 
 * transfers.
 * NOTE that the blocking mode example is designed with a large packet buffer
 * in mind, it needs at least 1MB buffer (SPW_BUF_KB_BLOCKS > 1024).
 */
#undef INTERRUPT_MODE

#ifdef INTERRUPT_MODE
#define IRQ_RX_CNT 124
#define IRQ_TX_CNT 60
#define BLOCKING_DMA 1
#else
#define IRQ_RX_CNT 0
#define IRQ_TX_CNT 0
#undef BLOCKING_DMA
#endif

/* Enable NGMP performance meassurements - may be set from compiler */
/*#define TEST_NGMP*/

#ifdef TEST_NGMP
/* Address DMA regions setup has 8 different configurations:
 *
 * CFG1 - demo coherent system without L2-cache
 *   MMU disabled
 *   L2 cache disabled
 *   Masters via CPU-bus
 *
 * CFG2 - demo coherent system with L2-cache
 *   MMU disabled
 *   L2 cache enabled
 *   Masters via CPU-bus
 *
 * CFG3 - demo coherent MMU system without L2-cache
 *   MMU enabled
 *   L2 cache disabled
 *   Masters via CPU-bus
 *
 * CFG4 - demo coherent MMU system with L2-cache
 *   MMU enabled
 *   L2 cache enabled
 *   Masters via CPU-bus
 *
 * CFG5 - demo coherent system with L2-cache, DMA not cached
 *   MMU disabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters via CPU-bus
 *
 * CFG6 - demo coherent MMU system with L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters via CPU-bus
 *
 * CFG7 - demo non-coherent MMU system without L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache disabled
 *   Masters direct into MEM-bus
 *
 * CFG8 - demo non-coherent MMU system with L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters direct into MEM-bus
 *
 *
 * NOTE that bootloader/GRMON must enable/disable L2-Cache. See l2c GRMON
 *      command.
 *
 * NOTE that bootloader/GRMON must configure stack 0x00ffffff to avoid conflict
 *      with DMA region 0x01000000 - 0x07ffffff. See stack GRMON comand
 *
 * NOTE Normal RTEMS behaviour is CFG2, or CFG8 when I/O trafic redirected
 */
int dma_cfg = 2; /* Config (override from GRMON with "wmem dma_cfg 8") */
#endif

#ifdef TEST_NGMP
/* Override from GRMON by "wmem buf_hdr_start START_ADDRESS". 0 means
 * dynamically allocated buffers */
void *buf_hdr_start = (void *)0x01100000; /* memory not used by RTEMS */
void *buf_data_start = (void *)0x01800000;
#else
void *buf_hdr_start = (void *)0; /* use malloc */
void *buf_data_start = (void *)0; /* use malloc */
#endif
void *buf_hdr_cur;
void *buf_data_cur;


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
/*#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF*//* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPW_ROUTER /* SpaceWire Router  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW2  /* SpaceWire packet driver  */

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

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "config.c"

rtems_task test_app(rtems_task_argument ignored);
rtems_task task_dma_process(rtems_task_argument devidx);

/* Set priority of GRSPW Packet driver worker task lower than application
 * task. When application task sleep the worker may do job.
 */
int grspw_work_task_priority = 100;

rtems_id tids[4];
rtems_id period;

rtems_task Init(
  rtems_task_argument ignored
)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	drvmgr_print_topo();

	/* Run SpaceWire Test application */
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '1' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tids[0]);
#ifdef BLOCKING_DMA
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '2' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_NO_FLOATING_POINT, &tids[1]);
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '3' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_NO_FLOATING_POINT, &tids[2]);
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '4' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_NO_FLOATING_POINT, &tids[3]);
#endif
	rtems_task_start(tids[0], test_app, 0);
	rtems_task_suspend( RTEMS_SELF );
}

#include <grlib/grspw_pkt.h>
#include "grspw_pkt_lib.h"
#include "spwlib.h"
#ifdef TEST_NGMP
#include "mmu_setup.h"
#endif

#undef DEBUG_TEST

#ifdef DEBUG_TEST
#define PRINT(str...) printf(str); fflush(NULL)
#define PRINT2(str...) printf(str); fflush(NULL)
#define PRINT3(str...) printf(str); fflush(NULL)
#else
#define PRINT(str...)
#define PRINT2(str...)
#define PRINT3(str...)
#endif


/* SpaceWire parmeters */
#define SPW_PROT_ID 155

/* SpaceWire packet payload (data) content layout */
struct pkt_hdr {
	unsigned char addr;
	unsigned char protid;
	unsigned char dstadr_next; /* 0 or -1, stops packet */
	unsigned char resv; /* Zero for now */
	unsigned int pkt_number;
	unsigned short data[PKT_DATA_SIZE/2];
};

/* SpaceWire Routing table entry */
struct route_entry {
	unsigned char dstadr_next;
	unsigned char dstadr[16];	/* 0 terminates array */
};

#if (SPW_LINKS_USED == 5)
struct route_entry initial_route = 
{
	.dstadr_next = 0x11,
	.dstadr = {0x1, 0x10, 0},
};

#define ROUTE_MAX 256
struct route_entry routetab[ROUTE_MAX] = 
{
	/* DST */  /* NEXT-ADR, {PATH/LOGICAL ADDRESS} */
	/* 0x00 */ {0, {0, 0}},
	/* 0x01 */ {0x10, {1+ROUTER_SPWLINK_NO, 0x1, 0}},
	/* 0x02 */ {0x12, {1+ROUTER_SPWLINK_NO+1, 0x2, 0}},
	/* 0x03 */ {0, {0}},
	/* 0x04 */ {0, {0}},
	/* 0x05 */ {0, {0}},
	/* 0x06 */ {0, {0}},
	/* 0x07 */ {0, {0}},
	/* 0x08 */ {0, {0}},
	/* 0x09 */ {0, {0}},
	/* 0x0a */ {0, {0}},
	/* 0x0b */ {0, {0}},
	/* 0x0c */ {0, {0}},
	/* 0x0d */ {0, {0}},
	/* 0x0e */ {0, {0}},
	/* 0x0f */ {0, {0}},
	/* 0x10 RASTA-IO 0 */ {0x11, {0x1, 0x10, 0}},
	/* 0x11 RASTA-IO 1 */ {0x2, {0x2, 0x11, 0}},
	/* 0x12 RASTA-IO 2 */ {0x1, {0x3, 0x12, 0}},
};

/* Map used grspw_device index to SpW device in system. This MAP
 * is created so that test opens both the first on-chip and all
 * GR-RASTA-IO AMBA ports, when 8 SpW links are implemented in
 * the on-chip router.
 *
 * Spw0 - First on-chip AMBA port  (grspw0)
 * SpW1 - Second on-chip AMBA port (grspw1)
 * SpW2 - First RASTA-IO AMBA/SpW Port (grspw4)
 * SpW3 - Second RASTA-IO AMBA/SpW Port (grspw5)
 * SpW4 - Third RASTA-IO AMBA/SpW Port (grspw6)
 *
 * grspw2 and grspw3 are not used by this example.
 */
int idx2devno[SPW_LINKS_USED] = {
	0, 1, ROUTER_AMBA_PORTS, ROUTER_AMBA_PORTS+1, ROUTER_AMBA_PORTS+2};

#else
#error SPW_LINKS_USED not supported, select 5 only
#endif

/* Table of start contents of packets, the table is indexed using the
 * Packet Number.
 */
unsigned short pkts_content_tab[PKT_CNT];

struct grspw_device {
	/* GRSPW Device layout - must be the same as 'struct grspw_dev' */
	void *dh;
	void *dma[4];
	int index;
	struct grspw_hw_sup hwsup;

	/* Test structures */
	struct grspw_config cfg;
	struct spwlib_list rx_list;
	struct spwlib_list rx_prep_list;
	struct spwlib_list tx_list;
	struct spwlib_list check_list;
	struct route_entry *routetab;
	int rxpkt_number_next;
	int txpkt_number_next;
	int rxcheck_no;
	int txcheck_no;
};
#define DEV(device) ((struct grspw_dev *)(device))

static struct grspw_device devs[32];

/* Make PKT_CNT packets per RX/TX channel */
struct spwlib_pool_cfg poolcfgs[2] = 
{
	SPWLIB_POOL_CFG(PKT_SIZE, HDR_SIZE, PKT_CNT_TOT),
	SPWLIB_POOL_CFG_END,
};

struct spwlib_poolset *poolset;

/* Allocate memory for headers and payload (data). One may wish to
 * to allocate on certain boundaries to increase cache performance.
 *
 * Pool number may be used to allocate memory 
 */
void *spw_alloc_pkt(void *data, int poolno, int hdr, int length)
{
	void *retval;

	if (hdr) {
		/* Headers are smaller so alignment is done differently */
		retval = buf_hdr_cur;
		buf_hdr_cur += length;
	} else {
		retval = buf_data_cur;
		buf_data_cur += length;
	}

	/*printf("ALLOCED: %p %d\n", retval, length);*/
	return retval;
}

void spw_alloc_reset(void *data)
{
	buf_hdr_cur = buf_hdr_start;
	buf_data_cur = buf_data_start;
}

int pkt_pool_setup(void)
{
	struct spwlib_alloc_cfg memalgo;

	printf("\n\n---- SETTING UP POOLS ----\n");

	/* Allocate memory dynamically of use user provided area */
	if (buf_hdr_start == 0) {
		buf_hdr_start = malloc(BUF_HDR_SIZE + 0x7);
		if (buf_hdr_start == NULL)
			return -1;
		buf_hdr_start = (void *)(((unsigned int)buf_hdr_start + 0x7) & ~0x7);
	}
	printf("BUFFER FOR HEADERS: %p - %p\n", buf_hdr_start, buf_hdr_start + BUF_HDR_SIZE);
	if (buf_data_start == 0) {
		buf_data_start = malloc(BUF_DATA_SIZE + 0x1f);
		if (buf_data_start == NULL)
			return -1;
		buf_data_start = (void *)(((unsigned int)buf_data_start + 0x1f) & ~0x1f);
		
	}
	printf("BUFFER FOR PAYLOAD: %p - %p\n", buf_data_start, buf_data_start + BUF_DATA_SIZE);

	memalgo.data = NULL;
	memalgo.alloc = spw_alloc_pkt;
	memalgo.reset = spw_alloc_reset;

	poolset = spwlib_poolset_alloc(&poolcfgs[0]);
	if ( poolset == NULL ) {
		printf("Failed to allocate poolset\n");
		return -1;
	}

	if ( spwlib_poolset_pkt_alloc(poolset, &memalgo) ) {
		printf("Failed to allocate pool packets\n");
		return -2;
	}

	spwlib_poolset_print(poolset);

	return 0;
}

/* Set:
 *   - packet destination address
 *   - NEXT packet destination address
 *   - Header length
 */
void pkt_init_hdr(struct spwlib_pkt *pkt, struct route_entry *route)
{
	int i;
	struct pkt_hdr *pkt_hdr = (struct pkt_hdr *)pkt->data;
	unsigned char *hdr = pkt->hdr;

	pkt_hdr->dstadr_next = route->dstadr_next;

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
}

/* Packets are used differently for different devices:
 * The Initiator:
 *   - Starts by preparing PKT_CNT packets with data according to a pattern
 *   - A receive queue is prepared with PKT_CNT packets for receiving data
 *   - All received packets are put last in the transmit queue for
 *     retransmission.
 *
 * The Responder:
 *   - Prepares PKT_CNT*2 packets for receiving
 *   - No TX packets are created
 *   - When a Packet is Received that packet is scheduled for transmission
 *     in the same order as received in.
 *
 */
int pkts_init(struct grspw_device *device, int initiator)
{
	int i, cnt, pkt_number;
	unsigned short pkt_content;
	struct spwlib_pkt *pkt;
	struct pkt_hdr *pkt_hdr;

	spwlib_list_clr(&device->rx_list);
	spwlib_list_clr(&device->rx_prep_list);
	spwlib_list_clr(&device->tx_list);
	spwlib_list_clr(&device->check_list);

	if ( initiator ) {
		/* Initiator */

		/* Allocate PKT_CNT 1k Packets for RX/TX buffers */
		cnt = spwlib_pkt_chain_take(poolset, PKT_SIZE, HDR_SIZE, PKT_CNT, &device->rx_prep_list);
		if ( cnt != PKT_CNT ) {
			printf("Failed allocating RX packets for %d\n", initiator);
			return -1;
		}
		cnt = spwlib_list_count(&device->rx_prep_list);
		if ( cnt != PKT_CNT ) {
			printf("pkts_init: ERROR1 rxlist: %d\n", cnt);
			return -2;
		}

		/* Allocate PKT_CNT Packets for TX buffers */
		cnt = spwlib_pkt_chain_take(poolset, PKT_SIZE, HDR_SIZE, PKT_CNT, &device->tx_list);
		if ( cnt != PKT_CNT ) {
			printf("Failed allocating TX packets for %d\n", initiator);
			return -1;
		}
		cnt = spwlib_list_cnt(&device->tx_list);
		if ( cnt != PKT_CNT ) {
			printf("pkts_init: ERROR2 txlist: %d\n", cnt);
		}

		/* Initialize TX Packets with some pattern */
		pkt_number = 0;
		pkt_content = 0;
		pkt = device->tx_list.head;
		while ( pkt ) {
			PRINT("pkt_init6a %d pkt=%p\n", pkt_number, pkt);
			if ( HDR_SIZE > 0 )
				memset(pkt->hdr, 0, HDR_SIZE);

			pkt_hdr = (struct pkt_hdr *)pkt->data;
			pkt_hdr->protid = SPW_PROT_ID;
			pkt_hdr->resv = 0;
			/* Set PKT-ID to index */
			pkt_hdr->pkt_number = pkt_number;
			pkts_content_tab[pkt_number] = pkt_content;

			/* Init Data */
			for (i=0; i<PKT_DATA_SIZE/2; i++) {
				pkt_hdr->data[i] = pkt_content++;
			}

			/* Init Header and DST address in data */
			pkt_init_hdr(pkt, &initial_route);

			pkt_number++;
			pkt->dlen = PKT_SIZE;
			pkt->flags |= PKT_FLAG_TR_DATA | PKT_FLAG_TR_HDR;
			pkt = pkt->next;
		}
	} else {
		/* Allocate PKT_CNT*2 Packets for RX/TX buffers */
		cnt = spwlib_pkt_chain_take(poolset, PKT_SIZE, HDR_SIZE, PKT_CNT*2, &device->rx_prep_list);
		if ( cnt != PKT_CNT*2 ) {
			printf("Failed allocating RX packets for %d\n", initiator);
			return -1;
		}
		cnt = spwlib_list_count(&device->rx_prep_list);
		if ( cnt != PKT_CNT*2 ) {
			printf("pkts_init: ERROR3 txlist: %d\n", cnt);
		}
	}

	/* Clear RX packets and set Translate address (if over PCI) */
	pkt = device->rx_prep_list.head;
	while ( pkt ) {
		if ( HDR_SIZE > 0 )
			memset(pkt->hdr, 0, HDR_SIZE);
		memset(pkt->data, 0, PKT_SIZE);
		pkt->flags |= PKT_FLAG_TR_DATA | PKT_FLAG_TR_HDR;
		pkt = pkt->next;
	}

	return 0;
}

struct grspw_config dev_configs[8] = 
{
	/*** GRSPW[0] - AMBA Port 0 on on-chip Router ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = 0x1,
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
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				/* Interrupt just before buffers BD table is empty */
				.rx_irq_en_cnt = IRQ_RX_CNT,
				.tx_irq_en_cnt = IRQ_TX_CNT,
			},
			/* The other 3 DMA Channels are unused */
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				.rx_irq_en_cnt = 0, /* Disable RX IRQ generation */
				.tx_irq_en_cnt = 0, /* Disable TX IRQ generation */
			},
			
		},
	},

	/*** GRSPW[1] - AMBA Port 1 on on-chip Router ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = 0x2,
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
		.tc_cfg = TCOPTS_EN_RX,	/* Enable TimeCode Reception */
		.tc_isr_callback = NULL,/* No TimeCode ISR */
		.tc_isr_arg = NULL,	/* No TimeCode ISR Argument */
		.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
		.chan = 
		{
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				/* Interrupt just before buffers BD table is empty */
				.rx_irq_en_cnt = IRQ_RX_CNT,
				.tx_irq_en_cnt = IRQ_TX_CNT,
			},
			/* The other 3 DMA Channels are unused */
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				.rx_irq_en_cnt = 0, /* Disable RX IRQ generation */
				.tx_irq_en_cnt = 0, /* Disable TX IRQ generation */
			},
		},
	},

	/*** GRSPW[4] - GRSPW0 on GR-RASTA-IO[0] ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = 0x10,
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
		.tc_cfg = TCOPTS_EN_RX,	/* Enable TimeCode Reception */
		.tc_isr_callback = NULL,/* No TimeCode ISR */
		.tc_isr_arg = NULL,	/* No TimeCode ISR Argument */
		.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
		.chan = 
		{
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				/* Interrupt just before buffers BD table is empty */
				.rx_irq_en_cnt = IRQ_RX_CNT,
				.tx_irq_en_cnt = IRQ_TX_CNT,
			},
			/* The other 3 DMA Channels are unused */
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				.rx_irq_en_cnt = 0, /* Disable RX IRQ generation */
				.tx_irq_en_cnt = 0, /* Disable TX IRQ generation */
			},
		},
	},

	/*** GRSPW[5] - GRSPW1 on GR-RASTA-IO[0] ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = 0x11,
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
		.tc_cfg = TCOPTS_EN_RX,	/* Enable TimeCode Reception */
		.tc_isr_callback = NULL,/* No TimeCode ISR */
		.tc_isr_arg = NULL,	/* No TimeCode ISR Argument */
		.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
		.chan = 
		{
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				/* Interrupt just before buffers BD table is empty */
				.rx_irq_en_cnt = IRQ_RX_CNT,
				.tx_irq_en_cnt = IRQ_TX_CNT,
			},
			/* The other 3 DMA Channels are unused */
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				.rx_irq_en_cnt = 0, /* Disable RX IRQ generation */
				.tx_irq_en_cnt = 0, /* Disable TX IRQ generation */
			},
		},
	},

	/*** GRSPW[6] - GRSPW2 on GR-RASTA-IO[0] ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = 0x12,
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
		.tc_cfg = TCOPTS_EN_RX,	/* Enable TimeCode Reception */
		.tc_isr_callback = NULL,/* No TimeCode ISR */
		.tc_isr_arg = NULL,	/* No TimeCode ISR Argument */
		.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
		.chan = 
		{
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				/* Interrupt just before buffers BD table is empty */
				.rx_irq_en_cnt = IRQ_RX_CNT,
				.tx_irq_en_cnt = IRQ_TX_CNT,
			},
			/* The other 3 DMA Channels are unused */
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE, /* Max 1024 bytes per packet */
				.rx_irq_en_cnt = 0, /* Disable RX IRQ generation */
				.tx_irq_en_cnt = 0, /* Disable TX IRQ generation */
			},
		},
	},
};

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

	if ( grspw_cfg_set(DEV(dev), &dev_configs[dev->index]) ) {
		grspw_close(dev);
		return -1;
	}
	printf("\n\n---- APPLICATION CONFIGURATION ----\n");
	dev->cfg = dev_configs[dev->index];
	grspw_cfg_print(&dev->hwsup, &dev->cfg);
	printf("\n\n");

	/* This will result in an error if only one port available */
	if ( dev->hwsup.nports < 2 ) {
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
	grspw_link_ctrl(dev->dh, NULL, NULL, &clkdiv);
	ctrl = LINKOPTS_ENABLE | LINKOPTS_AUTOSTART | LINKOPTS_START;
	clkdiv &= 0xff00;
	stscfg = LINKSTS_MASK;
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
		if ( dev->dma[i] )
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

struct route_entry *get_next_route(struct route_entry *tab, struct spwlib_pkt *pkt)
{
	struct pkt_hdr *hdr = pkt->data;
	int dstadr = hdr->dstadr_next;
	struct route_entry *r;

	if ( dstadr >= ROUTE_MAX ) {
		printf(" ROUTE %d does not exist\n", dstadr);
		return NULL;
	}

	r = &tab[dstadr];
	if ( r->dstadr_next == 0 ) {
		printf(" dstadr_next = 0, %d, %p, %p\n", dstadr, pkt->data, &hdr->dstadr_next);
		return NULL;
	}

	return r;
}

void pktlist_clr_flags(struct spwlib_list *lst, unsigned short flags)
{
	struct spwlib_pkt *pkt = lst->head;
	while ( pkt ) {
		pkt->flags &= ~flags;
		pkt = pkt->next;
	}
}

int debug_test(int got, int exp, void *adr)
{
	static int i = 0;
	asm volatile("ta 0x01\n\t"::);
	return i++;
}

int pktlist_check_rx(struct grspw_device *device, struct spwlib_list *lst, int content)
{
	struct spwlib_pkt *pkt = lst->head;
	struct pkt_hdr *pkt_hdr;
	unsigned short exp, got;
	int i;

	device->rxcheck_no++;
	while ( pkt ) {
		/* Check Reception HW-Flags */
		if ( (pkt->flags & (RXPKT_FLAG_TRUNK|RXPKT_FLAG_EEOP|RXPKT_FLAG_RX))
		     != RXPKT_FLAG_RX ) {
			printf("SpW%d PKT RX ERROR: 0x%08x\n", device->index,
				pkt->flags);
			return -1;
		}

		/* Check Packet number (Sequence check) */
		pkt_hdr = (struct pkt_hdr *)pkt->data;
		if ( pkt_hdr->pkt_number != device->rxpkt_number_next ) {
			printf("SpW%d PKT RX SEQ ERROR: exp %d, got %d\n",
				device->index,
				device->rxpkt_number_next, pkt_hdr->pkt_number);
			return -2;
		}
		/* Check Packet Content */
		if ( content ) {
			exp = pkts_content_tab[pkt_hdr->pkt_number];
			for (i=0; i<PKT_DATA_SIZE/2; i++) {
				got = pkt_hdr->data[i];
				if ( exp != got ) {
					debug_test(got, exp, &pkt_hdr->data[i]);
					printf(" BAD RX DATA in PACKET: got 0x%04x expected 0x%04x (%p)\n",
						got, exp, &pkt_hdr->data[i]);
					return -1;
				}
				exp++;
			}
		}
		/* Prepare next Packet content */
		device->rxpkt_number_next++;
		if (device->rxpkt_number_next >= PKT_CNT)
			device->rxpkt_number_next = 0;

		pkt = pkt->next;
	}
	return 0;
}

int pktlist_check_tx(struct grspw_device *device, struct spwlib_list *lst, int content)
{
	struct spwlib_pkt *pkt = lst->head;
	struct pkt_hdr *pkt_hdr;
	unsigned short exp, got;
	int i;

	device->txcheck_no++;
	while ( pkt ) {
		/* Check Packet number (Sequence check) */
		pkt_hdr = (struct pkt_hdr *)pkt->data;
		if ( pkt_hdr->pkt_number != device->txpkt_number_next ) {
			printf("SpW%d PKT TX SEQ ERROR: exp %d, got %d\n",
				device->index,
				device->txpkt_number_next, pkt_hdr->pkt_number);
			return -2;
		}

		/* Check Packet Content */
		if ( content ) {
			exp = pkts_content_tab[pkt_hdr->pkt_number];
			for (i=0; i<PKT_DATA_SIZE/2; i++) {
				got = pkt_hdr->data[i];
				if ( exp != got ) {
					debug_test(got, exp, &pkt_hdr->data[i]);
					printf(" BAD TX DATA in PACKET: got 0x%04x expected 0x%04x (%p)\n",
						got, exp, &pkt_hdr->data[i]);
					return -1;
				}
				exp++;
			}
		}

		/* Prepare next Packet content */
		device->txpkt_number_next++;
		if (device->txpkt_number_next >= PKT_CNT)
			device->txpkt_number_next = 0;

		pkt = pkt->next;
	}
	return 0;
}

/* Process:
 *   1. Take transmitted packets and prepare for recepion
 *   1b. Put SEND->SCHED packet in driver unless STOP=1
 *   2. Get received packets
 *   2b. If STOP=1, Put Packets to check list, then return
 *   3. Prepare received packets for transmission by setting new header.
 *   4. Put prepared packets last in tx-queue
 *   5. Schedule packets in tx-queue
 */
int grspw_process(struct grspw_device *dev, int options, int stop)
{
	struct spwlib_list lst;
	struct spwlib_pkt *pkt;
	struct route_entry *route;
	int result, count;

	/* 1. */
	spwlib_list_clr(&lst);
	count = -1;
	result = grspw_dma_tx_reclaim(dev->dma[0], stop << 1,
					(struct grspw_list *)&lst, &count);
	if ( result < 0 ) {
		printf("Reclaim failed: %d (%d)\n", result, dev->index);
		return -1;
	}
	if (count > 0)
		lst.count = count;
	if ( spwlib_list_is_empty(&lst) == 0 ) {
		/* Check TX Flags... optimized, done in grspw_tx_reclaim() for
		 * us.
		 */

		/* Clear TX Packet Flags, but let translation persist */
		pktlist_clr_flags(&lst, ~(unsigned short)PKT_FLAG_MASK);

		spwlib_list_append_list(&dev->rx_prep_list, &lst);
	}
	PRINT("1E\n");

	if ( spwlib_list_is_empty(&dev->rx_prep_list) == 0 ) {
		result = grspw_dma_rx_prepare(dev->dma[0], 0,
				(struct grspw_list *)&dev->rx_prep_list,
				spwlib_list_cnt(&dev->rx_prep_list));
		if ( result ) {
			printf("Prepare failed: %d (%d)\n", result, dev->index);
			return -5;
		}
		spwlib_list_clr(&dev->rx_prep_list);
	}

	/* 2. */
	PRINT("2A\n");
	spwlib_list_clr(&lst);
	count = -1;
	result = grspw_dma_rx_recv(dev->dma[0], 0, (struct grspw_list *)&lst,
					&count);
	if ( result ) {
		printf("Receive failed: %d (%d)\n", result, dev->index);
		return -10;
	}
	if (count > 0)
		lst.count = count;

	/* Check Packet Sequence Number and HW-Transmission Flags and optionally
	 * the packet contents
	 */
	if ( spwlib_list_is_empty(&lst) == 0 ) {
#ifdef SKIP_CHECK_FOR_EVERY_RECEIVED_PACKET
		if ( pktlist_check_rx(dev, &lst, 0) ) {
#else
		if ( pktlist_check_rx(dev, &lst, 1) ) {
#endif
			printf(" ERROR In Packet\n");
			return -100;
		}
	}

	if ( stop ) {
		if ( spwlib_list_is_empty(&lst) == 0 ) {
			spwlib_list_append_list(&dev->check_list, &lst);
		}
		return 0;
	}
	PRINT("2B\n");
	if ( spwlib_list_is_empty(&lst) == 0 ) {
		/* 3. */
		PRINT("3\n");
		pkt = lst.head;
		while ( pkt ) {
			/* Clear RX Flags but let translation flags persist */
			pkt->flags &= (unsigned short)PKT_FLAG_MASK;

			/* Find route entry */
			if ( (route=get_next_route(dev->routetab, pkt)) == NULL ) {
				printf("Failed to get route for packet\n");
				return -15;
			}

			/* Init Packet header accordingly */
			pkt_init_hdr(pkt, route);
			pkt = pkt->next;
		}
		spwlib_list_append_list(&dev->rx_list, &lst);

		/* 4. */
		PRINT("4\n");
		spwlib_list_append_list(&dev->tx_list, &dev->rx_list);
		spwlib_list_clr(&dev->rx_list);
	}

	/* 5. */
	PRINT("5A\n");
	if ( (options & 2) && spwlib_list_is_empty(&dev->tx_list) == 0 ) {
		PRINT("5B\n");

#ifndef SKIP_CHECK_FOR_EVERY_TRANSMITTED_PACKET
		/* Check Packet Sequence Number and HW-Transmission Flags */
		if ( pktlist_check_tx(dev, &device->tx_list, 0) ) {
			printf(" ERROR In TX Packet sequence\n");
			return -17;
		}
#endif
		result = grspw_dma_tx_send(dev->dma[0], 0,
					(struct grspw_list *)&dev->tx_list,
					dev->tx_list.count);
		if ( result < 0 ) {
			printf("Send failed: %d (%d)\n", result, dev->index);
			return -20;
		}
		spwlib_list_clr(&dev->tx_list);
	}

	return 0;
}

int stop_tasks = 0;
#ifdef BLOCKING_DMA
rtems_task task_dma_process(rtems_task_argument devidx)
{
	struct grspw_device *dev = &devs[devidx];
	int rc, recv, shutdown;

	while (stop_tasks < 2) {

		/* Wait for PKT_CNT/4 to arrive before generating
		 * transmit packets
		 */
		if (stop_tasks == 0) {
			rc = grspw_dma_rx_wait(dev->dma[0], PKT_CNT/4, 0, 0xfffffff, 40);
			if (rc == 2) {
				/* Timeout */
			} else if (rc != 0) {
				/* Error */
				printf("dma_rx_wait(%d) failed: %d\n", dev->index, rc);
			} else {
				grspw_dma_rx_count(dev->dma[0], 0, 0, &recv);
				if (recv < PKT_CNT/4) {
					printf("dma_rx_wait returned early: %d\n", recv);
					exit(0);
				}
			}
		}

		shutdown = grspw_process(dev, 3, 0);
		if (shutdown) {
			printf("Shutting down dev%d: \n", dev->index);
			break;
		}

#if 0
		/* Wait until all but 64 packets been transmitted. The
		 * descriptor table can house 64 packets. Note also depending
		 * on how IRQs are enabled (cfg.tx_irq_en_cnt), there will be
		 * some unhandled TX descriptors around until we call
		 * grspw_dma_tx_reclaim() or grspw_dma_tx_send().
		 */
 		if (stop_tasks == 0) {
			int sent;

			rc = grspw_dma_tx_wait(dev->dma[0], 0x0fffffff, 0, PKT_CNT/8-64, 40);
			if (rc == 2) {
				/* timeout */
			} else if (rc != 0) {
				/* Error */
				printf("dma_tx_wait(%d) failed: %d\n", dev->index, rc);
			} else {
				grspw_dma_tx_count(dev->dma[0], 0, 0, &sent);
				if (sent < (PKT_CNT/8 - 64)) {
					printf("dma_tx_wait returned early: %d\n", sent);
					exit(0);
				}
			}
		}
#endif
		rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
	}
	rtems_task_delete(RTEMS_SELF);
}
#endif

extern int router_setup_custom(void);

rtems_task test_app(rtems_task_argument ignored)
{
	int i, result;
	int shutdown, stop, devno, options, tot;
	struct timespec t0, t1, t2;
	double time;
	int initiator, loopcnt, do_stop, cnt;
	struct grspw_stats stats;
	int rx_ready, rx_sched, rx_recv, rx_hwcnt;
	int tx_send, tx_sched, tx_sent, tx_hwcnt;
#ifdef INTERRUPT_MODE
	int status;
#endif

	printf("\nTest built for SpW-router with %d SpW Ports and at least %d AMBA ports.\n"
	       "Note that the SpW-links will not be used.\n"
	       "There are %d GRSPW cores in the system.\n"
	       "System clock: %lu us / tick\n",
	       ROUTER_SPWLINK_NO, SPW_LINKS_USED, grspw_dev_count(),
	       rtems_configuration_get_microseconds_per_tick());
	printf("\n\n");

#ifdef TEST_NGMP
	switch (dma_cfg) {
	case 1:
		address_region_setup(MMU_DISABLE, L2C_DISABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	default:
	case 2:
		address_region_setup(MMU_DISABLE, L2C_ENABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 3:
		address_region_setup(MMU_ENABLE, L2C_DISABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 4:
		address_region_setup(MMU_ENABLE, L2C_ENABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 5:
		address_region_setup(MMU_DISABLE, L2C_ENABLE | L2C_MTRR_ENABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 6:
		address_region_setup(MMU_ENABLE, L2C_ENABLE | L2C_MTRR_ENABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 7:
		address_region_setup(MMU_ENABLE, L2C_DISABLE | L2C_MTRR_DISABLE, IOMMU_BUS_MEM|IOMMU_PREFETCH_DISABLE);
		break;
	case 8:
		address_region_setup(MMU_ENABLE, L2C_ENABLE | L2C_MTRR_ENABLE, IOMMU_BUS_MEM|IOMMU_PREFETCH_DISABLE);
		break;
	}
#endif

	printf("Setting up SpaceWire router\n");
	if (router_setup_custom()) {
		printf("Failed router initialization, aborting\n");
		return;
	}

	memset(devs, 0, sizeof(devs));
	for (i=0; i<SPW_LINKS_USED; i++) {
		if ( dev_init(i) ) {
			printf("Failed to initialize GRSPW%d\n", i);
			return;
		}
		fflush(NULL);
	}

	if (pkt_pool_setup()) {
		printf("Packet setup failed\n");
		return;
	}
	printf("PKT_CNT:             %d\n", PKT_CNT);
	printf("PKT_CNT_TOT:         %d\n", PKT_CNT_TOT);
	printf("PKT_SIZE:            %d\n", PKT_SIZE);
	printf("SPW_LINKS_USED:      %d\n", SPW_LINKS_USED);
	printf("DMA_CHANS_PER_LINK:  %d\n", SPW_DMA_CHANS_PER_LINK_USED);
	printf("DIRECTIONS:          %d\n", SPW_DIRECTIONS_USED);

	/* Set up the simplest case:
	 *   - Two nodes
	 *   - initiator (SPW0) with address 1
	 *   - responder (SPW1) with address 2
	 */
	for (i=0; i<SPW_LINKS_USED; i++) {
		if ( i == SPW_INITIATOR )
			initiator = 1;
		else
			initiator = 0;

		/* Initialize Packets for SPW[N] */
		printf("Initializing Packets for SpW%d. (Initiator=%s)\n", i,
			initiator ? "YES" : "NO");
		result = pkts_init(&devs[i], initiator);
		if ( result < 0 ) {
			printf("Packet initialization failed SpW%d: %d\n", 
				i, result);
			return;
		}

		/* Assign route table for SPW devices. All use the same "routing
		 * table".
		 */
		devs[i].routetab = &routetab[0];

		/* Init Packet Check */
		devs[i].rxpkt_number_next = 0;
		devs[i].txpkt_number_next = 0;
		devs[i].rxcheck_no = 0;
		devs[i].txcheck_no = 0;
	}

	printf("\n\n");
	for (i=0; i<SPW_LINKS_USED; i++) {
		printf("SpW%d Number of packets:\n", i);
		cnt = spwlib_list_cnt(&devs[i].rx_list);
		printf( "      rx_list: %d\n", cnt);
		cnt = spwlib_list_cnt(&devs[i].rx_prep_list);
		printf( "      rx_prep_list: %d\n", cnt);
		cnt = spwlib_list_cnt(&devs[i].tx_list);
		printf( "      tx_list: %d\n", cnt);
		cnt = spwlib_list_cnt(&devs[i].check_list);
		printf( "      check_list: %d\n", cnt);		
	}

	printf("\n\nStarting SpW DMA channels\n");
	for ( i=0; i<SPW_LINKS_USED; i++) {
		printf("Starting SpW%d: ", i);
		fflush(NULL);
		if ( grspw_start(DEV(&devs[i])) ) {
			printf("Failed to initialize SpW%d\n", i);
			return;
		}
		printf("Started Successfully\n");
	}

	printf("Starting Packet processing loop, will take approx %d secs\n",
		TEST_LENGTH_SEC);

	/* Packet processing loop */
	options = 1; /* Only allow RX prepare first time */
	devno = -1;
	shutdown = 0;
	loopcnt = 0;
	stop = 0;
	stop_tasks = 0;

#ifdef INTERRUPT_MODE
	/* Create period */
	status = rtems_rate_monotonic_create(rtems_build_name('P','E','R','1'), &period);
	if (status != RTEMS_SUCCESSFUL) {
		printf("Monotonic rate period cration failed: %d\n", status);
		exit(0);
	}
	rtems_rate_monotonic_reset_all_statistics();
#endif

	rtems_clock_get_uptime(&t0);
	while ( shutdown == 0 ) {
		/* Check is 10s has gone now and then... */
		if ( (stop == 0) && ((loopcnt & 0x7) == 0x7) ) {
			rtems_clock_get_uptime(&t1);
			if ( t1.tv_sec > (t0.tv_sec + TEST_LENGTH_SEC) ) {
				stop = 1;
				stop_tasks = 1;
				/* Calculate when to stop */
				t1.tv_sec += TEST_END_WAIT_SEC;
			}
		} else if ( stop && ((loopcnt & 0x7) == 0x7) ) {
			/* Check if INITIATOR:
			 *   - TX_SCHEDULE Count is Zero (nothing more to send)
			 *   - Number of Received == Number of Transmitted
			 */
			if ( stop == 1 ) {
				grspw_dma_tx_count(devs[SPW_INITIATOR].dma[0], NULL,
							&tx_sched, NULL, NULL);
				if ( tx_sched == 0 )
					stop = 2;
			}
			if ( stop == 2 ) {
				/* Stop if all packets we sent has been collected
				 * back at the receiver.
				 */
				grspw_stats_get(DEV(&devs[SPW_INITIATOR]), &stats);
				if ( stats.chan[SPW_INITIATOR].rx_pkts ==
				     stats.chan[SPW_INITIATOR].tx_pkts )
					stop = 3;
			}
			if (stop > 2) {
				/* Stop 3 loops after the conditions above
				 * are true.
				 */
				break;
			}

			/* Stop Collect Process by time, this is to avoid
			 * endless hang if not all packets are received
			 */
			rtems_clock_get_uptime(&t2);

			if ( (t2.tv_sec > t1.tv_sec) ||
			     ((t2.tv_sec == t1.tv_sec) && (t2.tv_nsec > t1.tv_nsec)))
				break;
		}

		for ( i=0; i<SPW_LINKS_USED; i++) {
			/*printf("PROCESS%d %d, %d\n", j, stop, i);*/

#ifdef BLOCKING_DMA
			if (i > 0) {
				if (loopcnt == 1)
					rtems_task_start(tids[i], task_dma_process, i);
				else if (loopcnt > 1)
					continue;
			}
#endif

			/* Only Initiator collect the packets in the end. */
			do_stop = 0;
			if ( (i == SPW_INITIATOR) && stop )
				do_stop = 1;

			shutdown = grspw_process(&devs[i], options, do_stop);
			if ( shutdown ) {
				printf("Shutting down: \n");
				devno = i;
				break;
			}
		}
#ifdef INTERRUPT_MODE
		if (stop == 0)
			status = rtems_rate_monotonic_period(period, 20);
		else
			rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
#endif
		options = 3;
		loopcnt++;
	}
	stop_tasks = 2;

	/* Get Stats of Channel 0, this is to determine how many packets have
	 * been received.
	 */
	grspw_stats_get(DEV(&devs[SPW_INITIATOR]), &stats);
	tot = stats.chan[SPW_INITIATOR].rx_pkts;

	/* Stop time */
	rtems_clock_get_uptime(&t1);

	/* Let the other tasks shutdown */
	rtems_task_wake_after(2);

	printf("\n\nShutting down: %d (SpW %d), loops=%d, stop=%d\n\n", shutdown, devno, loopcnt, stop);
	for (i=0; i<SPW_LINKS_USED; i++) {
		printf("\n\n--- SpW%d Device ---\n", i);
		printf(" RX List count: %d\n", spwlib_list_cnt(&devs[i].rx_list));
		printf(" RX PREP List count: %d\n", spwlib_list_cnt(&devs[i].rx_prep_list));
		printf(" TX List count: %d\n", spwlib_list_cnt(&devs[i].tx_list));
		printf(" Check List count: %d\n", spwlib_list_cnt(&devs[i].check_list));
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
	printf(" t0.tv_sec:     %u\n", (unsigned int)t0.tv_sec);
	printf(" t0.tv_nsec:    %ld\n", t0.tv_nsec);
	printf(" t1.tv_sec:     %u\n", (unsigned int)t1.tv_sec);
	printf(" t1.tv_nsec:    %ld\n", t1.tv_nsec);
	time = (float)t1.tv_sec + (float)t1.tv_nsec/1000000000;
	time = time - (float)t0.tv_sec - (float)t0.tv_nsec/1000000000;
	printf(" Time:          %f seconds\n", time);
	printf(" Packets sent:  %d packets\n", tot);
	printf(" Packet size:   %d bytes in total\n", PKT_SIZE);
	printf(" Throughput:    %f packets/sec\n", ((float)tot)/time);
	printf(" Throughput:    %f bytes/sec\n", ((((float)tot)*PKT_SIZE))/time);
	printf(" Throughput:    %f bits/sec\n", 8 * ((((float)tot)*PKT_SIZE)/time));
	printf(" All DMAs:      %f bits/sec\n", 8 * ((((float)tot)*PKT_SIZE)/time) * SPW_DIRECTIONS_USED);

	/* Check Content and Packet number of resulting Received Packets
	 * Since we stopped transmitting abrupt, we look at the sequence number
	 * of first packet to init packet checker.
	 */
	if ( spwlib_list_is_empty(&devs[SPW_INITIATOR].check_list) == 0 ) {
		struct pkt_hdr *pkt_hdr = (struct pkt_hdr *)
			devs[SPW_INITIATOR].check_list.head->data;
		devs[SPW_INITIATOR].rxpkt_number_next = pkt_hdr->pkt_number;

		result = pktlist_check_rx(&devs[SPW_INITIATOR],
					&devs[SPW_INITIATOR].check_list,
					1);
		if ( result ) {
			printf("###PACKET CONTENT/PKT-SEQUENCE CHECK FAILED: %d\n", result);
		} else {
			printf("PACKET CONTENT/PKT-SEQUENCE CHECK SUCCESSFUL\n");
		}
	} else {
		printf("###NO RECEIVED PACKETS IN CHECKLIST: PKT CHECK FAILED\n");
	}

	for ( i=0; i<SPW_LINKS_USED; i++) {
		dev_cleanup(i);
	}
	sleep(1);

	printf("\n\n");
#ifdef INTERRUPT_MODE
	rtems_rate_monotonic_report_statistics();
	printf("\n\n");
#endif

	exit(0);

	return;
}
