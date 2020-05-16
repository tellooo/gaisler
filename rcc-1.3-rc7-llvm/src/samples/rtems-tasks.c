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
#include <assert.h>

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
  struct drvmgr_key grlib_drv_res_gptimer0[] =
  {
  	/* If all timers should not be used (typically on an AMP system, or when timers
  	 * are used customly in a project) one can limit to a range of timers.
  	 * timerStart: start of range (0..6)
  	 * timerCnt: Number of timers
  	 */
	#if 0
  	{"timerStart", DRVMGR_KT_INT, {(unsigned int)SET_START}},
  	{"timerCnt", DRVMGR_KT_INT, {(unsigned int)SET_NUMBER_FO_TIMERS}},
	  /* Select Prescaler (Base frequency of all timers on a timer core) */
  	{"prescaler", DRVMGR_KT_INT, {(unsigned int)SET_PRESCALER_HERE}},
  	/* Select which timer should be used as the system clock (default is 0) */
  	{"clockTimer", DRVMGR_KT_INT, {(unsigned int)TIMER_INDEX_USED_AS_CLOCK}},
  	#endif
  	DRVMGR_KEY_EMPTY
  };

  /* Use GPTIMER core 4 (not present in most systems) as a high
   * resoulution timer */
  struct drvmgr_key grlib_drv_res_gptimer4[] =
  {
  	{"prescaler", DRVMGR_KT_INT, {(unsigned int)4}},
  	DRVMGR_KEY_EMPTY
  };

  struct drvmgr_bus_res grlib_drv_resources =
  {
    .next = NULL,
    .resource = {
  #if 0
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 0, &grlib_drv_res_gptimer0[0]},
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 1, NULL}, /* Do not use timers on this GPTIMER core */
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 2, NULL}, /* Do not use timers on this GPTIMER core */
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 3, NULL}, /* Do not use timers on this GPTIMER core */
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 4, &grlib_drv_res_gptimer4[0]},
  #endif
  	DRVMGR_RES_EMPTY
    }
  };
 #endif

 #include <drvmgr/drvmgr_confdefs.h>
#endif

/*
 *  Handy macros and static inline functions
 */

/*
 *  Macro to hide the ugliness of printing the time.
 */

#define print_time(_s1, _tb, _s2) \
  do { \
    iprintf( "%s%02"PRIu32":%02"PRIu32":%02"PRIu32"   %02"PRIu32"/%02"PRIu32"/%04"PRIu32"%s", \
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
  rtems_status_code status;
  rtems_time_of_day time;

  puts( "\n\n*** CLOCK TICK TEST ***" );

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
      puts( "*** END OF CLOCK TICK TEST ***" );
      exit( 0 );
    }
    put_name( Task_name[ task_index ], FALSE );
    print_time( " - rtems_clock_get - ", &time, "\n" );
    status = rtems_task_wake_after( task_index * 5 * rtems_clock_get_ticks_per_second() );
    assert(status == RTEMS_SUCCESSFUL);
  }
}
