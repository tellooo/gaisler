/*
  This file contains the GRASCS demo software
  
  -------------------------------------------------------------------------
  --  This file is a part of GAISLER RESEARCH source code.
  --  Copyright (C) 2008, Gaisler Research AB - all rights reserved.
  --
  -- ANY USE OR REDISTRIBUTION IN PART OR IN WHOLE MUST BE HANDLED IN
  -- ACCORDANCE WITH THE GAISLER LICENSE AGREEMENT AND MUST BE APPROVED
  -- IN ADVANCE IN WRITING.
  --
  -- BY DEFAULT, DISTRIBUTION OR DISCLOSURE IS NOT PERMITTED.
  -------------------------------------------------------------------------- 
  
  The demostration software performs the following tasks:
  1. Initialize driver
  2. Start synchronization interface
  3. Start the serial interface
  4. Sends a single word of data to all slaves
  5. Send a block of data to all slaves
  6. Reads a single word of data from first slave
  7. Read a block of data from first slave
  8. Change input select to second slave
  9. Reads a single word of data from second slave
  10. Read a block of data from second slave
  11. Stop synchronization interface
  12. Change source of synchronization pulse
  13. Stop serial interface
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
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

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

#include <rtems/libi2c.h>
#include <libchip/i2c-2b-eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grlib/grascs.h>

rtems_task main_task(rtems_task_argument argument);

rtems_id   Task_id[3];         /* array of task ids */
rtems_name Task_name[3];       /* array of task names */

rtems_task Init(rtems_task_argument ignored) {
  
  rtems_status_code status;

  printf("******** Starting GRASCS RTEMS demo ********\n");

  /* 1. Initialize driver */
  printf("Initilizing driver\n");
  status = ASCS_init();

  if(status < 0) {
    printf("ERROR: Failed to initialize ASCS driver\n");
    exit(0);
  }
  printf("Successfully intialized ASCS driver\n");

  Task_name[1] = rtems_build_name( 'T', 'S', 'K', 'A');

  status = rtems_task_create(Task_name[1], 1, RTEMS_MINIMUM_STACK_SIZE*2,
			     RTEMS_DEFAULT_MODES,
			     RTEMS_DEFAULT_ATTRIBUTES, &Task_id[1]);

  status = rtems_task_start(Task_id[1], main_task, 1);

  status = rtems_task_delete(RTEMS_SELF);
}

