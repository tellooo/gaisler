/*
 * This file is the SMP part of the SP/SMP demonstration. Some of the code is
 * based on "Test_task" according to below.
 */
/*  Test_task
 *
 *  This routine serves as a test task.  It verifies the basic task
 *  switching capabilities of the executive.
 *
 *  Input parameters:  NONE
 *
 *  Output parameters:  NONE
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id: tasks.c,v 1.7.2.1 2000/05/05 12:58:06 joel Exp $
 */

#include <rtems.h>
#include <stdio.h>
#include <assert.h>

#ifndef CFG_ISMASTER
 #error CFG_ISMASTER not set
#endif

const int STARTNEXT = CFG_ISMASTER;

/*
 ****
 * RTEMS SMP for GR740:
 * - Do not use APBUART0
 * - Do not use GPTIMER0
 * - Do not use GPTIMER2
 * - Do not use GPTIMER3
 * - Do not use GPTIMER4
 * - Debug UART is APBUART1
 * - Use processors CPU0, CPU1, CPU2
 *
 * For GPTIMER1: the RTEMS image only uses subtimer index 1 (zero-based index).
 *
 * CPU3 is free to use APBUART0 and all GPTIMER except for GPTIMER1.
 ****
 */
#ifdef CONFIGURE_MAXIMUM_PROCESSORS
#undef CONFIGURE_MAXIMUM_PROCESSORS
#endif
#define CONFIGURE_MAXIMUM_PROCESSORS 4

/* functions */

rtems_task Init(
  rtems_task_argument argument
);

rtems_task Test_task(
  rtems_task_argument argument
);

/* global variables */

/*
 *  Keep the names and IDs in global variables so another task can use them.
 */ 

extern rtems_id   Task_id[ 4 ];         /* array of task ids */
extern rtems_name Task_name[ 4 ];       /* array of task names */

/* configuration information */

#include <bsp.h> /* for device driver prototypes */

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

/* Describe whitch processors to use in thie SMP application. */
#if CFG_ISMASTER
  /* RTEMS SMP on CPU[0,1,2]. Something else on CPU[3]. */
  #define CONFIGURE_SCHEDULER_ASSIGNMENTS \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
    RTEMS_SCHEDULER_ASSIGN_NO_SCHEDULER
#else
  /* Something else on CPU[0]. RTEMS SMP on CPU[1,2,3]. */
  #define CONFIGURE_SCHEDULER_ASSIGNMENTS \
    RTEMS_SCHEDULER_ASSIGN_NO_SCHEDULER, \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY)
#endif

#include <rtems/confdefs.h>

/* If --drvmgr was enabled during the configuration of the RTEMS kernel */
#ifdef RTEMS_DRVMGR_STARTUP
 #ifdef LEON3
  /* Add Timer and UART Driver for this example */
  #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
   #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
  #endif
  #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
   #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
  #endif

  #include <grlib/ambapp_bus.h>
  /* OPTIONAL FOR GRLIB SYSTEMS WITH GPTIMER AS SYSTEM CLOCK TIMER */
  struct drvmgr_key grlib_drv_res_gptimer1[] =
  {
  /*
   * SMP test for GR740. Use GPTIMER1, subtimer 1 only and no other.
   * timerCnt = 1 prevents shared GPTIMER interrupts since GPTIMER1 does
   * not have separate interrupts.
   */
   /* timerStart: start of range (0..6) */
   {"timerStart", DRVMGR_KT_INT, {(unsigned int)1}},
   /* timerCnt: Number of timers */
   {"timerCnt", DRVMGR_KT_INT, {(unsigned int)1}},
   DRVMGR_KEY_EMPTY
  };

  struct drvmgr_bus_res grlib_drv_resources =
  {
    .next = NULL,
    .resource = {
    {DRIVER_AMBAPP_GAISLER_APBUART_ID, 0, NULL}, /* Do not use APBUART0 */
    {DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 0, NULL}, /* Do not use GPTIMER0 */
    {DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 1, &grlib_drv_res_gptimer1[0]},
    {DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 2, NULL}, /* Do not use GPTIMER2 */
    {DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 3, NULL}, /* Do not use GPTIMER3 */
    {DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 4, NULL}, /* Do not use GPTIMER4 */
    DRVMGR_RES_EMPTY
    }
  };
 #endif

 #include <drvmgr/drvmgr_confdefs.h>
#endif

int debug_uart_index = 2; /* second UART -- APBUART[1] */
/*
 *  Handy macros and static inline functions
 */

/*
 *  Macro to hide the ugliness of printing the time.
 */

#define print_time(_s1, _tb, _s2) \
  do { \
    printf( "%s%02"PRIu32":%02"PRIu32":%02"PRIu32"   %02"PRIu32"/%02"PRIu32"/%04"PRIu32"%s", \
       _s1, (_tb)->hour, (_tb)->minute, (_tb)->second, \
       (_tb)->month, (_tb)->day, (_tb)->year, _s2 ); \
    fflush(stdout); \
  } while ( 0 )

