
/* Application example for using the memory controller with EDAC together with
 * the memory scrubber under the Gaisler RTEMS environment.
 * 
 *-----------------------------------------------------------------------
 * Operation:
 *
 * This test sets up the memscrub device to repeatedly scrub a
 * memory area. If multiple correctable errors occur during the same
 * scrubber iteration, the scrubber is switched into regeneration
 * mode. When regeneration has completed, the ordinary scrubbing is
 * resumed.
 *
 * The driver can be used in two modes, a "managed" mode where we
 * get an IRQ on every single error on the AHB bus and every completed
 * scrub run, and a silent mode where it sets up the scrubber to loop
 * in the background and only interrupt after multiple correctable
 * errors condition or on an uncorrectable error. The driver is
 * completely interrupt driven, and in the silent mode it does not
 * consume any CPU time during normal scrubbing operation.
 *
 * An RTEMS message queue is used to communicate status information
 * from the driver's interrupt handler to the user. Depending on mode,
 * different amount of messages will be sent. The driver ignores errors 
 * resulting from the queue being full, therefore the user is not 
 * required to read out the queue. The application needs to provide 
 * room for the message queue via the configuration table.
 *
 * Configuration:
 *
 * The test is configured via a number of driver resources:
 *   "memstart","memsize":        Determines the memory area to scrub
 *   "opermode":                  Operating mode:
 *                                  0=Quiet mode
 *                                  1=Verbose mode
 *                                  2=Semi-verbose mode (done but no single-CE messages)
 *                                (default: 0)
 *   "regenthres":                Number of errors before regeneration mode
 *                                is entered (default: 5)
 *   "scrubdelay","regendelay":   Delay time in cycles between processing 
 *                                bursts in scrub/regeneration modes.
 *                                (default: 100 scrub / 10 regen)
 *-----------------------------------------------------------------------
 *
 * Author: Javier Jalle/Magnus Hjorth, Cobham Gaisler
 * Contact: support@gaisler.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <rtems.h>
#include <amba.h>
#include "msgq.h"

#define OPERMODE_QUIET 0
#define OPERMODE_VERBOSE 1
#define OPERMODE_SEMIVERBOSE 2

/* -------------- Application setup ----------   */
#ifndef MEMSTART
#define MEMSTART (0x40000000)
#endif
#ifndef MEMSIZE
#define MEMSIZE  (0x1000000)
#endif
#ifndef OPERMODE
#define OPERMODE OPERMODE_QUIET
#endif
#ifndef REGEN_DELAY
#define REGEN_DELAY 5
#endif
#ifndef REGEN_THRESHOLD
#define REGEN_THRESHOLD 5
#endif
#ifndef SCRUB_DELAY
#define SCRUB_DELAY 10
#endif

#define MEMEND (MEMSTART + MEMSIZE -1)
int opermode = OPERMODE;

/* ---------------- RTEMS Setup ---------------  */

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */
#include <grlib/memscrub.h>


rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

/* Set some values if someone should modify the example. The shared IRQ layer
 * need one semaphore.
 */
#define CONFIGURE_MAXIMUM_TASKS             8
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    MEMSCRUB_MAXIMUM_MESSAGE_QUEUES
#define CONFIGURE_MESSAGE_BUFFER_MEMORY     MEMSCRUB_MESSAGE_BUFFER_MEMORY
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
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_MEMSCRUB

#include <drvmgr/drvmgr_confdefs.h>

#include "config.c"

static void memscrub_isr(void *arg, uint32_t ahbaccess, uint32_t ahbstatus, uint32_t scrubstatus);

/* ----------------- Application code ---------------- */

/* /////////
 * Report task. Reads the scrubber message queue and prints a text
 * message whenever something has happened. */

static rtems_task report_task_entry(rtems_task_argument ignored)
{
    int ret;
    struct memscrub_message msg;
    while (1) {
        ret = memscrub_msgq_pop(1,&msg);
        if (ret != 0){
            printf("[R] Fail when obtaining message\n");
            continue;
        }
        memscrub_msgq_print(&msg);
    }
}

