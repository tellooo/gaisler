/*
 * RTEMS SpaceWire packet library demonstration application modified for
 * NGMP research project - demonstrate cache/DMA coherence options.
 *
 * The application requires the GRSPW Router. The application has been designed
 * to be able to run on several boards each with one or more SpaceWire nodes
 * on a SpaceWire network. In that case the routing table can easily be
 * updated to make packets travel off-chip onto a SpaceWire network.
 *
 * In the default configuration the data path of a SpaceWire packet is as
 * follows:
 *
 *  AMBAPort0 TX  ->  AMBAPort1 RX
 *  AMBAPort1 TX  ->  AMBAPort2 RX
 *  AMBAPort2 TX  ->  AMBAPort3 RX
 *  AMBAPort3 TX  ->  AMBAPort0 RX
 *
 * This means that every packet will exercise 8 DMA operation channels every
 * time the packet travels one round-trip. The number of round trips are
 * counted and performance figures can be calculated ontop of that.
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

/* SpW links are connected SpW0 <-> SpW1 without SpW router being used.
 * Configuration is for UT699, GR712RC, UT699e, UT700 etc.
 */
/*#define DUAL_LOOPBACK*/

/* SpW links on SpaceWire Router determine which PATH addess the AMBA
 * targets has
 */
#define ROUTER_SPWLINK_NO 8

/* Number of SpaceWire Links to share the packet buffers with, each
 * SpaceWire Link have 2 Packet Queues per DMA Unit.
 */
#ifdef DUAL_LOOPBACK
#define SPW_LINKS_USED 2  /* do not configure when SpW0 & SpW1 in loopback */
#else
#define SPW_LINKS_USED 4  /* Configure this */
#endif
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
#define TEST_LENGTH_SEC (5)
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

/* GRSPW SpaceWire run clock divisor */
#define GRSPW_RUN_CLKDIV 0

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
 * At least, this sample application has configured IRQs to happen every 120 or
 * 56 descriptor - minimum four descriptors remains.
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
#define INTERRUPT_MODE

#ifdef INTERRUPT_MODE

 #if (PKT_CNT > 256)
  #define IRQ_RX_CNT 120		/* Interrupt when only 8 BDs left */
  #define IRQ_TX_CNT 0			/* No TX IRQs. We only wait for RX. */
 #else
  #define IRQ_RX_CNT 8			/* Interrupt on every 8:th RX BD */
  #define IRQ_TX_CNT 0			/* No TX IRQs. We only wait for RX. */
 #endif
 #define BLOCK_PKTS 16			/* block until N packets are available*/
 #define BLOCK_TIMEOUT 2		/* .. but timeout after I ticks. */
 #define BLOCKING_DMA 1
 #define SPY_PROFILE 0
#else
 #define IRQ_RX_CNT 0
 #define IRQ_TX_CNT 0
 #undef BLOCKING_DMA
 #undef SPY_PROFILE
 #undef BLOCK_PKTS
 #undef BLOCK_TIMEOUT 
#endif

/* Enable CPU usage statistics on RTEMS */
#define DEMO_PROFILING

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
void *buf_hdr_start_alloced;
void *buf_data_start_alloced;
void *buf_hdr_cur;
void *buf_data_cur;

#define TASK_PRIO 10
int spacewire_router_demo(int run_for_secs);
void test_app(void);
volatile int stop_tasks = 0;

/* Application benchmarking/debugging parameters */
char *tnames[4] = {"SpW_DMA0", "SpW_DMA1", "SpW_DMA2", "SpW_DMA3"};
int twaits[4];
int twaits_stopping[4];
int ttimeouts[4];
int ttimeouts_stopping[4];
int tprocesses[4];
int tprocesses_stopping[4];
int isrfilter_calls[4];
int isrfilter_txdma_calls[4];
int isrfilter_rxdma_calls[4];

int spacewire_length_secs;
int spacewire_router_demo_exitcode;

#ifdef __rtems__
/* RTEMS */
#include "test_rtems.c"
#include <grlib/grspw_pkt.h>
#define OS_DELAY_TICKS(_ticks) rtems_task_wake_after(_ticks)
#else
/* VxWorks */
#include "test_vxworks.c"
#include <hwif/io/grlibGrspwPkt.h>
#define OS_DELAY_TICKS(_ticks) taskDelay(_ticks)
#endif
#include <string.h>
#include <inttypes.h>