/*
 *  Macro to print an task name that is composed of ASCII characters.
 *
 */

#define put_name( _name, _crlf ) \
  do { \
    uint32_t c0, c1, c2, c3; \
    \
    c0 = ((_name) >> 24) & 0xff; \
    c1 = ((_name) >> 16) & 0xff; \
    c2 = ((_name) >> 8) & 0xff; \
    c3 = (_name) & 0xff; \
    putchar( (char)c0 ); \
    if ( c1 ) putchar( (char)c1 ); \
    if ( c2 ) putchar( (char)c2 ); \
    if ( c3 ) putchar( (char)c3 ); \
    if ( (_crlf) ) \
      putchar( '\n' ); \
  } while (0)

/*
 *  This allows us to view the "Test_task" instantiations as a set
 *  of numbered tasks by eliminating the number of application
 *  tasks created.
 *
 *  In reality, this is too complex for the purposes of this
 *  example.  It would have been easier to pass a task argument. :)
 *  But it shows how rtems_id's can sometimes be used.
 */

#define task_number( tid ) \
  ( rtems_object_id_get_index( tid ) - \
     rtems_configuration_get_rtems_api_configuration()->number_of_initialization_tasks )


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/*
 *  Keep the names and IDs in global variables so another task can use them.
 */

rtems_id   Task_id[ 4 ];         /* array of task ids */
rtems_name Task_name[ 4 ];       /* array of task names */

rtems_task Init(
  rtems_task_argument argument
)
{
  (void) argument;
  rtems_status_code status;
  rtems_time_of_day time;

  puts( "SMP: *** CLOCK TICK TEST ***" );

  time.year   = 1988;
  time.month  = 12;
  time.day    = 31;
  time.hour   = 9;
  time.minute = 0;
  time.second = 0;
  time.ticks  = 0;

  status = rtems_clock_set( &time );
  assert(status == RTEMS_SUCCESSFUL);

  Task_name[ 1 ] = rtems_build_name( 'T', 'A', '1', ' ' );
  Task_name[ 2 ] = rtems_build_name( 'T', 'A', '2', ' ' );
  Task_name[ 3 ] = rtems_build_name( 'T', 'A', '3', ' ' );

  status = rtems_task_create(
    Task_name[ 1 ], 1, RTEMS_MINIMUM_STACK_SIZE * 2, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &Task_id[ 1 ]
  );
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_create(
    Task_name[ 2 ], 1, RTEMS_MINIMUM_STACK_SIZE * 2, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &Task_id[ 2 ]
  );
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_create(
    Task_name[ 3 ], 1, RTEMS_MINIMUM_STACK_SIZE * 2, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &Task_id[ 3 ]
  );
  assert(status == RTEMS_SUCCESSFUL);

  if (STARTNEXT) {
    printf("SMP:   I am the master\n");
    printf("SMP:   Waking up CPU[3]...\n");
    /* ...using multiprocessor status register of interrupt controller 3. */
    *((volatile unsigned int *) 0xff907010) = (1<<3);
    status = rtems_task_wake_after( 2 * rtems_clock_get_ticks_per_second() );
  } else {
    printf("SMP:   I am the slave\n");
  }

  status = rtems_task_start( Task_id[ 1 ], Test_task, 1 );
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_start( Task_id[ 2 ], Test_task, 2 );
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_start( Task_id[ 3 ], Test_task, 3 );
  assert(status == RTEMS_SUCCESSFUL);

  status = rtems_task_delete( RTEMS_SELF );
  assert(status == RTEMS_SUCCESSFUL);
}
#include <stdio.h>

rtems_task Test_task(
  rtems_task_argument unused
)
{
  (void) unused;
  rtems_id          tid;
  rtems_time_of_day time;
  uint32_t  task_index;
  rtems_status_code status;

  status = rtems_task_ident( RTEMS_SELF, RTEMS_SEARCH_ALL_NODES, &tid );
  assert(status == RTEMS_SUCCESSFUL);
  task_index = task_number( tid );
  for ( ; ; ) {
    status = rtems_clock_get_tod( &time );
    assert(status == RTEMS_SUCCESSFUL);
    if ( time.second >= 3335 ) {
      puts( "SMP: *** END OF CLOCK TICK TEST ***" );
      exit( 0 );
    }
    put_name( Task_name[ task_index ], FALSE );
    print_time( " - rtems_clock_get - ", &time, "\n" );
    status = rtems_task_wake_after( task_index * 1 * rtems_clock_get_ticks_per_second() );
    assert(status == RTEMS_SUCCESSFUL);
  }
}

