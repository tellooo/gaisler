/* Simple SatCAN FPGA wrapper interface test based on OC_CAN interface test.
 *
 * This sample application is tailored for GR712RC FPGA.
 *
 * --------------------------------------------------------------------------
 *  --  This file is a part of GAISLER RESEARCH source code.
 *  --  Copyright (C) 2008, Gaisler Research AB - all rights reserved.
 *  --
 *  -- ANY USE OR REDISTRIBUTION IN PART OR IN WHOLE MUST BE HANDLED IN
 *  -- ACCORDANCE WITH THE GAISLER LICENSE AGREEMENT AND MUST BE APPROVED
 *  -- IN ADVANCE IN WRITING.
 *  --
 *  -- BY DEFAULT, DISTRIBUTION OR DISCLOSURE IS NOT PERMITTED.
 *  -------------------------------------------------------------------------- 
 *
 */

#include <rtems.h>

#define CONFIGURE_INIT
#include <bsp.h> /* for device driver prototypes */
rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_NULL_DRIVER 1
#define CONFIGURE_MAXIMUM_TASKS             8
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 16
#define CONFIGURE_INIT_TASK_PRIORITY	100
#define CONFIGURE_MAXIMUM_DRIVERS 16

/* Configure RTEMS Kernel */
#include <rtems/confdefs.h>

/* Configure Driver manager */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_OCCAN
#if defined(RTEMS_DRVMGR_STARTUP) /* if --drvmgr was given to configure */
 /* Add Timer and UART Driver for this example */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
 #endif
 #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
 #endif
#endif
#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <grlib/satcan.h>
#include <grlib/canmux.h>

#include <grlib/occan.h>
#include "../occan_lib.h"

#undef ENABLE_NETWORK
#include "../config.c"


/* Messages send SatCAN -> OC, not including SYNC */
#define SATCAN_TO_OC_MESSAGES 5
/* Messages send OC -> SatCAN  */
#define OC_TO_SATCAN_MESSAGES 5 

/* Use 8K DMA area? */
/* #define SATCAN_8K_DMA_AREA */

/* OCCAN register values for system running at 48 MHz */
#define BTR0 0x01
#define BTR1 0x27

rtems_task task1(rtems_task_argument argument);
rtems_task task2(rtems_task_argument argument);
rtems_id sync1;
rtems_id sync2;

/*extern int errno;*/

#define MAXIMUM(x,y) ((x) > (y) ? x : y)

rtems_id   Task_id[3];         /* array of task ids */
rtems_name Task_name[3];       /* array of task names */

/* Callback functions used for SatCAN IRQs */

unsigned int ahb_irq_cnt = 0;
unsigned int pps_irq_cnt = 0;
unsigned int marker_irq_cnt[5] = {0, 0, 0, 0, 0};
unsigned int sync_irq_cnt = 0;
unsigned int can_irq_cnt = 0;

#define MINIMUM(x,y) (x < y ? x : y)

#define IVEC_ARRAY_SIZE 100

#define TIMEOUT_FOR_OBTAIN 100

unsigned int ivecarray[IVEC_ARRAY_SIZE];

unsigned int cancritical = 0;

void ahb_irq_cb(void) { ahb_irq_cnt++; }
void pps_irq_cb(void) { pps_irq_cnt++; }
void m5_irq_cb(void) { marker_irq_cnt[4]++; }
void m4_irq_cb(void) { marker_irq_cnt[3]++; }
void m3_irq_cb(void) { marker_irq_cnt[2]++; }
void m2_irq_cb(void) { marker_irq_cnt[1]++; }
void m1_irq_cb(void) { marker_irq_cnt[4]++; }
void sync_irq_cb(void) { sync_irq_cnt++; }
void can_irq_cb(unsigned int fifo) {
	static int irqindex = 0;
	can_irq_cnt++;
	if (irqindex < IVEC_ARRAY_SIZE) {
		ivecarray[irqindex] = fifo;
		irqindex++;
	}
	if (fifo == SATCAN_IRQ_CRITICAL) {
		printk("IRQ! CAN Critical error\n");
		cancritical++;
	} else if (fifo == SATCAN_IRQ_EOD2) {
		rtems_semaphore_release(sync2);
	} else if (fifo != SATCAN_IRQ_EOD1) {
		printk("IRQ! fifo value is 0x%x\n", fifo);
	}
}