/*
 * Set this to demonstrate different work-task/messageQ set ups:
 *  0 - default behaviour:
 *      One single work-task/msgQ to service all GRSPW devices.
 *
 *  1 - Custom work-task setup 1:
 *      One work-task & msgQ per GRSPW device. User controls work-task priority
 *      per GRSPW device. In this configuration one should disable GRSPW driver
 *      to create work-task/msgQ by setting GRSPWPKT_WORKTASK_PRIO=-1.
 *
 *  2 - Custom work-task setup 2:
 *      Default a common work-task and MsgQ to handle DMA Errors (dma_stop) and
 *      Link errors (dma_stop on all channels) on all GRSPW devices.
 *      Handle custom DMA TX/RX operation interrupts by signalling directly to
 *      an application specific binary semaphore, one semaphore for RX and one
 *      for TX per GRSPW device.
 */
int spacewire_worktask_setup_demo = 1;

int spacewire_router_demo(int run_for_secs)
{
	if (run_for_secs <= 0)
		spacewire_length_secs = TEST_LENGTH_SEC;
	else
		spacewire_length_secs = run_for_secs;

#if defined(INTERRUPT_MODE) && defined(GRSPWPKT_WORKTASK_PRIO)
	if (GRSPWPKT_WORKTASK_PRIO < 0 && spacewire_worktask_setup_demo == 0) {
		printf(" ### Aborting. Bad configuration to disable driver's\n");
		printf("     work-task and rely on driver to handle IRQ.\n");
		return -1;
	}
#endif
	/* Run SpaceWire Test application */
	spacewire_router_demo_exitcode = -1;

	buf_hdr_start_alloced = NULL;
	buf_data_start_alloced = NULL;

	tids[1] = tids[2] = tids[3] = 0;
	memset(tids, 0, sizeof(tids));
	memset(twaits, 0, sizeof(twaits));
	memset(twaits_stopping, 0, sizeof(twaits_stopping));
	memset(ttimeouts, 0, sizeof(ttimeouts));
	memset(ttimeouts_stopping, 0, sizeof(ttimeouts_stopping));
	memset(tprocesses, 0, sizeof(tprocesses));
	memset(tprocesses_stopping, 0, sizeof(tprocesses_stopping));
	memset(isrfilter_calls, 0, sizeof(isrfilter_calls));
	memset(isrfilter_txdma_calls, 0, sizeof(isrfilter_txdma_calls));
	memset(isrfilter_rxdma_calls, 0, sizeof(isrfilter_rxdma_calls));
	
	grspw_demo_os_task_setup();

	/* Wait to by unblocked by test application */
#if __rtems__
	rtems_task_suspend( RTEMS_SELF );
#else
	taskSuspend(0);
#endif

	return spacewire_router_demo_exitcode;
}

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


#if defined(DUAL_LOOPBACK) && (SPW_LINKS_USED == 2)
/* Dual SpW connectors, SpW0 <-> SpW1 - tested on a GR712RC */
struct route_entry initial_route = 
{
	.dstadr_next = 0x1,
	.dstadr = {0x2, 0},
};

#define ROUTE_MAX 256
struct route_entry routetab[ROUTE_MAX] = 
{
	{0, {0, 0}},
	{0x2, {0x1, 0}},
	{0x1, {0x2, 0}},
};

#elif (SPW_LINKS_USED == 2)
/* Router is used with two SpaceWires. This setup can be used on 
 * a LEON4-N2X or GR740 (with internal router). The GR740 has 8 SpW ports and is
 * when AMBA port N has SpW path address 9+N.
 *
 * On systems without integrated routers when an "external" router is connected. In the latter case it is very
 * likely that the node/path addresses must be specifically set up to match the
 * SpW network set up.
 */
struct route_entry initial_route = 
{
	.dstadr_next = 0x1,
	.dstadr = {1+ROUTER_SPWLINK_NO+1, 0x2, 0},
};

#define ROUTE_MAX 256
struct route_entry routetab[ROUTE_MAX] = 
{
	{0, {0, 0}},
	{0x2, {1+ROUTER_SPWLINK_NO, 0x1, 0}},
	{0x1, {1+ROUTER_SPWLINK_NO+1, 0x2, 0}},
};
#elif (SPW_LINKS_USED == 4)
/* Router is used with four SpaceWires. This setup can be used on 
 * a LEON4-N2X or GR740 (with internal router). The GR740 has 8 SpW ports and is
 * when AMBA port N has SpW path address 9+N.
 *
 * On systems without integrated routers when an "external" router is connected. In the latter case it is very
 * likely that the node/path addresses must be specifically set up to match the
 * SpW network set up.
 */
struct route_entry initial_route = 
{
	.dstadr_next = 0x3,
	.dstadr = {1+ROUTER_SPWLINK_NO+1, 0x2, 0},
};

