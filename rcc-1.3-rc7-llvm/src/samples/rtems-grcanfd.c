/* Simple GRCAN-FD interface example.
 *
 *  COPYRIGHT (c) 
 *  Cobham Gaisler 2019,
 *
 * Example of two CAN nodes on the same target LEON device communicates in
 * "loopback" mode.
 *
 * Three tasks are communicating:
 *  - [Node0] TX Task sends data to Loopback task,
 *  - [Node1] Loopback task Received CAN messages, modified the ID and resends
 *            the CAN message back to the bus immediately.
 *  - [Node0] RX Task receives CAN message again and verifies that ID and data
 *            is correct.
 * 
 * The above tasks and interrupt handlers collect CAN statistics which are
 * printed periodically by a fourth status task. It call can_print_stats().
 *
 * In order for the example to work the following is required:
 *  - two GRCANFD nodes on the same LEON device
 *  - Node0 and Node1 must be connected to the same bus, externally or
 *    internally
 *    in the FPGA.
 *
 * Note that it is also possible to operate the example with two GRCAN nodes
 * by uncommenting the ENABLE_FD_FRAMES define.
 */

/* GR-CPCI-GR740 notes:
 * When running this example on GR740, the following have to be configured:
 * - CAN pin multiplexing
 * - Enable GRCAN clock
 * - uncomment #define ENABLE_FD_FRAMES
 *
 * This can be done in GRMON2 using the following commands before running the
 * example:
 *   grmon2> wmem 0xffa0b000 0x000ffc3c
 *   grmon2> grcg enable 5
 *
 * In addition, when using GRCAN on the GR-CPCI-GR740 development board, the
 * PCB signal matrix has to be configured correspondingly:
 * - JP11 jumpers 15, 16, 21 and 22 shall be in the "BC" position.
 *
 * See the "GR-CPCI-GR740 Quick Start Guide" for more information.
 */
 
#include <rtems.h>
#define CONFIGURE_INIT
#include <bsp.h>		/* for device driver prototypes */
#include <assert.h>

rtems_task Init(rtems_task_argument argument);	/* forward declaration needed */