rtems_task main_task(rtems_task_argument unused) {

  int i, retval;

  /********************/
  /* Data preparation */
  /********************/

  /* define the DBITS32 if a 32 bit core is used or DBITS8 for a
     8 bit core. 16 bit is default. */
  /* #define DBITS32 */
  /* #define DBITS8 */

  /* NWORDS is the number of words that should is sent/received in
     the TC_send_block/TM_recv_block calls below */
#define NWORDS 8

  /* tmp[i] is filled with the data that the core should send in the
     i:th TC.*/
#ifdef DBITS32
  unsigned int tmp[NWORDS+1];
  
  tmp[0] = 0x04030201;
  for(i = 1; i <= NWORDS; i++)
    tmp[i] = tmp[i-1] + (4 | (4 << 8) | (4 << 16) | (4 << 24));
#else
#ifdef DBITS8
  unsigned char tmp[NWORDS+1];

  tmp[0] = 0x01;
  for(i = 1; i <= NWORDS; i++)
    tmp[i] = tmp[i-1] + 1;
#else
  unsigned short int tmp[NWORDS+1];

  tmp[0] = 0x0201;
  for(i = 1; i <= NWORDS; i++)
    tmp[i] = tmp[i-1] + (2 | (2 << 8));
#endif
#endif
  
  /* 2. Start synchronization interface */
  printf("Starting synchronization interface\n");
  ASCS_TC_sync_start();
  while(!(ASCS_iface_status() & GRASCS_STS_ERUNNING))
    ;

  /* 3. Start serial interface */
  printf("Starting serial interface\n");
  ASCS_start(); 

  /* 4. Send a single TC */
  printf("Starting single TC\n");
  retval = ASCS_TC_send((int*)tmp);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_STARTSTOP)
      printf("ERROR: Failed to start TC because serial interface never started\n");
    else if(retval == -GRASCS_ERROR_TRANSACTIVE)
      printf("ERROR: Failed to start TC because core says a transaction is in progress\n");
    goto demoend;
  }
  printf("Single TC done\n");

  /* 5 .Send a block of TCs */
  printf("Starting block of TCs\n");
  retval = ASCS_TC_send_block((int*)(tmp+1),NWORDS);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_STARTSTOP)
      printf("ERROR: Failed to start TC because serial interface never started\n");
    else if(retval == -GRASCS_ERROR_TRANSACTIVE)
      printf("ERROR: Failed to start TC because core says a transaction is in progress\n");
    goto demoend;
  }
  printf("Block of TCs done\n");
  
  /* 6.Send a single TM and check that data is from first slave */
  printf("Starting single TM\n");
  retval = ASCS_TM_recv((int*)tmp);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_STARTSTOP)
      printf("ERROR: Failed to start TM because serial interface never started\n");
    else if(retval == -GRASCS_ERROR_TRANSACTIVE)
      printf("ERROR: Failed to start TM because core says a transaction is in progress\n");
    goto demoend;
  }
  printf("Single to first slave TM is done. Data:\n\t0x%x\n", tmp[0]);  
  
  /* 7. Send a block of TMs and check that data is from first slave */
  printf("Starting a block of TMs\n");
  retval = ASCS_TM_recv_block((int*)(tmp+1),NWORDS);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_STARTSTOP)
      printf("ERROR: Failed to start TM because serial interface never started\n");
    else if(retval == -GRASCS_ERROR_TRANSACTIVE)
      printf("ERROR: Failed to start TM because unprocessed TM exist\n");
    goto demoend;
  }
  printf("Block of TMs to first slave is done. Data:\n");
  for(i = 1; i <= NWORDS; i++)
    printf("\t0x%x\n",tmp[i]);
  
  /* 8. Set slave sel to second slave */
  printf("Setting slave select to 1\n");
  retval = ASCS_input_select(1);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_CAPFAULT)
      printf("ERROR: Failed to change slave select due to invalid slave number\n");
    else
      printf("ERROR: Failed to change slave select due to active TM\n");
    goto demoend;
  }
  
  /* 9. Send a single TM and check that data is from second slave */
  printf("Starting single TM\n");
  retval = ASCS_TM_recv((int*)tmp);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_STARTSTOP)
      printf("ERROR: Failed to start TM because serial interface never started\n");
    else if(retval == -GRASCS_ERROR_TRANSACTIVE)
      printf("ERROR: Failed to start TM because core says a transaction is in progress\n");
    goto demoend;
  }
  printf("Single to second slave TM is done. Data:\n\t0x%x\n", tmp[0]);  
  
  /* 10. Send a block of TMs and check that data is from second slave */
  printf("Starting a block of TMs\n");
  retval = ASCS_TM_recv_block((int*)(tmp+1),NWORDS);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_STARTSTOP)
      printf("ERROR: Failed to start TM because serial interface never started\n");
    else if(retval == -GRASCS_ERROR_TRANSACTIVE)
      printf("ERROR: Failed to start TM because unprocessed TM exist\n");
    goto demoend;
  }
  printf("Block of TMs to first slave is done. Data:\n");
  for(i = 1; i <= NWORDS; i++)
    printf("\t0x%x\n",tmp[i]);
      
  /* 11. Stop synchronization interface */
  printf("Stopping synchronization interface\n");
  ASCS_TC_sync_stop();
  while((ASCS_iface_status() & GRASCS_STS_ERUNNING))
    ;  

  /* 12. Change source of synchronization pulse */
  printf("Changing source of synchronization pulse\n");
  retval = ASCS_etr_select(3,10);
  if(retval < 0) {
    if(retval == -GRASCS_ERROR_CAPFAULT)
      printf("Failed to change ETR source due to invalid src or frequency\n");
    else
      printf("Failed to change ETR source since synch interface is running\n");
    goto demoend;
  }

  /* 13. Stop serial interface */
  printf("Stopping serial interface\n");
  ASCS_stop();
  while((ASCS_iface_status() & GRASCS_STS_RUNNING))
    ;  

 demoend:
  
  if(retval < 0)
    printf("******** GRASCS RTEMS demo ended with failure ********\n");
  else
    printf("******** GRASCS RTEMS demo ended successfully ********\n");

  exit(0);  
}