#define ROUTE_MAX 256
struct route_entry routetab[ROUTE_MAX] = 
{
	/* DST */  /* NEXT-ADR, {PATH/LOGICAL ADDRESS} */
	/* 0x00 */ {0, {0, 0}},
	/* 0x01 */ {0x2, {1+ROUTER_SPWLINK_NO, 0x1, 0}},
	/* 0x02 */ {0x3, {1+ROUTER_SPWLINK_NO+1, 0x2, 0}},
	/* 0x03 */ {0x4, {1+ROUTER_SPWLINK_NO+2, 0x3, 0}},
	/* 0x04 */ {0x1, {1+ROUTER_SPWLINK_NO+3, 0x4, 0}},
};
#else
#error SPW_LINKS_USED not supported, select 2 or 4
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

	OS_MSGQ_TYPE msgQ; /* optional work-task message queue */
	OS_SEM_TYPE rxDmaSem;
	OS_SEM_TYPE txDmaSem;
};
#define DEV(device) ((struct grspw_dev *)(device))

static struct grspw_device devices[32];

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
		buf_hdr_start_alloced = malloc(BUF_HDR_SIZE + 0x7);
		if (buf_hdr_start_alloced == NULL)
			return -1;
		buf_hdr_start = (void *)(((unsigned int)buf_hdr_start_alloced + 0x7) & ~0x7);
	}
	printf("BUFFER FOR HEADERS: %p - %p\n", buf_hdr_start, buf_hdr_start + BUF_HDR_SIZE);
	if (buf_data_start == 0) {
		buf_data_start_alloced = malloc(BUF_DATA_SIZE + 0x1f);
		if (buf_data_start_alloced == NULL)
			return -1;
		buf_data_start = (void *)(((unsigned int)buf_data_start_alloced + 0x1f) & ~0x1f);
		
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

void pkt_pool_free(void)
{
#ifdef TEST_NGMP
	/* buffers statically assigned, do nothing.. */
#else
	/* free memory */
	if (buf_hdr_start_alloced) {
		free(buf_hdr_start_alloced);
		buf_hdr_start_alloced = NULL;
		buf_hdr_start = 0;
	}
	if (buf_data_start_alloced) {
		free(buf_data_start_alloced);
		buf_data_start_alloced = NULL;
		buf_data_start = 0;
	}
#endif
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

	/* Clear RX packets */
	pkt = device->rx_prep_list.head;
	while ( pkt ) {
		if ( HDR_SIZE > 0 )
			memset(pkt->hdr, 0, HDR_SIZE);
		memset(pkt->data, 0, PKT_SIZE);
		pkt = pkt->next;
	}

	return 0;
}

struct grspw_config dev_configs[8] = 
{
	/*** GRSPW[0] ***/
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

	/*** GRSPW[1] ***/
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

	/*** GRSPW[2] ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = 0x3,
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

	/*** GRSPW[3] ***/
	{
		.adrcfg =
		{
			.promiscuous = 0,
			.def_addr = 0x4,
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

/* Demonstrate one work task per GRSPW device */
int dev_worktask_setup1(struct grspw_device *dev)
{
	struct grspw_work_config wc;
	int tid;

#ifdef GRSPWPKT_WORKTASK_PRIO
	if (GRSPWPKT_WORKTASK_PRIO >= 0) {
		puts("### This setup is not recommended. It will result in a");
		puts("    dangeling work-task & msgQ created by the driver.");
		puts("    It can be avoided by GRSPWPKT_WORKTASK_PRIO=-1.");
		puts("");
	}
#endif

	/* Create one Message Queue per work task with max 64 number of
	 * messages in the queue.
	 * Work task: default stack size. Set priority to lower than application
	 * tasks (NOTE: THIS MIGHT NOT BE OPTIMAL AND IS APPLICATION SPECIFIC).
	 */
	dev->msgQ = OS_MSGQ_NULL;
	tid = grspw_work_spawn(TASK_PRIO+1, 0, &dev->msgQ, 64);
	if (tid == OS_GRSPW_WORK_SPAWN_ERROR)
		return -1;

#ifdef SETUP_CPU_AFFINITY_WT
	grspw_demo_os_set_affinity_wt(tid, dev->index);
#endif

	/* build a GRSPW work configuration */
	wc.msgisr = (grspw_msgqisr_t)OS_MSGQ_SEND;
	wc.msgisr_arg = (void *)dev->msgQ;

	/* Assign msgQ and work-task to GRSPW device */
	grspw_work_cfg(dev->dh, &wc);
	return 0;
}

int grspw_quit_event_detected = 0;
void grspw_work_event(enum grspw_worktask_ev ev, unsigned int msg)
{
	/* Called from work-task */
	if (ev == WORKTASK_EV_QUIT) {
		grspw_quit_event_detected++;
	} else if (spacewire_worktask_setup_demo == 2) {
		struct grspw_device *dev =
			&devices[(msg & WORK_CORE_MASK) >> WORK_CORE_BIT];
		if (ev == WORKTASK_EV_DMA_STOP) {
			/* A single DMA channel stopped */
			OS_SEM_FLUSH(dev->txDmaSem);
			OS_SEM_FLUSH(dev->rxDmaSem);
		} else if (ev == WORKTASK_EV_SHUTDOWN) {
			/* All channels DMA stopped.. example only deals with
			 * one channel anyway..
			 */
			OS_SEM_FLUSH(dev->txDmaSem);
			OS_SEM_FLUSH(dev->rxDmaSem);
		}
	}
}

/* one work-task and one associated message Q, we delete them both
 * but to ensure ISR does not use a deleted MsgQ it is unassigned first.
 */
void dev_worktask_close1(struct grspw_device *dev)
{
	/* unassign msgQ first */
	struct grspw_work_config wc = {NULL, NULL};
	grspw_work_cfg(dev, &wc);

	/* Send "delete msgQ and exit Work-task message" to work-task */
	grspw_quit_event_detected = 0;
	grspw_work_free(dev->msgQ, 1);
	/* Wait for work-task to exit */
	while (grspw_quit_event_detected == 0)
		OS_DELAY_TICKS(1);
	dev->msgQ = OS_MSGQ_NULL;
}

/* Demonstrate custom RX/TX DMA handling, use work-task */

int wt_setup2_filter(void *data, unsigned int *buf, unsigned int n, int t, int p)
{
	unsigned int msg = *buf;
	struct grspw_device *dev = &devices[(msg&WORK_CORE_MASK) >> WORK_CORE_BIT];
	isrfilter_calls[dev->index]++;

	/* A simplification.. only support one DMA channel */
	if (msg & WORK_DMA_TX_MASK) {
		*buf &= ~WORK_DMA_TX_MASK; /* filter event to work-task */
		OS_SEM_GIVE(dev->txDmaSem);
		isrfilter_txdma_calls[dev->index]++;
	}
	if (msg & WORK_DMA_RX_MASK) {
		*buf &= ~WORK_DMA_RX_MASK; /* filter event to work-task */
		OS_SEM_GIVE(dev->rxDmaSem);
		isrfilter_rxdma_calls[dev->index]++;
	}
#if __rtems__
	if ((*buf & ~WORK_CORE_MASK) == 0)
		return RTEMS_SUCCESSFUL; /* all work handled.. */
	return rtems_message_queue_send((rtems_id)data, (char *)buf, n);
#else
	if ((*buf & ~WORK_CORE_MASK) == 0)
		return OK; /* all work handled.. */
	return msgQSend((MSG_Q_ID)data, (char *)buf, n, t, p);
#endif
}

int wt_setup2_refcount = 0;
OS_MSGQ_TYPE wt_setup2_msgQ = OS_MSGQ_NULL;
int dev_worktask_setup2(struct grspw_device *dev)
{
	struct grspw_work_config wc;
	int tid;

#ifdef GRSPWPKT_WORKTASK_PRIO
	if (GRSPWPKT_WORKTASK_PRIO >= 0) {
		puts("### This setup is not recommended. It will result in a");
		puts("    dangeling work-task & msgQ created by the driver.");
		puts("    It can be avoided by GRSPWPKT_WORKTASK_PRIO=-1.");
		puts("");
	}
#endif
	wt_setup2_refcount++;
	if (wt_setup2_refcount == 1) {

		/* Create one common Message Queue and work task with max 64
		 * number of messages in the queue.
		 * Work task: default stack size. Set priority to lower than
		 * application tasks (NOTE: THIS MIGHT NOT BE OPTIMAL AND IS
		 * APPLICATION SPECIFIC).
		 */
		wt_setup2_msgQ = OS_MSGQ_NULL;
		tid = grspw_work_spawn(TASK_PRIO+1, 0, &wt_setup2_msgQ, 64);
		if (tid == OS_GRSPW_WORK_SPAWN_ERROR)
			return -1;
	} else if (wt_setup2_msgQ == OS_MSGQ_NULL) {
		return -1;
	}

#ifdef __rtems__
	/* create application semaphores */
	if (rtems_semaphore_create(rtems_build_name('S', 'E', '0', '0'), 0,
	    RTEMS_FIFO | RTEMS_SIMPLE_BINARY_SEMAPHORE | \
	    RTEMS_NO_INHERIT_PRIORITY | RTEMS_LOCAL | \
	    RTEMS_NO_PRIORITY_CEILING, 0, &dev->txDmaSem) != RTEMS_SUCCESSFUL)
		return -1;
	if (rtems_semaphore_create(rtems_build_name('S', 'E', '0', '1'), 0,
	    RTEMS_FIFO | RTEMS_SIMPLE_BINARY_SEMAPHORE | \
	    RTEMS_NO_INHERIT_PRIORITY | RTEMS_LOCAL | \
	    RTEMS_NO_PRIORITY_CEILING, 0, &dev->rxDmaSem) != RTEMS_SUCCESSFUL)
		return -1;
#else
	/* Create application simple semaphores */
	dev->txDmaSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	dev->rxDmaSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
#endif

	/* build a GRSPW work configuration */
	wc.msgisr = (grspw_msgqisr_t)wt_setup2_filter; /* install filter */
	wc.msgisr_arg = (void *)wt_setup2_msgQ;
	/* Assign msgQ and work-task to GRSPW device */
	grspw_work_cfg(dev->dh, &wc);

	return 0;
}

int simplewait2(struct grspw_device *dev, int chan, int rx, int timeout)
{
	int sts;
	unsigned int dmastat;

	if (spacewire_worktask_setup_demo != 2) {
		printf("simplewait2 not available when not in work-task setup2\n");
		return -1;
	}
	if (rx < 1 || rx > 2)
		return -1; /* illegal input*/

	/* Reenable ints and read old Packet Sent / Packet Received DMA bits */
	dmastat = grspw_dma_enable_int(dev->dma[chan], rx, 0);

	/* Wait for interrupt if not work already available  */
	if (rx == 1) {
		if (dmastat & (1 << 6)) /* GRSPW_DMACTRL_PR */
			return 0;
		sts = OS_SEM_TAKE(dev->rxDmaSem, timeout);
	} else {
		if (dmastat & (1 << 5)) /* GRSPW_DMACTRL_PS */
			return 0;
		sts = OS_SEM_TAKE(dev->txDmaSem, timeout);
	}
#ifdef __rtems__
	/* decode semTake() return */
	if (sts == RTEMS_TIMEOUT) {
		return 2;
	} else if (sts != RTEMS_SUCCESSFUL) {
		return -1;
	}
#else
	/* decode semTake() return */
	if (sts == ERROR) {
		if (errno == S_objLib_OBJ_TIMEOUT)
			return 2;
		else
			return -1;
	}
#endif
	return 0;
}

/* one work-task and one associated message Q, we delete them both
 * but to ensure ISR does not use a deleted MsgQ it is unassigned first.
 */
void dev_worktask_close2(struct grspw_device *dev)
{
	/* unassign msgQ first */
	struct grspw_work_config wc = {NULL, NULL};
	grspw_work_cfg(dev, &wc);
	
	wt_setup2_refcount--;
	if (wt_setup2_refcount == 0) {

		/* Send "delete msgQ and exit Work-task message" to work-task */
		grspw_quit_event_detected = 0;
		grspw_work_free(wt_setup2_msgQ, 1);
		/* Wait for work-task to exit */
		while (grspw_quit_event_detected == 0)
			OS_DELAY_TICKS(1);
	}
	dev->msgQ = OS_MSGQ_NULL;
}

int dev_init(int idx)
{
	struct grspw_device *dev = &devices[idx];
	struct grspw_link_state ls;
	int i, ctrl, clkdiv, stscfg;

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

	/* Demonstrate how to make a custom set up of the work-task if
	 * requested by the User. Otherwise leave have the default.
	 */
	switch (spacewire_worktask_setup_demo) {
		case 1:
			if (dev_worktask_setup1(dev)) {
				grspw_close(dev);
				printf("Failed to perform work-task setup1\n");
				return -1;			
			}
			break;
		case 2:
			if (dev_worktask_setup2(dev)) {
				grspw_close(dev);
				printf("Failed to perform work-task setup2\n");
				return -1;			
			}
			break;
		default:
			/* use default set-up */
			grspw_work_cfg(dev->dh, NULL);
			break;
	}

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
	clkdiv |= (GRSPW_RUN_CLKDIV & 0xff);
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

int dev_dma_close_all(int idx)
{
	struct grspw_device *dev = &devices[idx];
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
	struct grspw_device *dev = &devices[idx];

	if (dev->dh == NULL)
		return;

	/* Stop all DMA activity first */
	grspw_stop(DEV(dev));
	/* wait for other tasks to be thrown out from driver */
	OS_DELAY_TICKS(4);

	/* close all DMA channels */
	if (dev_dma_close_all(idx)) {
		printf("FAILED to close GRSPW%d DMA\n", idx);
	}

	/* Demonstrate how to make a custom set up of the work-task if
	 * requested by the User. Otherwise leave have the default.
	 */
	switch (spacewire_worktask_setup_demo) {
		case 1:
			dev_worktask_close1(dev);
			break;
		case 2:
			dev_worktask_close2(dev);
			break;
		default:
			break;
	}

	if (grspw_close(dev->dh)) {
		printf("FAILED to close GRSPW%d\n", idx);
	}
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

	if (stop_tasks > 0)
		tprocesses_stopping[dev->index]++;

	/* 1. */
	spwlib_list_clr(&lst);
	count = -1;
	result = grspw_dma_tx_reclaim(dev->dma[0], 0,
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

		/* Clear TX Packet Flags */
		pktlist_clr_flags(&lst, 0xffff);

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
			/* Clear RX Flags */
			pkt->flags = 0;

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

#ifdef BLOCKING_DMA
rtems_task task_dma_process(rtems_task_argument devidx)
{
	struct grspw_device *dev = &devices[devidx];
	int rc, recv, shutdown;
	int timeout;

	while (stop_tasks < 2) {

		/* Wait for PKT_CNT/4 to arrive before generating
		 * transmit packets
		 */
		if (stop_tasks > 0) {
			timeout = 1; /* shortest timeout during shutdown */
			twaits_stopping[devidx]++;
		} else {
			timeout = BLOCK_TIMEOUT;
			twaits[devidx]++;
		}
		if (spacewire_worktask_setup_demo == 2) {
			rc = simplewait2(dev, 0, 1, timeout);
		} else {
			rc = grspw_dma_rx_wait(dev->dma[0], BLOCK_PKTS, 0, 0xfffffff, timeout);
		}

		if (rc == 2) {
			/* Timeout */
			if (stop_tasks == 0)
				ttimeouts[devidx]++;
			else
				ttimeouts_stopping[devidx]++;
		} else if (rc != 0) {
			/* Error */
			printf("dma_rx_wait(%d) failed: %d\n", dev->index, rc);
		} else if(spacewire_worktask_setup_demo != 2) {
			grspw_dma_rx_count(dev->dma[0], 0, 0, &recv, NULL);
			if (recv < BLOCK_PKTS) {
				printf("dma_rx_wait returned early: %d\n", recv);
				exit(0);
			}
		}

		tprocesses[devidx]++;
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

			rc = grspw_dma_tx_wait(dev->dma[0], 0x0fffffff, 0, PKT_CNT/8-64, 20);
			if (rc == 2) {
				/* timeout */
			} else if (rc != 0) {
				/* Error */
				printf("dma_tx_wait(%d) failed: %d\n", dev->index, rc);
			} else {
				grspw_dma_tx_count(dev->dma[0], 0, 0, &sent, NULL);
				if (sent < (PKT_CNT/8 - 64)) {
					printf("dma_tx_wait returned early: %d\n", sent);
					exit(0);
				}
			}
		}
#endif
	}
	rtems_task_delete(RTEMS_SELF);
}
#endif

extern int router_setup_custom(void);

void test_app(void)
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
	int timeout;
	char *intmode = "YES";
#else
	char *intmode = "NO";
#endif

#ifdef DUAL_LOOPBACK
	printf("\nTest built for loopback between two SpW connectors (DUAL_LOOPBACK)\n"
	       "without SpW Router.\n"
	       "The SpW-links will be used and clocking will affect the throughput.\n"
	       "There are %d GRSPW cores in the system.\n"
	       "Interrupt mode: %s\n"
	       "System clock: %" PRIu32 " us / tick\n",
	       grspw_dev_count(),
	       intmode,
	       rtems_configuration_get_microseconds_per_tick());
#else
	printf("\nTest built for SpW-router with %d SpW Ports and at least %d AMBA ports.\n"
	       "Note that the SpW-links will not be used.\n"
	       "There are %d GRSPW cores in the system.\n"
	       "Interrupt mode: %s\n"
	       "System clock: %" PRIu32 " us / tick\n",
	       ROUTER_SPWLINK_NO, SPW_LINKS_USED, grspw_dev_count(),
	       intmode,
	       rtems_configuration_get_microseconds_per_tick());
#endif
	printf("\n\n");

#ifdef TEST_NGMP
	printf("DMA configuration number: %d\n", dma_cfg);
	switch (dma_cfg) {
	case 1:
		address_region_setup(
                MMU_DISABLE, 
                L2C_DISABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	default:
	case 2:
		address_region_setup(
                MMU_DISABLE, 
                L2C_ENABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 3:
		address_region_setup(
                MMU_ENABLE, 
                L2C_DISABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 4:
		address_region_setup(
                MMU_ENABLE, 
                L2C_ENABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 5:
		address_region_setup(
                MMU_DISABLE, 
                L2C_ENABLE | L2C_MTRR_ENABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 6:
		address_region_setup(
                MMU_ENABLE, 
                L2C_ENABLE | L2C_MTRR_ENABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 7:
		address_region_setup(
                MMU_ENABLE, 
                L2C_DISABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_MEM|IOMMU_PCI_BUS_MEM|IOMMU_PREFETCH_DISABLE);
		break;
	case 8:
		address_region_setup(
                MMU_ENABLE, 
                L2C_ENABLE | L2C_MTRR_ENABLE, 
                IOMMU_SPW_BUS_MEM|IOMMU_PCI_BUS_MEM|IOMMU_PREFETCH_DISABLE);
		break;
	}
#endif

	printf("Setting up SpaceWire router\n");
	if (router_setup_custom()) {
#ifdef DUAL_LOOPBACK
		/* assuming no router */
		printf("Failed router initialization. Continuing.\n");
#else
		printf("Failed router initialization. Aborting.\n");
		return;
#endif
	}

	memset(devices, 0, sizeof(devices));
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
		result = pkts_init(&devices[i], initiator);
		if ( result < 0 ) {
			printf("Packet initialization failed SpW%d: %d\n", 
				i, result);
			return;
		}

		/* Assign route table for SPW devices. All use the same "routing
		 * table".
		 */
		devices[i].routetab = &routetab[0];

		/* Init Packet Check */
		devices[i].rxpkt_number_next = 0;
		devices[i].txpkt_number_next = 0;
		devices[i].rxcheck_no = 0;
		devices[i].txcheck_no = 0;
	}

	printf("\n\n");
	for (i=0; i<SPW_LINKS_USED; i++) {
		printf("SpW%d Number of packets:\n", i);
		cnt = spwlib_list_cnt(&devices[i].rx_list);
		printf( "      rx_list: %d\n", cnt);
		cnt = spwlib_list_cnt(&devices[i].rx_prep_list);
		printf( "      rx_prep_list: %d\n", cnt);
		cnt = spwlib_list_cnt(&devices[i].tx_list);
		printf( "      tx_list: %d\n", cnt);
		cnt = spwlib_list_cnt(&devices[i].check_list);
		printf( "      check_list: %d\n", cnt);		
	}

	printf("\n\nStarting SpW DMA channels\n");
	for ( i=0; i<SPW_LINKS_USED; i++) {
		printf("Starting SpW%d: ", i);
		fflush(NULL);
		if ( grspw_start(DEV(&devices[i])) ) {
			printf("Failed to initialize SpW%d\n", i);
			return;
		}
		printf("Started Successfully\n");
	}

	printf("Starting Packet processing loop, will take approx %d secs\n",
		spacewire_length_secs);

	/* Packet processing loop */
	options = 1; /* Only allow RX prepare first time */
	devno = -1;
	shutdown = 0;
	loopcnt = 0;
	stop = 0;
	stop_tasks = 0;

#ifdef DEMO_PROFILING
	rtems_cpu_usage_reset();
#endif

	rtems_clock_get_uptime(&t0);
	pci_test_start();
	while ( shutdown == 0 ) {
		/* Check if total seconds has gone now and then do stop sequence */
		if ( (stop == 0) && ((loopcnt & 0x7) == 0x7) ) {
			rtems_clock_get_uptime(&t1);
			if ( t1.tv_sec > (t0.tv_sec + spacewire_length_secs) ) {
				stop = 1;
				stop_tasks = 1;
				pci_test_stop();
				/* Calculate when to stop */
				t1.tv_sec += TEST_END_WAIT_SEC;
			}
		} else if ( stop && ((loopcnt & 0x7) == 0x7) ) {
			/* Check if INITIATOR:
			 *   - TX_SCHEDULE Count is Zero (nothing more to send)
			 *   - Number of Received == Number of Transmitted
			 */
			if ( stop == 1 ) {
				grspw_dma_tx_count(devices[SPW_INITIATOR].dma[0], &tx_send,
							&tx_sched, NULL, NULL);
				if (tx_sched == 0 && tx_send == 0)
					stop = 2;
			}
			if ( stop == 2 ) {
				/* Stop if all packets we sent has been collected
				 * back at the receiver.
				 */
				grspw_stats_get(DEV(&devices[SPW_INITIATOR]), &stats);
				grspw_dma_tx_count(devices[SPW_INITIATOR].dma[0], &tx_send, &tx_sched, &tx_sent, NULL);
				if ((stats.chan[SPW_INITIATOR].rx_pkts ==
				     stats.chan[SPW_INITIATOR].tx_pkts) &&
				    ((tx_send + tx_sent + tx_sched) == 0)) {
					printf("Test finished (nothing more to send) rx_pkts (%d) == tx_pkts(%d), stop: %d\n", stats.chan[SPW_INITIATOR].rx_pkts, stats.chan[SPW_INITIATOR].tx_pkts, stop);
					stop = 3;
				}
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
			     ((t2.tv_sec == t1.tv_sec) && (t2.tv_nsec > t1.tv_nsec))) {
				printf("Test aborted due to time, stop: %d\n", stop);
				break;
			}
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

			tprocesses[i]++;
			shutdown = grspw_process(&devices[i], options, do_stop);
			if ( shutdown ) {
				printf("Shutting down: \n");
				devno = i;
				break;
			}
		}
#ifdef INTERRUPT_MODE
		int rc;
		if (stop > 0) {
 			timeout = 1; /* shortest timeout during shutdown */
			twaits_stopping[SPW_INITIATOR]++;
		} else {
			timeout = BLOCK_TIMEOUT;
			twaits[SPW_INITIATOR]++;
		}
		if (spacewire_worktask_setup_demo == 2) {
			rc = simplewait2(&devices[SPW_INITIATOR], 0, 1, timeout);
		} else {
			rc = grspw_dma_rx_wait(devices[SPW_INITIATOR].dma[0], BLOCK_PKTS, 0, 0xfffffff, timeout);
		}
		if (rc == 2) {
			/* Timeout */
			if (stop == 0)
				ttimeouts[SPW_INITIATOR]++;
			else
				ttimeouts_stopping[SPW_INITIATOR]++;
		} else if (rc != 0) {
			/* Error */
			printf("dma_rx_wait(%d) failed: %d\n", SPW_INITIATOR, rc);
		}
#endif
		options = 3;
		loopcnt++;
	}

	/* Get Stats of Channel 0, this is to determine how many packets have
	 * been received.
	 */
	grspw_stats_get(DEV(&devices[SPW_INITIATOR]), &stats);
	tot = stats.chan[SPW_INITIATOR].rx_pkts;

	/* Stop time */
	rtems_clock_get_uptime(&t1);

#ifdef DEMO_PROFILING
	rtems_cpu_usage_report();
#endif

	stop_tasks = 2;
	pci_test_task_stop();

	/* Let the other tasks shutdown, timeout must be longer than
	 * dma_rx_wait timeout plus time to cleanup.
	 */
	OS_DELAY_TICKS(40);

	printf("\n\nShutting down: %d (SpW %d), loops=%d, stop=%d\n\n", shutdown, devno, loopcnt, stop);

	for (i=0; i<SPW_LINKS_USED; i++) {
		printf("\n\n--- SpW%d Device ---\n", i);
		printf(" RX List count: %d\n", spwlib_list_cnt(&devices[i].rx_list));
		printf(" RX PREP List count: %d\n", spwlib_list_cnt(&devices[i].rx_prep_list));
		printf(" TX List count: %d\n", spwlib_list_cnt(&devices[i].tx_list));
		printf(" Check List count: %d\n", spwlib_list_cnt(&devices[i].check_list));
		grspw_dma_rx_count(devices[i].dma[0], &rx_ready, &rx_sched, &rx_recv, &rx_hwcnt);
		grspw_dma_tx_count(devices[i].dma[0], &tx_send, &tx_sched, &tx_sent, &tx_hwcnt);
		printf(" DRVQ RX_READY: %d\n", rx_ready);
		printf(" DRVQ RX_SCHED: %d\n", rx_sched);
		printf(" DRVQ RX_RECV: %d\n", rx_recv);
		printf(" DRVQ RX_HWCNT: %d\n", rx_hwcnt);
		printf(" DRVQ TX_SEND: %d\n", tx_send);
		printf(" DRVQ TX_SCHED: %d\n", tx_sched);
		printf(" DRVQ TX_SENT: %d\n", tx_sent);
		printf(" DRVQ TX_HWCNT: %d\n", tx_hwcnt);

		grspw_stats_get(DEV(&devices[i]), &stats);
		grspw_stats_print(DEV(&devices[i]), &stats);
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
#ifdef TEST_NGMP
	printf("#DMACFG=%02d\n", dma_cfg);
#endif
	printf("#SPWRESULT=%f\n", 8 * ((((float)tot)*PKT_SIZE)/time) * SPW_DIRECTIONS_USED);

	/* Check Content and Packet number of resulting Received Packets
	 * Since we stopped transmitting abrupt, we look at the sequence number
	 * of first packet to init packet checker.
	 */
	if ( spwlib_list_is_empty(&devices[SPW_INITIATOR].check_list) == 0 ) {
		struct pkt_hdr *pkt_hdr = (struct pkt_hdr *)
			devices[SPW_INITIATOR].check_list.head->data;
		devices[SPW_INITIATOR].rxpkt_number_next = pkt_hdr->pkt_number;

		result = pktlist_check_rx(&devices[SPW_INITIATOR],
					&devices[SPW_INITIATOR].check_list,
					1);
		if ( result ) {
			printf("###PACKET CONTENT/PKT-SEQUENCE CHECK FAILED: %d\n", result);
		} else {
			printf("PACKET CONTENT/PKT-SEQUENCE CHECK SUCCESSFUL\n");
		}
	} else {
		printf("###NO RECEIVED PACKETS IN CHECKLIST: PKT CHECK FAILED\n");
	}

	/* Print PCI test results */
	pci_test_print_results();

	for ( i=0; i<SPW_LINKS_USED; i++) {
		dev_cleanup(i);
	}
	sleep(1);
	pkt_pool_free();

	printf("\n\n");
	return;
}