/* configure RTEMS kernel */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             16
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (24 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_INIT_TASK_PRIORITY	100
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_MAXIMUM_DRIVERS 16

#define RTEMS_PCI_CONFIG_LIB
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO

#include <rtems/confdefs.h>

/* Configure Driver manager */
#if defined(RTEMS_DRVMGR_STARTUP) && defined(LEON3)	/* if --enable-drvmgr was given to configure */
 /* Add Timer and UART Driver for this example */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
 #endif /* CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
 #endif /* CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER */
#endif /* defined(RTEMS_DRVMGR_STARTUP) && defined(LEON3) */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF	/* PCI is for GR-RASTA-IO GRCAN */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI	/* PCI is for GR-RASTA-IO GRCAN */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2	/* PCI is for GR-RASTA-IO GRCAN */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO	/* GR-RASTA-IO PCI TARGET has a GRCAN core */

#ifdef LEON2
 /* PCI support for AT697 */
 #define CONFIGURE_DRIVER_LEON2_AT697PCI
 /* AMBA PnP Support for GRLIB-LEON2 */
 #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif /* LEON2 */

#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN	/* GRCAN Driver */

#include <drvmgr/drvmgr_confdefs.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <ctype.h>
#include <bsp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/* Include driver configurations and system initialization */
#include "config.c"

#include <grlib/grcan.h>

/* Select CAN core(s) to be used in sample application.
 *  - 0                        (First CAN core)
 *  - 1                        (Second CAN core)
 *  - etc...
 */
#ifndef GRCAN_DEVICE0_NUMBER
 #define GRCAN_DEVICE0_NUMBER 1
#endif
#ifndef GRCAN_DEVICE1_NUMBER
 #define GRCAN_DEVICE1_NUMBER 2
#endif

/* If RX_MESSAGES_CHANGED if defined it is assumed that the
 * received messages has been changed from the transmitted
 * message. In that case it is assumed that the ID field of
 * each message has been decremented once.
 *
 * This option is useful when the messages are looped on an
 * external CAN board. CAN is not designed to receive the
 * exact same message as is beeing transmitted:
 *  WE SEND -> CAN_BUS -> External CAN Board changes the ID 
 *  field -> CAN_BUS -> WE RECEIVE and verify message
 */
#define RX_MESSAGES_CHANGED_ID
/*
 * #define RX_MESSAGES_CHANGED_DATA
 */

/* Define this to get more statistics printed to console */
/*#define PRINT_MORE_STATS*/

/* Define this to enable FD frames in RX and TX handling */
/*#define ENABLE_FD_FRAMES*/

/* Define this to enable FD frames in Loopback */
/*#define LOOPBACK_ENABLE_FD_FRAMES*/

/* Disable all FD enable frames if FD is disabled by the user */
#ifdef ENABLE_FD_FRAMES
  #define NUM_FRAMES 12
#else
  #define NUM_FRAMES 4
#endif

/* CAN Channel select */
enum {
	CAN_CHAN_SEL_A,
	CAN_CHAN_SEL_B,
	CAN_CHAN_SEL_NUM
};
int can_chan_sel = CAN_CHAN_SEL_A;	/* Default to channel A */
int loop_chan_sel = CAN_CHAN_SEL_A;	/* Default to channel A */

#define UNUSED(var) ((void) var)
/* Status printing Task entry point */
rtems_task status_task1(rtems_task_argument argument);

/* CAN routines */
int can_init(int devno, int chan_sel);
void can_start(void);
void can_print_stats(void);

/* Loopback routines */
int loop_init(int devno, int chan_sel);
void loop_start(void);

int status_init(void);
void status_start(void);

/* ========================================================= 
   initialisation */

/* Initializes the 12 CAN messages in the global variable 
 * "CANFDMsg fdmsgs[12]".
 */
void init_send_fd_messages(void);

rtems_task Init(rtems_task_argument ignored)
{
	UNUSED(ignored);
	rtems_status_code status;
	int loop_ret;

	printf("******** Initializing CAN test ********\n");

	/* Initialize Driver manager, in config.c */
	system_init();

	/*drvmgr_print_devs(0xfffff);
	drvmgr_print_topo();*/

	init_send_fd_messages();

	/* Initialize RX/TX */
	if (can_init(GRCAN_DEVICE0_NUMBER, can_chan_sel)) {
		printf("CAN INITIALIZATION FAILED, aborting\n");
		exit(1);
	}

	/* Initialize Loopback */
	loop_ret = loop_init(GRCAN_DEVICE1_NUMBER, loop_chan_sel);
	if (loop_ret) {
		printf("INFO: LOOP INITIALIZATION FAILED. Loop task deactivated.\n");
	}

	if (status_init()) {
		printf("STATUS INITIALIZATION FAILED, aborting\n");
		exit(3);
	}

	if (loop_ret == 0) {
		loop_start();
	}

	can_start();

	status_start();

	status = rtems_task_delete(RTEMS_SELF);
	assert(status == RTEMS_SUCCESSFUL);
}

rtems_id tstatus;		/* array of task ids */
rtems_name tstatusname;		/* array of task names */

int status_init(void)
{
	rtems_status_code status;

	tstatusname = rtems_build_name('S', 'T', 'S', '0');

	/*
	 * Create a status printing task with the highest priority, this may
	 * result in CAN messages may be dropped. The CAN bus has no flow
	 * control stopping when receiver is full. ==> packets may be dropped
	 * when CAN receive task doesn't get enough CPU time. Packet drop can
	 * be reduced by inserting sleep calls and increasing the receive CAN
	 * buffer size.
	 */
	status = rtems_task_create(
		tstatusname,
		4,
		RTEMS_MINIMUM_STACK_SIZE * 4,
		RTEMS_DEFAULT_MODES | RTEMS_PREEMPT,
		RTEMS_FLOATING_POINT,
		&tstatus
	);
	if (status != RTEMS_SUCCESSFUL) {
		return -1;
	}

	return 0;
}

void status_start(void)
{
	rtems_status_code status;

	printf("Starting status task1\n");

	/* Starting Status task */
	status = rtems_task_start(tstatus, status_task1, 1);
	assert(status == RTEMS_SUCCESSFUL);
}

/* Status Task */
rtems_task status_task1(rtems_task_argument unused)
{
	UNUSED(unused);

	while (1) {
		can_print_stats();
		fflush(stdout);
		sleep(2);
	}
}

/* CAN Implementation */

rtems_task canrx_task(rtems_task_argument argument);
rtems_task cantx_task(rtems_task_argument argument);

static rtems_id tds[2];		/* array of task ids */
static rtems_name tnames[2];	/* array of task names */
/* Arbitration of GRCAN device between RX and TX tasks. */
static rtems_id tx_sem;

/* File descriptors of /dev/grcan0 */
void *candev;

/* Print one CAN message to terminal */
void print_fdmsg(int i, CANFDMsg *msg);

/* Verify content of CAN message 'msg' against msgs[index].
 * Returns what went wrong.
 */
int verify_fdmsg(CANFDMsg *msg, int index, int silent);

/* ========================================================= 
   initialisation */

/* CAN Baud-rate settings. Driver will auto-generate CAN timing parameters
 * for this baud-rate.
 *
 * Set CAN_BAUD to zero to use the CAN_TIMING to control baud-rate instead.
 */
int CAN_BAUD = 250000;
int CAN_FD_BAUD = 2000000; /* only relevant when FD is enabled */

/* CAN baud-rate timing parameters, are overriden by the
 * CAN_BAUD, CAN_FD_BAUD paramters above. 
 * 
 */
static const struct grcan_timing CAN_TIMING = {
#if 0
	/* Set baud rate: 250k @ 30MHz */
	.scaler = 3,
	.ps1 = 8,
	.ps2 = 5,
	.rsj = 1,
	.bpr = 1,
#elif 0
	/* Set baud rate: 250k @ 40MHz */
	.scaler = 7,
	.ps1 = 0xf,
	.ps2 = 0x3,
	.rsj = 0x1,
	.bpr = 0,
#else
	/* Set baud rate: 250k @ 250MHz */
	/* according to calc_can_btrs */
	.scaler = 0x27,
	.ps1 = 0xf,
	.ps2 = 0x8,
	.rsj = 0x1,
	.bpr = 0x0,
#endif
};

static const struct grcanfd_timing CANFD_NOM_TIMING = {
	/* Set Nominal Baud-rate parameters here */
};
static const struct grcanfd_timing CANFD_FD_TIMING = {
	/* Set FD Baud-rate parameters here */
};

static const struct grcan_selection CAN_CHAN_SEL[CAN_CHAN_SEL_NUM] = {
	{
		/* Channel A */
		.selection = 0,
		.enable0 = 0,
		.enable1 = 1,
	},
	{
		/* Channel B */
		.selection = 1,
		.enable0 = 1,
		.enable1 = 0,
	},
};

static int can_init_dev(int devno, int chan_sel, void **dev_new);

int can_init(int devno, int chan_sel)
{
	rtems_status_code status;
	int i;

	printf("******** Initializing GRCAN test ********\n");

	for (i = 0; i < 2; i++) {
		tnames[i] = rtems_build_name('T', 'D', 'C', '0' + i);
	}

  /*** Create, but do not start, CAN RX/TX tasks ***/
	/* RX task */
	status = rtems_task_create(tnames[0], 2, RTEMS_MINIMUM_STACK_SIZE * 4,
				   RTEMS_DEFAULT_MODES,
				   RTEMS_FLOATING_POINT, &tds[0]
	    );
	assert(status == RTEMS_SUCCESSFUL);

	/* TX task */
	status = rtems_task_create(tnames[1], 5, RTEMS_MINIMUM_STACK_SIZE * 4,
				   RTEMS_DEFAULT_MODES,
				   RTEMS_FLOATING_POINT, &tds[1]
	    );
	assert(status == RTEMS_SUCCESSFUL);

	if ( rtems_semaphore_create(rtems_build_name('S', 'T', 'X', '0'),
		1,
		RTEMS_FIFO|RTEMS_SIMPLE_BINARY_SEMAPHORE|RTEMS_NO_INHERIT_PRIORITY|\
		RTEMS_LOCAL|RTEMS_NO_PRIORITY_CEILING,
		0,
		&tx_sem) != RTEMS_SUCCESSFUL ) {
		return -2;
	}

	return can_init_dev(devno, chan_sel, &candev);
}

static rtems_id tloop;
static rtems_name tloopname;
rtems_task loop_task1(rtems_task_argument argument);

#ifdef LOOPBACK_ENABLE_FD_FRAMES
  #define LB_MSGTYPE CANFDMsg
  #define LB_READ_MSG(_dev, _msgs, _len) grcanfd_read(_dev, _msgs, _len)
  #define LB_WRITE_MSG(_dev, _msgs, _len) grcanfd_write(_dev, _msgs, _len)
#else
  #define LB_MSGTYPE CANMsg
  #define LB_READ_MSG(_dev, _msgs, _len) grcan_read(_dev, _msgs, _len)
  #define LB_WRITE_MSG(_dev, _msgs, _len) grcan_write(_dev, _msgs, _len)
#endif

struct loop_stats {
	unsigned int received;
	unsigned int sent;
	unsigned int rx_err;
};
struct loop_ctx {
	struct loop_stats stats;
	void *dev;
	LB_MSGTYPE msg[50];
};

struct loop_ctx _loop_ctx;
static CANFDMsg fdmsgs[12];

/* Loopback task */
rtems_task loop_task1(rtems_task_argument arg)
{
	int rxmsgs;
	int txmsgs;
	struct loop_ctx *ctx;
	int i = 0;

	ctx = (struct loop_ctx *) arg;
	memset(&ctx->stats, 0, sizeof(struct loop_stats));

	iprintf("%s: Entering CAN Loop-Back loop\n", __func__);
	grcan_set_rxcomplete(ctx->dev, 1);
	while (1) {
		/* blocking read */
		rxmsgs = LB_READ_MSG(ctx->dev, &ctx->msg[0], 4);
		if (rxmsgs > 0) {
			ctx->stats.received += rxmsgs;
#ifdef RX_MESSAGES_CHANGED_ID
			for (int j = 0; j < rxmsgs; j++) {
				ctx->msg[j].id = ctx->msg[j].id - 1;
			}
#endif

			/* Just try send if possible */
			txmsgs = LB_WRITE_MSG(ctx->dev, &ctx->msg[0], rxmsgs);
			if (txmsgs > 0) {
				ctx->stats.sent += txmsgs;
			} else {
				grcan_stop(ctx->dev);
				grcan_start(ctx->dev);
			}
		} else if ((rxmsgs == -2) || (rxmsgs == -4)) {
			iprintf("%s: Experienced RX error (%d) recovering with grcan_start()\n", __func__, rxmsgs);
			/* OK to start since this task is the only user of ctx->dev. */
			grcan_stop(ctx->dev);
			grcan_start(ctx->dev);
			ctx->stats.rx_err++;
			sleep(1);
		} else {
			/* if in non-blocking mode work with other stuff here */
			iprintf("%s: RX blocking not working\n", __func__);
			ctx->stats.rx_err++;
			sleep(1);
		}
		i++;
	}

	while (1) {
		iprintf("%s: Sleeping loop task\n", __func__);
		sleep(1);
	}
}

int loop_init(int devno, int chan_sel)
{
	rtems_status_code status;

	printf("******** Initializing GRCAN loopback ********\n");

	tloopname = rtems_build_name('L', 'O', 'O', 'P');

	/*** Create, but do not start, CAN loop task ***/

	status = rtems_task_create(
		tloopname,
		/*
		 * Valid task priorities range from a high of 1 to a low of
		 * 255.
		 */
		3,
		RTEMS_MINIMUM_STACK_SIZE * 4,
		RTEMS_DEFAULT_MODES,
		RTEMS_DEFAULT_ATTRIBUTES,
		&tloop
	);
	assert(status == RTEMS_SUCCESSFUL);

	return can_init_dev(devno, chan_sel, &_loop_ctx.dev);
}

/*
 * Open and bring up a can device in a sane configuration.
 *
 * This function does not change global variables, except via dev_new.
 */
static int can_init_dev(int devno, int chan_sel, void **dev_new)
{
        int ret;
	const struct grcan_selection *selection;
	void *dev;

	if (!dev_new) {
		return -1;
	}
	*dev_new = NULL;

	printf("Opening GRCAN device %d\n", devno);
	dev = grcan_open(devno);
	if (NULL == dev) {
		printf("Failed to open GRCAN device %d\n", devno);
		return -1;
	}

	/* Start GRCAN driver */

	/* Select CAN channel */
	if (chan_sel == CAN_CHAN_SEL_A) {
		selection = &CAN_CHAN_SEL[CAN_CHAN_SEL_A];
	} else {
		selection = &CAN_CHAN_SEL[CAN_CHAN_SEL_B];
	}

	/* Set up CAN driver:
	 *  ¤ baud rate
	 *  ¤ Channel
	 *  ¤ TX blocking, and wait for all data to be sent.
	 *  ¤ RX non-blocking depending on ONE_TASK mode
	 */
	/* Set baud */
	if (CAN_BAUD != 0) {
		if (grcan_canfd_capable(dev)) {
			ret = grcanfd_set_speed(dev, CAN_BAUD, CAN_FD_BAUD);
		} else {
			ret = grcan_set_speed(dev, CAN_BAUD);
		}
		if (ret) {
			printf("grcan*_set_speed() failed: %d\n", ret);
			return -1;
		}
	} else {
		if (grcan_canfd_capable(dev)) {
			ret = grcanfd_set_btrs(dev, &CANFD_NOM_TIMING, &CANFD_FD_TIMING);
		} else {
			ret = grcan_set_btrs(dev, &CAN_TIMING);
		}
		if (ret) {
			printf("grcan*_set_btrs() failed: %d\n", ret);
			return -1;
		}
	}
	ret = grcan_set_selection(dev, selection);
	if (ret) {
		printf("grcan_set_selection() failed: %d\n", ret);
		return -1;
	}

	ret = grcan_set_txcomplete(dev, 1);
	if (ret) {
		printf("grcan_set_txcomplete() failed: %d\n", ret);
		return -1;
	}

	ret = grcan_set_rxcomplete(dev, 0);
	if (ret) {
		printf("grcan_set_rxcomplete() failed: %d\n", ret);
		return -1;
	}

#ifdef ONE_TASK
	/* in one task mode, we want TX to block instead */
	ret = grcan_set_rxblock(dev, 0);
#else
	/* in two task mode, we want TX _and_ RX to block */
	ret = grcan_set_rxblock(dev, 1);
#endif
	if (ret) {
		printf("grcan_set_rxblock() failed: %d\n", ret);
		return -1;
	}

	ret = grcan_set_txblock(dev, 1);
	if (ret) {
		printf("grcan_set_txblock() failed: %d\n", ret);
		return -1;
	}

	ret = grcan_clr_stats(dev);
	if (ret) {
		printf("grcan_clr_stats() failed: %d\n", ret);
		return -1;
	}

	/* Start communication */
	ret = grcan_start(dev);
	if (ret) {
		printf("grcan_start() failed: %d\n", ret);
		return -1;
	}

	*dev_new = dev;

	return 0;
}

void can_start(void)
{
	rtems_status_code status;

	/* Starting receiver first */
	status = rtems_task_start(tds[0], canrx_task, 1);
	assert(status == RTEMS_SUCCESSFUL);

	status = rtems_task_start(tds[1], cantx_task, 1);
	assert(status == RTEMS_SUCCESSFUL);
}

void loop_start(void)
{
	rtems_status_code status;

	/* Starting loop task */
	status = rtems_task_start(
		tloop,
		loop_task1,
		(rtems_task_argument) &_loop_ctx
	);
	assert(status == RTEMS_SUCCESSFUL);
}

#define ID_GAISLER 0x2000

void init_send_fd_messages(void)
{
	/* Send 1 STD Message */
	fdmsgs[0].extended = 0;
	fdmsgs[0].rtr = 0;
	fdmsgs[0].fdopts = GRCAN_FDOPT_NOM;
	fdmsgs[0].id = 10;
	fdmsgs[0].len = 4;
	fdmsgs[0].data.bytes[0] = 0x2;
	fdmsgs[0].data.bytes[1] = 0xc4;
	fdmsgs[0].data.bytes[2] = 0x4f;
	fdmsgs[0].data.bytes[3] = 0xf2;
	fdmsgs[0].data.bytes[4] = 0x23;

	/* Send 1 EXT Message */
	fdmsgs[1].extended = 1;
	fdmsgs[1].rtr = 0;
	fdmsgs[1].fdopts = GRCAN_FDOPT_NOM;
	fdmsgs[1].id = 10;
	fdmsgs[1].len = 4 + 1;
	fdmsgs[1].data.bytes[0] = 0x2;
	fdmsgs[1].data.bytes[1] = 0xc4;
	fdmsgs[1].data.bytes[2] = 0x4f;
	fdmsgs[1].data.bytes[3] = 0xf2;
	fdmsgs[1].data.bytes[4] = 0x23;
	fdmsgs[1].data.bytes[5] = 0xa2;

	/* Send 1 Ext FD Message */
	fdmsgs[2].extended = 1;
	fdmsgs[2].rtr = 0;
	fdmsgs[2].fdopts = GRCAN_FDOPT_NOM;
	fdmsgs[2].id = 10 + 880;
	fdmsgs[2].len = 8;
	fdmsgs[2].data.bytes[0] = 0xaa;
	fdmsgs[2].data.bytes[1] = 0xbb;
	fdmsgs[2].data.bytes[2] = 0x11;
	fdmsgs[2].data.bytes[3] = 0x22;
	fdmsgs[2].data.bytes[4] = 'U';
	fdmsgs[2].data.bytes[5] = 0x12;
	fdmsgs[2].data.bytes[6] = 0xff;
	fdmsgs[2].data.bytes[7] = 0x00;

	/* Send 1 EXT FD Message */
	fdmsgs[3].extended = 1;
	fdmsgs[3].rtr = 0;
	fdmsgs[3].fdopts = GRCAN_FDOPT_NOM;
	fdmsgs[3].id = 0xff | ID_GAISLER;
	fdmsgs[3].len = 7;
	fdmsgs[3].data.bytes[0] = 'G';
	fdmsgs[3].data.bytes[1] = 'a';
	fdmsgs[3].data.bytes[2] = 'i';
	fdmsgs[3].data.bytes[3] = 's';
	fdmsgs[3].data.bytes[4] = 'l';
	fdmsgs[3].data.bytes[5] = 'e';
	fdmsgs[3].data.bytes[6] = 'r';

#ifdef ENABLE_FD_FRAMES
	/* Generate EXT FD Messages of length N(i) */
	for (int i = 4; i < (4 + sizeof(i2len)); i++) {
		char i2len[8] = {12, 16, 20, 24, 24, 32, 48, 64};

		fdmsgs[i].extended = 1;
		fdmsgs[i].rtr = 0;
		fdmsgs[i].fdopts = GRCAN_FDOPT_FDFRM;
		if (i != 8) {
			fdmsgs[i].fdopts |= GRCAN_FDOPT_FDBTR;
		}
		fdmsgs[i].id = 10 + (i - 4) * 2;
		fdmsgs[i].len = i2len[i - 4];
		for (int j = 0; j < fdmsgs[i].len; j++) {
			fdmsgs[i].data.bytes[j] = j + 4;
		}
	}
#endif
}

/* Verify content of a CAN message */
int verify_fdmsg(CANFDMsg *msg, int index, int silent)
{
	int i;
	CANFDMsg *src = &fdmsgs[index];

	if (
		(msg->extended && !src->extended) ||
		(!msg->extended && src->extended)
	) {
		if (!silent) {
			printf("Expected extended=%d but got extended=%d\n",
			       src->extended, msg->extended );
		}
		return -1;
	}

	if (msg->rtr != src->rtr) {
		/* Remote Transmission Request does not match */
		return -2;
	}

	if (msg->fdopts != src->fdopts) {
		/* FD options does not match */
		if (!silent) {
			printf("Expected fdopts=0x%x but got fdopts=0x%x\n",
			       src->fdopts, msg->fdopts);
		}
		return -3;
	}
#ifdef RX_MESSAGES_CHANGED_ID
	/* Decremented the ID once */
	if (msg->id != (src->id - 1)) {
		if (!silent) {
			printf("Expected id=0x%" PRIu32 " but got id=0x%" PRIu32
			       "\n", src->id - 1, msg->id);
		}
#else
	if (msg->id != src->id) {
		if (!silent) {
			printf("Expected id=0x%" PRIu32 " but got id=0x%" PRIu32
			       "\n", src->id, msg->id);
		}
#endif
		return -4;
	}

	if (msg->len != src->len) {
		if (!silent) {
			printf("Expected len=%d but got len=%d\n",
			       src->len, msg->len);
		}
		return -5;
	}
	for (i = 0; i < msg->len; i++) {
#ifdef RX_MESSAGES_CHANGED_DATA
		if (msg->data.bytes[i] != (src->data.bytes[i] + 1))
#else
		if (msg->data.bytes[i] != src->data.bytes[i])
#endif
			return -10 - i;
	}
	return 0;
}

/* Staticstics */
static volatile int rxpkts = 0, txpkts = 0;
/*
 * rx_syncs: Number of times an unexpected packet is received.
 * rx_errors: Number of packets not matching a packet known by the example.
 */
static volatile int rx_syncs = 0, rx_errors = 0;

/* RX Task */
rtems_task canrx_task(rtems_task_argument unused)
{
	UNUSED(unused);
	CANFDMsg rxmsgs[10];
	int i, j, cnt, index = 0, error, e, tot = 0;

	memset(rxmsgs, 0, sizeof(rxmsgs));

#ifdef ENABLE_FD_FRAMES
	if (grcan_canfd_capable(candev) == 0) {
		printf("CAN RX device not FD capable. Aborting.\n");
		exit(-1);
	}
#endif

	while (1) {
		cnt = grcanfd_read(candev, rxmsgs, 10);
		if (cnt < 1) {

			/* blocking mode: should not fail unless CAN errors.
			 * In this example we only handle bus-off errors
			 */
			if (cnt < 0) {
				rtems_status_code status;

				printf("%s: read returned %d, doing error recovery with grcan_start()\n", __func__, cnt);
				status = rtems_semaphore_obtain(tx_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
				if (status != RTEMS_SUCCESSFUL) {
					printf("%s: FAILURE: rtems_semaphore_obtain(tx_sem) failed\n", __func__);
					exit(1);
				}
				grcan_stop(candev);
				grcan_start(candev);
				rtems_semaphore_release(tx_sem);
			} else {
				printf("CAN grcan_read() failed: %d\n", cnt);
				break;
			}
			continue;
		}

		/* Statistics */
		rxpkts += cnt;

		/* For every message received we compare the content against
		 * expected content.
		 *
		 * If a message have been dropped we synchronize with the
		 * message stream to avoid getting multiple errors from one
		 * dropped message.
		 *
		 */
		for (i = 0; i < (int) (cnt); i++) {
			error = verify_fdmsg(&rxmsgs[i], index, 0);
			if (error) {
				printf("Message rx error: %d, index: %d, at msg %d\n",
				       error, index, tot);

				/* Print message */
				print_fdmsg(0, &rxmsgs[i]);

				/* Try to sync if one has been lost */
				e = 0;
				for (j = 0; j < NUM_FRAMES; j++) {
					if (!verify_fdmsg(&rxmsgs[i], j, 0)) {
						printf ("Synced from message %d to %d\n", index, j);
						rx_syncs++;
						index = j;
						e = 1;
						break;
					}
				}
				if (e != 1) {
					rx_errors++;
				}
			}
			index++;
			if (index >= NUM_FRAMES) {
				index = 0;
			}
			tot++;
		}
	}

	while (1) {
		printf("Sleeping Task1\n");
		sleep(1);
	}
}

/* TX Task */
rtems_task cantx_task(rtems_task_argument unused)
{
	UNUSED(unused);
	int cnt, ofs, i;

#ifdef ENABLE_FD_FRAMES
	if (grcan_canfd_capable(candev) == 0) {
		printf("CAN TX device not FD capable. Aborting.\n");
		exit(-1);
	}
#endif

	/* Print messages that we be sent to console */
	printf
	    ("************** MESSAGES THAT WILL BE TRANSMITTED *************\n");
	for (i = 0; i < NUM_FRAMES; i++)
		print_fdmsg(i+1, &fdmsgs[i]);
	printf
	    ("**************************************************************\n");

	printf
	    ("******************* Start of transmission ********************\n");

	txpkts = 0;
	ofs = 0;
	while (1) {
		rtems_status_code status;

		/*
		 * Protect the candev with a semaphore so that canrx_task() can
		 * ensure we are not in the GRCAN driver with it.
		 */
		status = rtems_semaphore_obtain(tx_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
		if (status != RTEMS_SUCCESSFUL) {
			printf("FAILURE: rtems_semaphore_obtain(tx_sem) failed\n");
			exit(1);
		}

		/* Blocking transmit request. Returns when all messages
		 * requested has been scheduled for transmission (not actually
		 * sent, but taken care of by driver).
		 */
		cnt = grcanfd_write(candev, &fdmsgs[ofs], NUM_FRAMES - ofs);

		rtems_semaphore_release(tx_sem);
		if (cnt > 0) {
			/* Increment statistics */
			txpkts += cnt;
			ofs += cnt;
			if (ofs >= NUM_FRAMES) {
				ofs -= NUM_FRAMES;
			}
		} else {
			sched_yield();
			printf("TX CAN TASK: write failed: %d\n", cnt);
			/* NOTE: RX task is responsible for handling the error */
		}

#ifdef WAIT_AFTER_256_MSGS
		/* Wait a bit after each 256 messages - to slow things down */
		if ((txpkts & 0x100) && !(last & 0x100)) {
			rtems_task_wake_after(4);
		}
		last = txpkts;
#endif
		/* limit the number of outstanding CAN frames.
		 * Due to the priority on CAN bus (the ID number) one node
		 * could be starved, therefore this makes sure that the number
		 * outstanding messages are not more than the CAN DMA buffer
		 * length.
		 *
		 * This is to avoid overrun in all nodes.
		 */
		while ((txpkts - rxpkts) > (0x40 - 2)) {
			rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
		}
	}

	while (1) {
		printf("Sleeping Task 2\n");
		sleep(1);
	}
}

/* CAN HELP DEBUG FUNCTIONS */
char *msgstr_type[2] = { "STD", "EXT" };
char *msgstr_rtr[2] = { "", " RTR" };
char *msgstr_fdopts[4] = { "", "", " FDF", " FDF+BRS" };

/* PRINT A CAN-FD MESSAGE FROM DATA STRUCTURE */
void print_fdmsg(int i, CANFDMsg *msg)
{
	int j;
	char data_str_buf[64];
	int ofs, printlen, left;

	if (!msg)
		return;

	if (i > 0) {
		printf("MSG[%d]: %s%s%s length: %d, id: 0x%" PRIu32 "\n", i,
		       msgstr_type[(int)msg->extended],
		       msgstr_rtr[(int)msg->rtr], 
		       msgstr_fdopts[(int)msg->fdopts], msg->len, msg->id);
	} else {
		printf("MSG: %s%s%s length: %d, id: 0x%" PRIu32 "\n",
		       msgstr_type[(int)msg->extended],
		       msgstr_rtr[(int)msg->rtr],
		       msgstr_fdopts[(int)msg->fdopts], msg->len, msg->id);
	}
	/* print data */
	if (msg->len > 0) {
		if (i > 0) {
			ofs = sprintf(data_str_buf, "MSGDATA[%d]: ", i);
		} else {
			ofs = sprintf(data_str_buf, "MSGDATA: ");
		}
		left = msg->len;
		do {
			printlen = left > 8 ? 8 : left;
			for (j = 0; j < printlen; j++) {
				ofs += sprintf(data_str_buf + ofs,
				              "0x%02" PRIu8,
				              msg->data.bytes[msg->len-left+j]);
			}
			printf("%s  ", data_str_buf);
			ofs = 0;
			for (j = 0; j < printlen; j++) {
				if (isalnum(msg->data.bytes[msg->len-left+j]))
					ofs += sprintf(data_str_buf + ofs, "%c",
					               msg->data.bytes[msg->len-left+j]);
				else
					ofs += sprintf(data_str_buf + ofs, ".");
			}
			printf("%s\n", data_str_buf);
			ofs = sprintf(data_str_buf, "        ");
			left -= printlen;
		} while (left > 0);
	}
}

/* Print statistics gathered by RX and TX tasks, also
 * print statistics from driver.
 */
void can_print_stats(void)
{
	struct grcan_stats stats;
	static int cnt = 0;
	int ret;

	/* Get stats from GRCAN driver to print */
	ret = grcan_get_stats(candev, &stats);
	if (ret) {
		printf("grcan_get_stats() failed: %d (continue anyway)\n", ret);
	} else {
		/* Got stats from driver */

#ifdef PRINT_MORE_STATS
		/* Print extra stats */
		printf("CAN0 PASSV:   %7d\n", stats.passive_cnt);
		rtems_task_wake_after(4);
		printf("CAN0 OVERRUN: %7d\n", stats.overrun_cnt);
		rtems_task_wake_after(4);
		printf("CAN0 TXLOSS:  %7d\n", stats.txloss_cnt);
		rtems_task_wake_after(4);
		if (stats.busoff_cnt) {
			printf("CAN0 BUSOFF:  %7d\n", stats.busoff_cnt);
			rtems_task_wake_after(4);
		}
#endif
		if (stats.ahberr_cnt) {
			printf("CAN0 AHB:     %7d\n", stats.ahberr_cnt);
		}
		printf("CAN0 INTS:    %7d\n", stats.ints);
		rtems_task_wake_after(4);
	}
	/* Get stats from GRCAN driver to print */
	ret = grcan_get_stats(_loop_ctx.dev, &stats);
	if (ret) {
		printf("grcan_get_stats() failed: %d (continue anyway)\n", ret);
	} else {
		/* Got stats from driver */

#ifdef PRINT_MORE_STATS
		/* Print extra stats */
		printf("CAN1 PASSV:   %7d\n", stats.passive_cnt);
		rtems_task_wake_after(4);
		printf("CAN1 OVERRUN: %7d\n", stats.overrun_cnt);
		rtems_task_wake_after(4);
		printf("CAN1 TXLOSS:  %7d\n", stats.txloss_cnt);
		rtems_task_wake_after(4);
		if (stats.busoff_cnt) {
			printf("CAN1 BUSOFF:  %7d\n", stats.busoff_cnt);
			rtems_task_wake_after(4);
		}
#endif
		if (stats.ahberr_cnt) {
			printf("CAN1 AHB:     %7d\n", stats.ahberr_cnt);
		}
		printf("CAN1 INTS:    %7d\n", stats.ints);
		rtems_task_wake_after(4);
	}
	printf("CAN RXPKTS:  %7d  (LOOPBACK: %7d)\n", rxpkts, _loop_ctx.stats.received);
	rtems_task_wake_after(4);

	/* Print only number of RX syncs every tenth time */
	if (cnt++ >= 10) {
		cnt = 0;
		printf("CAN RXSYNCS: %7d\n", rx_syncs);
		if (rx_errors > 0) {
			printf("CAN RXERRORS: %7d\n", rx_errors);
		}
	}

	printf("CAN TXPKTS:  %7d  (LOOPBACK: %7d TX-RX: %7d)\n", txpkts, _loop_ctx.stats.sent, txpkts-rxpkts);
	rtems_task_wake_after(4);
}