static satcan_config satcan_conf = {
	.nodeno = 0,
	.dps = 0,
	.ahb_irq_callback = ahb_irq_cb,
	.pps_irq_callback = pps_irq_cb,
	.m5_irq_callback = m5_irq_cb,
	.m4_irq_callback = m4_irq_cb,
	.m3_irq_callback = m3_irq_cb,
	.m2_irq_callback = m2_irq_cb,
	.m1_irq_callback = m1_irq_cb,
	.sync_irq_callback = sync_irq_cb,
	.can_irq_callback = can_irq_cb,
};


/* ========================================================= 
   initialisation */

rtems_task Init(
        rtems_task_argument ignored
)
{
        int fd;
	char driver_name[20];

	system_init();

	rtems_semaphore_create(
                        rtems_build_name('S', 'Y', 'N', '1'), 
                        0, 
                        RTEMS_FIFO | RTEMS_SIMPLE_BINARY_SEMAPHORE | RTEMS_NO_INHERIT_PRIORITY | \
                        RTEMS_NO_PRIORITY_CEILING, 
                        0, 
                        &(sync1));

	rtems_semaphore_create(
                        rtems_build_name('S', 'Y', 'N', '1'), 
                        0, 
                        RTEMS_FIFO | RTEMS_SIMPLE_BINARY_SEMAPHORE | RTEMS_NO_INHERIT_PRIORITY | \
                        RTEMS_NO_PRIORITY_CEILING, 
                        0, 
                        &(sync2));
                        
	printf("******** Starting Gaisler SatCAN test for GR712RC ********\n");
        
	printf("Registering CAN_MUX driver: ");
	if (canmux_register()) {
		printf("ERROR! CAN_MUX driver could not be registered\n");
		exit(0);
	}
	printf("CAN_MUX driver registered successfully\n");
	
	printf("Configuring CAN_MUX: ");
	strcpy(driver_name, "/dev/canmux");
	fd = open(driver_name, O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s\n", driver_name);
		exit(0);
	}
	/* SatCAN on bus B, OC-CAN1 on bus A */
	if (ioctl(fd, CANMUX_IOC_BUSA_OCCAN1) || ioctl(fd, CANMUX_IOC_BUSB_SATCAN)) {
		printf("CAN_MUX ioctl failed\n");
		exit(0);
	}
	printf("CAN_MUX configured\n");

	printf("Registering SatCAN driver: ");
	if (satcan_register(&satcan_conf)) {
		printf("ERROR! SatCAN driver could not be registered\n");
		exit(0);
	}
	printf("SatCAN driver registered successfully\n");
	
        Task_name[1] = rtems_build_name( 'T', 'S', 'K', 'A' );
        Task_name[2] = rtems_build_name( 'T', 'S', 'K', 'B' );
                
        rtems_task_create(
                Task_name[1], 1, RTEMS_MINIMUM_STACK_SIZE * 2,
                RTEMS_DEFAULT_MODES, 
                RTEMS_DEFAULT_ATTRIBUTES, &Task_id[1]
                );

        rtems_task_create( 
                Task_name[2], 1, RTEMS_MINIMUM_STACK_SIZE * 2,
                RTEMS_DEFAULT_MODES,
                RTEMS_DEFAULT_ATTRIBUTES, &Task_id[2]
                );
        
       
        /* Starting receiver first */

        rtems_task_start(Task_id[2], task2, 2);

	rtems_task_start(Task_id[1], task1, 1);

        rtems_task_delete(RTEMS_SELF);
}


/* =========================================================  
   task1 */