rtems_task Init(rtems_task_argument ignored)
{
    int options;
    int optionsahb, optionsscrub;
    int ret;
    rtems_id report_task;

    printf("-- Scrubber RTEMS test application --\n");  
    printf("Config: Mem range: %08x-%08x\nOpermode:%s\n", 
            (unsigned int)MEMSTART, 
            (unsigned int)(MEMSTART+MEMSIZE-1),
            (opermode == OPERMODE_SEMIVERBOSE)? 
                "semiverbose":
                    ((opermode == OPERMODE_VERBOSE)?
                        "verbose":(
                            (opermode == OPERMODE_QUIET)?
                                "quiet": "unknown"))
            );

    /* Init msgq */
    ret = memscrub_msgq_init();
    if (ret != 0) {
        printf("Error: memscrub msgq init!\n");
        exit(1);
    }

    /* Register and enable ISR */
    ret = memscrub_isr_register(memscrub_isr,NULL);  
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub isr register!\n");
        exit(1);
    }

    /* Stop scrubbing */
    ret = memscrub_stop();
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub stop!\n");
        exit(1);
    }

    /* Set memory range */
    ret = memscrub_range_set(MEMSTART,MEMEND);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub set range!\n");
        exit(1);
    }

    /* Get options */
    switch(opermode){
        case OPERMODE_QUIET:
            optionsahb = MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_ENABLE |
                MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_DISABLE;
            optionsscrub = MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_DISABLE |
                MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_ENABLE;
            options = MEMSCRUB_OPTIONS_LOOPMODE_ENABLE;
            break;
        case OPERMODE_VERBOSE:
            optionsahb = MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_ENABLE |
                MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_ENABLE;
            optionsscrub = MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_DISABLE |
                MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_DISABLE;
            options = MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE;
            break;
        case OPERMODE_SEMIVERBOSE:
        default:
            optionsahb = MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_ENABLE |
                MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_DISABLE;
            optionsscrub = MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_DISABLE |
                MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_ENABLE;
            options = MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE;
            break;
    }

    /* Setup scrubber thresholds */
    ret = memscrub_ahberror_setup(0,0,optionsahb);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub ahberr setup!\n");
        exit(1);
    }
    ret = memscrub_scruberror_setup(0,REGEN_THRESHOLD, optionsscrub);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub scruberr setup!\n");
        exit(1);
    }

    /* Start scrubber */
    ret = memscrub_scrub_start(SCRUB_DELAY,options);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub start!\n");
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

int ce_count;
int ue_count;

/* Interrupt service routine */
static void memscrub_isr(void *arg, uint32_t ahbaccess, uint32_t ahbstatus, uint32_t scrubstatus)
{
    int tmp;
    int runcount;
    int action=0; /* 0=None, 1=Restart scrubbing, 2=Switch to regen*/

    /* Push messages */
    if (ahbstatus & AHBS_DONE){
        memscrub_msgq_push_done(ahbaccess, ahbstatus, scrubstatus);
        action=1;
    }
    if (ahbstatus & AHBS_NE){
        memscrub_msgq_push_error(ahbaccess, ahbstatus, scrubstatus);
    }

    /* Update CE stats */
    if (ahbstatus & AHBS_CE){
        tmp = ((ahbstatus & AHBS_CECNT)>> AHBS_CECNT_BIT);
        ce_count += tmp;
        /* Generate CE threshold flag manually for verbose mode */
        if (opermode == OPERMODE_VERBOSE){
            runcount = (scrubstatus >> STAT_RUNCOUNT_BIT) & STAT_RUNCOUNT;
            if(runcount >= REGEN_THRESHOLD){
                action = 2;
            }
        }
    }

    /* Update UE stats */
    if ((ahbstatus & (AHBS_NE|AHBS_CE|AHBS_SBC|AHBS_SEC))==AHBS_NE){
        tmp = ((ahbstatus & AHBS_UECNT)>> AHBS_UECNT_BIT);
        ue_count += tmp;
    }

    /* Check run count threshold */
    if ((ahbstatus & AHBS_SEC)){
        action=2;
    }

    /* Perform action*/
    if (action == 1){
        /* Restart scrubbing */
        memscrub_scrub_start(SCRUB_DELAY, 
                (opermode == OPERMODE_QUIET)? 
                    MEMSCRUB_OPTIONS_LOOPMODE_ENABLE:
                    MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE);
    }else if (action == 2){
        /* Switch to regen mode */
        memscrub_regen_start(REGEN_DELAY,
                MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE);
        /* Send message */
        memscrub_msgq_push_regen();
    }
}

