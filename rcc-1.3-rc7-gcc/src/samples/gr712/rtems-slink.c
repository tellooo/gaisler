/* RTEMS SLINK master demonstration application
 *
 * based on Simple OC_CAN interface test.
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
 * The demonstration software is built to run with Gaisler Research's
 * models of IO cards. These models may have different capabilities
 * than real IO cards:
 * The testbench enumerates the IO cards like:
 * IO card in TB      IOcard #  bit 21     R/RW
 *       0               0         1        W
 *       1               1         1        W
 *       2               2         1        W
 *       3               3         1        W
 *       4               1         0        RW
 *       5               2         0        RW
 *       6               3         0        RW
 *
 * The RW IO cards accept SEQUENCE operations
 * The RW IO cards see all requests with ch /= 0 as SEQUENCE operations
 * Responses to SEQUENCE operations are on the same channel as the req.
 * The RW IO cards can generate INTERRUPT and SLAVE-WORD-SEND transfers
 * All READ responses are on channel 3
 * All INTERRUPTs are on channel 0 with data 0xFFFF
 * All SLAVE-WORD-SEND are on channel 15 with data 0
 *
 * This demonstration application performs the following operations:
 * 1.  Initializes driver
 * 2.  Enables core
 * 3.  Writes a word to each IO card
 * 4.  Reads one word from each of the three IO cards that are able to
 *     transmit data.
 * 5.  Performs a SEQUENCE of length 256
 * 6.  Performs a SEQUENCE with interleaved READs
 * 7.  Starts a SEQUENCE and attempts to start a new SEQUENCE while the
 *     first is active. The first SEQUENCE is then aborted.
 * 8.  Prints any unsolicted requests received during the run.
 * 9.  Stops (disables) the core.
 * 10. Prints statistics if they have been collected.
 * 11  Print information about usage of SEQUENCE callback function.
 * 12. Prints information about received INTERRUPT transfers.
 *
 * This demonstration application does not take the appropriate 
 * measures on parity errors. Please see the SLINK documentation
 * for suggested strategies on discovery of a parity error.
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

#include <rtems/confdefs.h>

#if defined(RTEMS_DRVMGR_STARTUP) /* if --drvmgr was given to configure */
 /* Add Timer and UART Driver for this example */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
 #endif
 #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
 #endif
 #include <drvmgr/drvmgr_confdefs.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grlib/grslink.h>

/* **** Test configuration **** */
#define SLINK_NULLWORD 0x00000000
#define SLINK_PARITY   1  /* Odd parity */
#define SLINK_QSIZE    20 /* Size of each driver receive queue */


rtems_task task1(rtems_task_argument argument);

rtems_id   Task_id[3];         /* array of task ids */
rtems_name Task_name[3];       /* array of task names */


/* Counters used in slink_demo_irqhandler */
int iocard[3] = {0, 0, 0};

/*
 * Example handler for INTERRUPT transfers, increments global counters
 */
void slink_demo_irqhandler(int slinkwrd)
{
	/* 
	 * Payload (data bits) of word can be grabbed with:
	 * SLINK_WRD_PAYLOAD(slinkwrd)
	 * Here we only care about the card number, the model
	 * IO cards have IO card # field values from 1 to 3:
	 */
	iocard[SLINK_WRD_CARDNUM(slinkwrd)-1]++;
}

/* Counters used in slink_sequence_callback below */
int callback_count[4] = {0, 0, 0, 0};

/*
 * Example callback function for completed SEQUENCE operations
 */
void slink_sequence_callback(int status)
{
	/*
	 * The argument to this function can have one of these three
	 * values (as can be seen in the IRQ handler) in grslink.c:
	 * SLINK_COMPLETED
	 * SLINK_ABORTED
	 * SLINK_PARERR
	 * SLINK_AMBAERR
	 * We count the number of occurences in the global array
	 * callback_count. The main function then reads the values
	 * in the array and reports them after all tests has completed.
	 */
	switch (status) {
	case SLINK_COMPLETED: callback_count[0]++; break;
	case SLINK_ABORTED: callback_count[1]++; break;
	case SLINK_PARERR: callback_count[2]++; break;
	case SLINK_AMBAERR: callback_count[3]++; break;
	default: 
		printf("BUG! callback received invalid value: %x\n",
		       status);
		break;
	}
}

/* initialisation */