/* Helper function that prints a CAN critical message */
void show_cancritical(int fd)
{
	int i;
	int ret;
	satcan_msg msg;
	
	msg.header[0] = 0x99;
	msg.header[1] = 0;
	
	ret = read(fd, &msg, sizeof(satcan_msg));
	if (ret != sizeof(satcan_msg)) {
		printf("ERROR while reading CAN critical message\n");
		return;
	}

	printf("Contents of CAN critical message:\n");
	for (i = 0; i < SATCAN_HEADER_SIZE; i++)
		printf("header[%d]:\t0x%x\n", i, msg.header[i]);
	for (i = 0; i < SATCAN_PAYLOAD_SIZE; i++)
		printf("payload[%d]:\t0x%x\n", i, msg.payload[i]);
	printf("Interpretation of values:\n");
	printf("CAN critical error field: \n");
	printf("\tCAN Error State: 0x%x\n", (msg.payload[0] >> 1) & 0x3);
	printf("\tRx Over: %s\n", msg.payload[0] & 0x8 ? "Set" : "Not Set");
	printf("\tW.D Time Out: %s\n", msg.payload[0] & 0x10 ? "Set" : "Not Set");
	printf("\tGuard Window Violation: %s\n", msg.payload[0] & 0x80 ? "Set" : "Not Set");
	
	printf("CAN A Error:\n");
	printf("\tCRC Error: %s\n", msg.payload[1] & 0x1 ? "Set" : "Not Set");
	printf("\tForm. Error: %s\n", msg.payload[1] & 0x2 ? "Set" : "Not Set");
	printf("\tAck. Error: %s\n", msg.payload[1] & 0x4 ? "Set" : "Not Set");
	printf("\tStuff Error: %s\n", msg.payload[1] & 0x8 ? "Set" : "Not Set");
	printf("\tBit Error: %s\n", msg.payload[1] & 0x10 ? "Set" : "Not Set");
	printf("\tArb. Loss: %s\n", msg.payload[1] & 0x20 ? "Set" : "Not Set");
	printf("\tOverLoad: %s\n", msg.payload[1] & 0x40 ? "Set" : "Not Set");
	printf("\tTx_Error_Cnt(0): %s\n", msg.payload[1] & 0x80 ? "Set" : "Not Set");

	printf("CAN B Error:\n");
	printf("\tCRC Error: %s\n", msg.payload[4] & 0x1 ? "Set" : "Not Set");
	printf("\tForm. Error: %s\n", msg.payload[4] & 0x2 ? "Set" : "Not Set");
	printf("\tAck. Error: %s\n", msg.payload[4] & 0x4 ? "Set" : "Not Set");
	printf("\tStuff Error: %s\n", msg.payload[4] & 0x8 ? "Set" : "Not Set");
	printf("\tBit Error: %s\n", msg.payload[4] & 0x10 ? "Set" : "Not Set");
	printf("\tArb. Loss: %s\n", msg.payload[4] & 0x20 ? "Set" : "Not Set");
	printf("\tOverLoad: %s\n", msg.payload[4] & 0x40 ? "Set" : "Not Set");
	printf("\tTx_Error_Cnt(0): %s\n", msg.payload[4] & 0x80 ? "Set" : "Not Set");
}
      

