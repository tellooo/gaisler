
/* Application example for using the L4stat with Gaisler RTEMS environment.
 * 
 *-----------------------------------------------------------------------
 * Operation:
 *
 * This test sets up the L4stat device to count instructions by the
 * init CPU. 
 *
 * The application periodically polls and prints the counter value. 
 *
 * Configuration:
 *
 * The test is configured with:
 *   POLLTIME:      Determines the polling interval in seconds (Default: 1)
 *   EVENT:         Determines the event to log (Default: 0x11 is instructions)
 *   COUNTER:       Determines which counter index to use (Default: 0)
 *   
 *-----------------------------------------------------------------------
 *
 * Author: Javier Jalle, Cobham Gaisler
 * Contact: support@gaisler.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <rtems.h>

/* -------------- Application setup ----------   */
#ifndef POLLTIME
#define POLLTIME 1
#endif
#ifndef EVENT
#define EVENT 0x11
#endif
#ifndef COUNTER
#define COUNTER 0
#endif

/* ---------------- RTEMS Setup ---------------  */

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */
#include <grlib/l4stat.h>

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
#define CONFIGURE_INIT_TASK_ATTRIBUTES    RTEMS_DEFAULT_ATTRIBUTES
#define CONFIGURE_EXTRA_TASK_STACKS         (40 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MICROSECONDS_PER_TICK     RTEMS_MILLISECONDS_TO_MICROSECONDS(2)

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
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_L4STAT

#include <drvmgr/drvmgr_confdefs.h>

#include "config.c"

/* ----------------- Application code ---------------- */

/* 
 * Report task. Reads the counter and prints the value
 */

static rtems_task report_task_entry(rtems_task_argument ignored)
{
    int ret;
    uint32_t val;
    rtems_interval interval = rtems_clock_get_ticks_per_second()*POLLTIME;

    while (1) {
        /* Sleep for poll time */
        rtems_task_wake_after(interval);
        /* Poll counter */
        ret = l4stat_counter_get(COUNTER, &val);
        if (ret != 0){
            printf("[F] Fail when obtaining message\n");
            continue;
        }
        printf("[C] Counter value: %d\n", (unsigned int) val);
    }
}

rtems_task Init(rtems_task_argument ignored)
{
    int ret;
    int i;
    int cpu_self;
    rtems_id report_task;

    printf("-- L4stat RTEMS test application --\n");  
    printf("Config: Event: %d\nPolltime: %d s\n",EVENT,POLLTIME);

    /* Get current CPU */
    cpu_self = (int) rtems_scheduler_get_processor();

    /* Clear and disable counters */
    for (i=0; i<16; i++){
        ret = l4stat_counter_clear(i);
        if (ret != L4STAT_ERR_OK){
            printf("Error: l4stat clear!\n");
            exit(1);
        }
        ret = l4stat_counter_disable(i);
        if (ret != L4STAT_ERR_OK){
            printf("Error: l4stat disable!\n");
            exit(1);
        }
    }

    /* Enable counter for CPU */
    ret = l4stat_counter_enable(COUNTER, EVENT, cpu_self, 0);
    if (ret != L4STAT_ERR_OK){
        printf("Error: l4stat enable!\n");
        exit(1);
    }

    /* Launch application tasks */
    ret = rtems_task_create(rtems_build_name('r','e','p','t'), 200,
            RTEMS_MINIMUM_STACK_SIZE, 0, 0, &report_task);
    if (ret != RTEMS_SUCCESSFUL){
        rtems_fatal_error_occurred(ret);
    }
    ret = rtems_task_start(report_task, report_task_entry, 0);
    if (ret != RTEMS_SUCCESSFUL){
        rtems_fatal_error_occurred(ret);
    }

    /* Remove init task */
    rtems_task_delete(RTEMS_SELF);
    rtems_fatal_error_occurred(0);
}