rtems_task Init(rtems_task_argument ignored)
{
	int status;
  
	printf("******** Starting Gaisler GRSLINK test ********\n");
  
	printf("Initializing driver: ");
	status = SLINK_init(SLINK_NULLWORD, SLINK_PARITY, SLINK_QSIZE,
			    slink_demo_irqhandler,
			    slink_sequence_callback);
  
	if (status < 0) {
		printf("ERROR! SLINK driver initialization failed\n");
		exit(0);
	}
	printf("driver initialized successfully\n");
	
	Task_name[1] = rtems_build_name( 'T', 'S', 'K', 'A' );
	
	status = rtems_task_create(
		Task_name[1], 1, RTEMS_MINIMUM_STACK_SIZE * 2,
		RTEMS_DEFAULT_MODES, 
		RTEMS_DEFAULT_ATTRIBUTES, &Task_id[1]
		);

	status = rtems_task_start(Task_id[1], task1, 1);
	
	status = rtems_task_delete(RTEMS_SELF);
}


/* Helper function */
/*
 * This function generates the 'channel' argument to SLINK_[read,write]()
 * so that it fits with the IO card model addressing described at the top
 * of this file.
 */
int gen_channel(int iocard, int channo)
{
	int iocard_n_spare;
	
	if (iocard < 4)
		iocard_n_spare = (iocard << 1) | 1;
	else
		iocard_n_spare = (iocard-3) << 1;
	
	return (iocard_n_spare << 4) | channo;
}

	
rtems_task task1(rtems_task_argument unused) 
{
	int retval;
	int i;
	int channo;
	int readdata;
	int compwrds;
	int expected;
	int a[256], b[256];
	SLINK_stats *stats;
	
	printf("Starting test task...\n");
	
	
	/***************************************************************/ 
	/* Enable core                                                 */
	/***************************************************************/
	SLINK_start();
	

	/***************************************************************/ 
	/* Write one word to each IO card, IO card n gets written with */
	/* data 'n'.                                                   */
	/***************************************************************/	
	printf("\tWriting one word to each IO card\n");
	channo = 0;
	for (i = 0; i < 4; i++) { /* Method 1: Poll on SLINK_write */
		while (SLINK_write(i, gen_channel(i, channo)))
			;
	}
	for (i = 4; i < 7; i++) { /* Method 2: Make use of hwstatus */
		while(!SLINK_STS_TRANSFREE(SLINK_hwstatus()))
			;
		SLINK_write(i, gen_channel(i, channo));
	}
	

	/***************************************************************/ 
	/* Read one word from each IO card capable of transmission     */
	/***************************************************************/
	printf("\tReading one word from each card capable of transmission\n");
	for (i = 4; i < 7; i++) {
		while(!SLINK_STS_TRANSFREE(SLINK_hwstatus()))
			;
		retval = SLINK_read(i, gen_channel(i, channo), &readdata);
		switch (retval) {
		case -SLINK_PARERR: 
			printf("Core detected parity error on read %d!\n", i);
			break;
		case -SLINK_ROV:
			printf("Core got receive overflow on read %d\n", i);
			break;
		case -SLINK_QFULL:
			printf("SLINK_read reported QFULL on read %d!\n", i);
			break;
		case 0: /* Success response */ break;
		default:
			printf("BUG! Read reported unexpected %d for read %d\n",
			       retval, i);
			break;
		}
		if (SLINK_WRD_CARDNUM(readdata) != (i-3))
			printf("Expected data from IO card %d, "
			       "received from %d\n",
			       i, SLINK_WRD_CARDNUM(readdata));
		if (SLINK_WRD_CHAN(readdata) != 3)
			printf("IO card did not respond on channel 3, i = %d\n", i);
		if (SLINK_WRD_PAYLOAD(readdata) != i) 
			printf("Expected payload = 0x%x, received 0x%x\n",
			       i, SLINK_WRD_PAYLOAD(readdata));
	}    
	
	/***************************************************************/ 
	/* Perform a SEQUENCE                                          */
	/***************************************************************/
	printf("\tPerforming SEQUENCE\n");
	channo = 4;
	
	/* Build array for SEQUENCE operation */
	for (i = 0; i < 256; i++)
		a[i] = (gen_channel(i % 3 + 4, channo) << 16) | (i % 236 + 20);
	
  
	/* Perform a SEQUENCE of length 256 */
	SLINK_seqstart(a, b, 256, channo, 0);
	
	while (SLINK_seqstatus() == SLINK_ACTIVE)
		;
	switch (SLINK_seqstatus()) {
	case SLINK_COMPLETED: /* Successful! */ break; 
	case SLINK_PARERR: 
		printf("Core detected parity error during first sequence!\n");
		break; 
	case SLINK_AMBAERR: 
		printf("Core detected AMBA error during first sequence!\n"); 
		break; 
	case SLINK_ABORTED:
		printf("First sequence was aborted!\n"); 
		break; 
	default:
		printf("BUG! SLINK_seqstatus() returned unknown value!\n");
		break;
	}
	/* The SEQUENCE should complete and SLINK_seqwrds() should return 0 */
	if (SLINK_seqwrds() != 0) 
		printf("BUG! SLINK_seqwrds() did not return 0 after completed"
		       " first SEQUENCE\n");
  
	/* Check a array and responses in b array */
	for (i = 0; i < 256; i++) {
		expected = ((gen_channel(i % 3 + 4, channo) << 16) | 
			    (i % 236 + 20));
		if (expected != a[i]) 
			printf("Element a[%d] changed in first SEQUENCE!\n", i);
		/* IO card models respond with same data on the same channel */
		if (expected != b[i])
			printf("Element b[%d] has unexpected value %x\n", 
	     i, b[i]); 
	}
	
	/***************************************************************/ 
	/* Perform a SEQUENCE with interleaved READs                   */
	/***************************************************************/
	printf("\tPerforming SEQUENCE with interleaved READs\n");
	/* Perform a SEQUENCE of length 256 and interleave READs */
	for (i = 0; i < 256; i++)
		a[i] = (gen_channel(4, channo) << 16) | (i % 236 + 20);
	
	SLINK_seqstart(a, b, 256, channo, 0);
	
	channo = 0;
	while (SLINK_seqstatus() == SLINK_ACTIVE)
		for (i = 5; i < 7; i++) {
			while(!SLINK_STS_TRANSFREE(SLINK_hwstatus()))
				;
			retval = SLINK_read(i, gen_channel(i, channo), 
					    &readdata);
			if (retval)
				printf("Read returned unexpected %d\n", retval);
      
			if (SLINK_WRD_CARDNUM(readdata) != (i-3))
				printf("Expected data from IO card %d, "
				       "received from %d\n", i, 
				       SLINK_WRD_CARDNUM(readdata));
			if (SLINK_WRD_CHAN(readdata) != 3)
				printf("IO card did not respond on channel 3, "
				       "i = %d\n", i);
			if (SLINK_WRD_PAYLOAD(readdata) != i) 
				printf("Expected payload = 0x%x, got 0x%x\n",
				       i, SLINK_WRD_PAYLOAD(readdata));
		}
  
	if (SLINK_seqstatus() != SLINK_COMPLETED) 
		printf("SLINK_seqstatus() reports %d after SEQUENCE with "
		       "interleaved READs\n", SLINK_seqstatus());
	if (SLINK_seqwrds() != 0)
		printf("SLINK_seqwrds() did not report 0 after completed "
		       "SEQUENCE with interleaved reads\n");
	/* Check all 256 elements again */
	channo = 4;
	for (i = 0; i < 256; i++) {
		expected = ((gen_channel(4, channo) << 16) | (i % 236 + 20));
		if (expected != a[i]) 
			printf("Element a[%d] changed in SEQUENCE with "
			       "interleaved READs!\n", i);
		/* IO card models respond with same data on the same channel */
		if (expected != b[i])
			printf("Element b[%d] has unexpected value %x after "
			       "SEQUENCE with interleaved READs \n", i, b[i]);
	} 

	/***************************************************************/
	/* Start a SEQUENCE and a try to start a new SEQUENCE before   */
	/* the first has completed. Then abort the first SEQUENCE.     */
	/***************************************************************/
	printf("\tStarting two SEQUENCEs, aborting the first\n");
	channo = 5;
	/* Need to rebuild a array with new channo since IO card models
	   respond on the same channel */
	for (i = 0; i < 256; i++)
		a[i] = (gen_channel(i % 3 + 4, channo) << 16) | (i % 236 + 20);
	
	retval = SLINK_seqstart(a, b, 256, channo, 0);
	if (retval)
		printf("ERROR starting sequence!\n");
	
	retval = SLINK_seqstart(a, b, 100, channo, 0);
	if (retval == 0)
		printf("New SEQUENCE could be started while core was busy!\n");
	
	/* Wait until ten words have been completed */
	while (SLINK_STS_SI(SLINK_hwstatus()) < 10)
		;
	
	SLINK_seqabort();
	
	while (SLINK_seqstatus() == SLINK_ACTIVE)
	  ;
	
	if (SLINK_seqstatus() != SLINK_ABORTED)
		printf("SEQUENCE was not aborted!\n");
	
	/* Check number of completed words */
	compwrds = SLINK_seqwrds();
	if (compwrds <= 0)
		printf("Unexpected value, %d, of SLINK_seqwrds() after "
		       " aborted SEQUENCE\n", compwrds);
  
	/* Check integrity of a array */
	for (i = 0; i < 256; i++) {
		expected = ((gen_channel(i % 3 + 4, channo) << 16) | 
			    (i % 236 + 20));
		if (expected != a[i]) 
			printf("Element a[%d] changed in aborted SEQUENCE!\n",
			       i);
	}
  
	/* Check that number of completed words correspond to the number
	   of changed elements in the b array. */
	for (i = 0; i < compwrds; i++) {
		expected = ((gen_channel(i % 3 + 4, channo) << 16) | 
			    (i % 236 + 20));
		if (expected != b[i]) 
			printf("Element b[%d] has unexpected value in "
			       " aborted SEQUENCE!\n", i);
	}
	/* Check that the other elements were left unchanged */
	channo = 4;
	for (i = compwrds; i < 256; i++) {
		expected = ((gen_channel(4, channo) << 16) | 
			    (i % 236 + 20));
		if (expected != b[i]) 
			printf("Element b[%d] has unexpected value in "
			       " aborted SEQUENCE!\n", i);
	}


	/***************************************************************/ 
	/* Check if the core detected unsolicited requests             */
	/***************************************************************/
	printf("\tUnsolicited requests:\n");
	for (i = 0; i < SLINK_NUMQUEUES; i++)
		printf("\tNumber of elements in queue for IO card %d: %d\n",
		       i, SLINK_queuestatus(i));
	
	for (i = 0; i < SLINK_NUMQUEUES; i++)
	  while (SLINK_dequeue(i, &readdata) != -1)
		  printf("\tReceived unsolicited request from "
			 "card %d, channel %d with data 0x%x\n",
			 SLINK_WRD_CARDNUM(readdata) + 3,
			 SLINK_WRD_CHAN(readdata),
			 SLINK_WRD_PAYLOAD(readdata));
 

	/***************************************************************/
	/* Stop core                                                   */
	/***************************************************************/
	printf("\tStopping core\n");
	SLINK_stop();
	

	/***************************************************************/
	/* Print statistics                                            */
	/***************************************************************/
	printf("\tStatistics:\n");
	stats = SLINK_statistics();
	if (stats == NULL)
		printf("\tNo statistics collected\n");
	else
		printf("\tNumber of parity errors: %d\n"
		       "\tNumber of receive overflows: %d\n"
		       "\tNumber of completed READs: %d\n"
		       "\tNumber of performed WRITEs: %d\n"
		       "\tNumber of started SEQUENCES: %d\n"
		       "\tNumber of completed SEQUENCES: %d\n"
		       "\tNumber of INTERRUPT transfers: %d\n"
		       "\tNumber of lost words due to full queue: %d\n",
		       stats->parerr, stats->recov, stats->reads,
		       stats->writes, stats->sequences, stats->seqcomp,
		       stats->interrupts, stats->lostwords);
	

	/***************************************************************/
	/* Print information collected by SEQUENCE callback function   */
	/***************************************************************/
	printf("\tSEQUENCE callback function was called:\n"
	       "\t%d times with status SLINK_COMPLETED, %s\n"
	       "\t%d times with status SLINK_ABORTED, %s\n"
	       "\t%d times with status SLINK_PARERR, %s\n"
	       "\t%d times with status SLINK_AMBAERR, %s\n",
	       callback_count[0], callback_count[0] == 2 ? "OK" : "ERROR",
	       callback_count[1], callback_count[1] == 1 ? "OK" : "ERROR",
	       callback_count[2], callback_count[2] == 0 ? "OK" : "ERROR",
	       callback_count[3], callback_count[3] == 0 ? "OK" : "ERROR");
	
	/***************************************************************/
	/* Print IRQ information                                       */
	/***************************************************************/
	printf("\tINTERRUPT transfers:\n");
	for (i = 0; i < 3; i++) 
		printf("\tNumber of INTERRUPT transfers from IO card %d: %d\n",
		       i+4, iocard[i]);
	
	printf("Test completed, no more output will come from "
	       "this test application\n");	
}