rtems_task task1(rtems_task_argument unused) 
{
	int i;
	int j;
	int fd;
	int val;
	int ret;
	satcan_msg msgs[10];
	satcan_regmod regmod;
	rtems_status_code status;

	printf("Starting task 1\n");

	fd = open("/dev/satcan", O_RDWR);
	if (fd < 0) {
		printf("Failed to open /dev/satcan\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
	
	/* Read driver DMA mode */
	ioctl(fd, SATCAN_IOC_GET_DMA_MODE, &ret);
	if (ret != SATCAN_DMA_MODE_SYSTEM) {
		printf("Task1: ERROR: Driver DMA mode was not DMA_MODE_SYSTEM\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	/* Enable DMA TX channel 1 */
	if (ioctl(fd, SATCAN_IOC_EN_TX1_DIS_TX2)) {
		printf("Task1: ERROR: Failed to enable DMA TX channel 1\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

#ifdef SATCAN_8K_DMA_AREA
	/* Enable DMA area with 8K messages */
	if (ioctl(fd, SATCAN_IOC_DMA_8K)) {
		printf("Task1: ERROR: Failed to enable DMA area with 8K messages\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
#endif

	/* Enable override, CAN B and TX */;
	for (i = 0; i < 3; i++) {
		/* Configuration message */
		msgs[i].header[0] = 0xE0;
		msgs[i].header[1] = 0;
		msgs[i].header[2] = 0x81;
		msgs[i].header[3] = 0xFF;
		
		switch (i) {
			/* Enable override */
		case 0: msgs[i].payload[0] = 15; break;
			/* Enable CAN B */
		case 1: msgs[i].payload[0] = 5; break;
			/* Enable TX */
		case 2: msgs[i].payload[0] = 6; break;
		default: break;
		}
		
		for (j = 1; j < SATCAN_PAYLOAD_SIZE; j++)
			msgs[i].payload[j] = 0;
	}
	ret = write(fd, msgs, 3*sizeof(satcan_msg));
	if (ret != 3*sizeof(satcan_msg)) {
		printf("Task1: Write of messages failed, ret = %d\n", ret);
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	/* Configuring PLL */
	regmod.reg = SATCAN_PLL_RST;
	regmod.val = 1;
	if (ioctl(fd, SATCAN_IOC_SET_REG, &regmod)) {
		printf("Reset PLL failed\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
	regmod.reg = SATCAN_PLL_CMD;
	regmod.val = 1;
	if (ioctl(fd, SATCAN_IOC_SET_REG, &regmod)) {
		printf("Task1: Failed to choose 1 MHz clock as PLL output\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	/* Enable sync pulse and sync message */
	regmod.reg = SATCAN_CMD1;
	regmod.val = 0x30;
	if (ioctl(fd, SATCAN_IOC_OR_REG, &regmod)) {
		printf("Task1: Failed to enable sync pulse and sync message\n");
		rtems_task_delete(RTEMS_SELF);
 		return;
 	}

	/* Set CAN Critical No-masking */
	msgs[0].payload[0] = 9;
	ret = write(fd, msgs, sizeof(satcan_msg));
	if (ret != sizeof(satcan_msg)) {
		printf("Task1: Write CAN Critical No-masking failed\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
	
	/* Reading CMD1 register */
	regmod.reg = SATCAN_CMD1;
	if (ioctl(fd, SATCAN_IOC_GET_REG, &regmod)) {
		printf("Task1: Failed to read CMD1 register\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
#ifdef SATCAN_8K_DMA_AREA
	if (regmod.val != 0x30) {
		printf("Task1: Command register has unxecpected value 0x%08x\n",
		       regmod.val);
		rtems_task_delete(RTEMS_SELF);
		return;
	}
#else
	if (regmod.val != 0x31) {
		printf("Task1: Command register has unxecpected value 0x%08x\n",
		       regmod.val);
		rtems_task_delete(RTEMS_SELF);
		return;
	}
#endif

	/* Change DMA mode to DMA_MODE_USER */
	val = SATCAN_DMA_MODE_USER;
	if (ioctl(fd, SATCAN_IOC_SET_DMA_MODE, &val)) {
		printf("Task1: Failed to set DMA mode\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
	ioctl(fd, SATCAN_IOC_GET_DMA_MODE, &ret);
	if (ret != SATCAN_DMA_MODE_USER) {
		printf("Task1: ERROR: Driver DMA mode did not change\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	printf("Task1: Transmitting %d messages to OCCAN core:\n", SATCAN_TO_OC_MESSAGES);
	for (i = 0; i < SATCAN_TO_OC_MESSAGES; i++) {
		msgs[i].header[0] = 0x40;
		msgs[i].header[1] = 0;
		msgs[i].header[2] = 0x88;
		msgs[i].header[3] = 0;
		for (j = 0; j < 8; j++) 
			msgs[i].payload[j] = i+j;
	}
	ret = write(fd, msgs, SATCAN_TO_OC_MESSAGES*sizeof(satcan_msg));
	if (ret != SATCAN_TO_OC_MESSAGES*sizeof(satcan_msg)) {
		printf("Write of messages failed\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	/* Start cycle time counter */
	regmod.reg = SATCAN_START_CTC;
	regmod.val = 1;
	if (ioctl(fd, SATCAN_IOC_SET_REG, &regmod)) {
		printf("Task1: Failed to start cycle time counter\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	
	/* Enable DMA TX channel 2 */
	val = SATCAN_DMA_ENABLE_TX2;
	if (ioctl(fd, SATCAN_IOC_ACTIVATE_DMA, &val)) {
		printf("Task1:\t ERROR: Could not enable DMA TX channel 2\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
	/* 
	 * The interrupt callback releases sync2 when it receives an
	 * EOD2 interrupt
	 */
	do {
		status = rtems_semaphore_obtain(sync2, RTEMS_WAIT, TIMEOUT_FOR_OBTAIN);
		switch (status) {
		case RTEMS_UNSATISFIED:
			printf("Task1: ERROR: semaphore not available\n");
			rtems_task_delete(RTEMS_SELF);
			return;
		case RTEMS_TIMEOUT:
			if (cancritical) {
				printf("Task1: ERROR: CAN critical error\n");
				show_cancritical(fd);
				printf("Task1: Skipping remaining tests..\n");
				goto end;
			}
			break;
		case RTEMS_OBJECT_WAS_DELETED:
			printf("Task1: ERROR: semaphore deleted while waiting\n");
			rtems_task_delete(RTEMS_SELF);
			return;
		case RTEMS_INVALID_ID:
			printf("Task1: ERROR: invalid semaphore id\n");
			rtems_task_delete(RTEMS_SELF);
			return;
		default: break;
		}
			
	} while (status != RTEMS_SUCCESSFUL);

	printf("Task1: Sent %d messages to OCCAN core\n", SATCAN_TO_OC_MESSAGES);
	
	/* Disable DMA TX channel 2 */
	val = SATCAN_DMA_ENABLE_TX2;
	if (ioctl(fd, SATCAN_IOC_DEACTIVATE_DMA, &val)) {
		printf("Task1:\t ERROR: Could not disable DMA TX channel 2\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	/* Read Dma Channel Enable register */
	regmod.reg = SATCAN_DMA;
	if (ioctl(fd, SATCAN_IOC_GET_REG, &regmod)) {
		printf("Task1: Failed to read CMD1 register\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}
	if (regmod.val != 0x01) {
		printf("Task1: DMA channel enable register value: 0x%08x\n", regmod.val);
		rtems_task_delete(RTEMS_SELF);
		return;
	}

	/* 
	 * The task controlling OCCAN loops until it has received all expected 
	 * messages from the SatCAN core. The OCCAN core is then instructed to
	 * send a specified number of messsages. After this has been done the 
	 * sync1 semaphore is released.
	 */ 
	do {
		status = rtems_semaphore_obtain(sync1, RTEMS_WAIT, TIMEOUT_FOR_OBTAIN);
		switch (status) {
		case RTEMS_UNSATISFIED:
			printf("Task1: ERROR: semaphore not available\n");
			rtems_task_delete(RTEMS_SELF);
			return;
		case RTEMS_TIMEOUT:
			if (cancritical) {
				printf("Task1: ERROR: CAN critical error\n");
				show_cancritical(fd);
				printf("Task1: Skipping remaining tests..\n");
				goto end;
			}
			break;
		case RTEMS_OBJECT_WAS_DELETED:
			printf("Task1: ERROR: semaphore deleted while waiting\n");
			rtems_task_delete(RTEMS_SELF);
			return;
		case RTEMS_INVALID_ID:
			printf("Task1: ERROR: invalid semaphore id\n");
			rtems_task_delete(RTEMS_SELF);
			return;
		default: break;
		}
			
	} while (status != RTEMS_SUCCESSFUL);
	
	
	printf("Task1: Reading %d messages from OCCAN core\n", OC_TO_SATCAN_MESSAGES);
	for (i = 0; i < OC_TO_SATCAN_MESSAGES; i++) {
		msgs[i].header[0] = 0x40+i;
		msgs[i].header[1] = 0;
	}
	val = OC_TO_SATCAN_MESSAGES*sizeof(satcan_msg);
	if ((ret = read(fd, msgs, val)) != val)
		printf("Task1: ERROR! Read returned %d\n", ret);
	
	for (i = 0; i < OC_TO_SATCAN_MESSAGES; i++) { 
		for (j = 0; j < SATCAN_HEADER_SIZE; j++)
			if (msgs[i].header[0] != 0x40 + i)
				printf("Task1: ERROR! Mismatch on header %d\n", i);
		for (j = 0; j < SATCAN_PAYLOAD_SIZE; j++)
			if (msgs[i].payload[j] != (i+j))
				printf("Task1: ERROR!: Mismatch on data byte %d, "
				       "message %d, expected 0x%x, got 0x%x\n", 
				       j, i, i+j, msgs[i].payload[j]);
	}
	printf("Task1: Read %d messages from OCCAN core\n", OC_TO_SATCAN_MESSAGES);
	
 end:
	printf("Task1 statistics:\n");
	printf("Number of AHB interrupts detected: %d\n", ahb_irq_cnt);
	printf("Number of PPS interrupts detected: %d\n", pps_irq_cnt);
	for (i = 0; i < 5; i++)
		printf("Number of Time marker %d interrupts detected: %d\n",
		       i+1, marker_irq_cnt[i]);
	printf("Number of SYNC interrupts detected: %d\n", sync_irq_cnt);
	printf("Number of CAN interrupts detected: %d\n", can_irq_cnt);
	printf("\tInterrupt vectors reported to CAN IRQ callback:\n");
	for (i = 0; i < MINIMUM(can_irq_cnt, IVEC_ARRAY_SIZE); i++)
		printf("Call %d interrupt vector 0x%02x / %d\n",
		       i+1, ivecarray[i], ivecarray[i]);

	if (cancritical) {
		printf("Received %d CAN critical interrupts, last message:\n",
		       cancritical);
		show_cancritical(fd);
	}

	close(fd);

	printf("Task1: Done..\n");
	rtems_task_delete(RTEMS_SELF);
}

/* ========================================================= 
   task2 */

#define TSK2_RX_LEN 32
#define TSK2_TX_LEN 8

rtems_task task2(rtems_task_argument unused) 
{
	occan_t chan;
	CANMsg msgs[MAXIMUM(SATCAN_TO_OC_MESSAGES, OC_TO_SATCAN_MESSAGES)];
	int i, j, cnt, msgcnt;
	struct occan_afilter afilt;
	
	printf("Starting task 2\n");
	
	/* open device */
	chan=occanlib_open("/dev/occan1");

	if (!chan) {
		printf("Failed to occan device driver 0\n");
		rtems_task_delete(RTEMS_SELF);
		return;
	}

        /* Set speed */ 
	occanlib_set_btrs(chan,BTR0,BTR1);
  
	/* Set buf len */; 
	occanlib_set_buf_length(chan,TSK2_RX_LEN,TSK2_TX_LEN);
	
	/* Set blk mode */ 
	occanlib_set_blocking_mode(chan,0,1);
	
	/* Set filter to accept all */
	afilt.single_mode = 1;
	afilt.code[0] = 0x00;
	afilt.code[1] = 0x00;
	afilt.code[2] = 0x00;
	afilt.code[3] = 0x00;
	afilt.mask[0] = 0xff; /* don't care */
	afilt.mask[1] = 0xff;
	afilt.mask[2] = 0xff;
	afilt.mask[3] = 0xff;
	occanlib_set_filter(chan,&afilt);
	
	/* Start link */ 
	occanlib_start(chan);
	

	/* Get  messages with id 0x40 from SatCAN core */
	msgcnt = 0;
	printf("Task2: Receiving messages\n");
	while(msgcnt < SATCAN_TO_OC_MESSAGES) {
		/* blocking read */
		cnt = occanlib_recv_multiple(chan, msgs, SATCAN_TO_OC_MESSAGES);
		if (cnt > 0) {
			for (i = 0; i < cnt; i++) {
				//printf("RX MSG : %d\n",msgs[i].id);
				if (msgs[i].id == 0x40) {
					for (j = 0; j < SATCAN_PAYLOAD_SIZE; j++)
						if (msgs[i].data[j] != (msgcnt + j)) {
							printf("ERROR: Wrong data byte %d for message %d, "
							       "expected 0x%x, got 0x%x\n", j, msgcnt,
							       msgcnt + j, msgs[i].data[j]);
						}
					msgcnt++;
				} else if (msgs[i].id != 0x20) {
					printf("Task2: ERROR: Unrecognized message id: 0x%x\n", msgs[i].id);
				}
			}
		} else if (cnt < 0) {
			printf("Task2: Experienced RX error\n");
		}
	}
	printf("Task2: Received %d messages from SatCAN core\n", SATCAN_TO_OC_MESSAGES);

	
	printf("Task2: Sending %d messages to SatCAN core\n", OC_TO_SATCAN_MESSAGES);
	for (i = 0; i < OC_TO_SATCAN_MESSAGES; i++) {
		msgs[i].id = 0x40+i;
		msgs[i].extended = 0;
		msgs[i].rtr = 0;
		msgs[i].sshot = 0;
		msgs[i].len = 8;
		for (j = 0; j < 8; j++)
			msgs[i].data[j] = i+j;
	}
	if (occanlib_send_multiple(chan, msgs, OC_TO_SATCAN_MESSAGES) != OC_TO_SATCAN_MESSAGES)
		printf("Task2: ERROR! Failed to send messages\n");
	else
		printf("Task2: Sent %d messages to SatCAN core\n", OC_TO_SATCAN_MESSAGES);

	rtems_semaphore_release(sync1);

	sleep(10);

	occanlib_stop(chan);

	printf("Task2: Done..\n");
	rtems_task_delete(RTEMS_SELF);
}
