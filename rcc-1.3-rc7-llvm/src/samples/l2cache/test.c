
/* Application example for using the L2cache with Gaisler RTEMS environment.
 * 
 *-----------------------------------------------------------------------
 * Operation:
 *
 * This test sets up the L2cache to lock the ways and fetch the data
 * every time there is an correctable/uncorrectable error. It also
 * uses the L4STAT core to record L2cache errors.
 *
 *
 * Configuration:
 *
 * The test is configured with:
 *   POLLTIME:      Determines the polling interval in seconds (Default: 1)
 *   SCRUB_DELAY:   Determines the L2 scrubber delay (Default: 0x10)
 *   ADDR_WAY<n>:   Determines the address to fill in way <n>.
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
#ifndef SCRUB_DELAY
#define SCRUB_DELAY 0x10
#endif
#ifndef ADDR_WAY0
#define ADDR_WAY0 0x00000000
#endif
#ifndef ADDR_WAY1
#define ADDR_WAY1 0x00080000
#endif
#ifndef ADDR_WAY2
#define ADDR_WAY2 0x00100000
#endif
#ifndef ADDR_WAY3
#define ADDR_WAY3 0x00180000
#endif

/* ---------------- RTEMS Setup ---------------  */

#define EVENT_L2_HIT 0x60
#define EVENT_L2_MISS 0x61
#define EVENT_L2_CTE 0x63
#define EVENT_L2_UTE 0x64
#define EVENT_L2_CDE 0x65
#define EVENT_L2_UDE 0x66
#define CPU_L2_HIT 0
#define CPU_L2_MISS 0
#define CPU_L2_CTE 0
#define CPU_L2_UTE 0
#define CPU_L2_CDE 0
#define CPU_L2_UDE 0
#define COUNTER_L2_CTE 0
#define COUNTER_L2_UTE 1
#define COUNTER_L2_CDE 2
#define COUNTER_L2_UDE 3
#define COUNTER_L2_HIT 4
#define COUNTER_L2_MISS 5

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */
#include <grlib/l4stat.h>
#include <grlib/l2c.h>

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
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_L2CACHE

#include <drvmgr/drvmgr_confdefs.h>

#include "config.c"

/* ----------------- Application code ---------------- */

/* 
 * Report task. Reads the counter and prints the value
 */

uint32_t filled=0;

static void l2c_isr(void *data, uint32_t addr, uint32_t status){
    uint32_t cte=0;
    uint32_t ute=0;
    uint32_t ude=0;

    //printf("ISR triggered with args: data(0x%08x), addr(0x%08x), status(0x%08x)\n", (unsigned int) data, addr, status );
    l4stat_counter_get(COUNTER_L2_CTE, &cte);
    l4stat_counter_get(COUNTER_L2_UTE, &ute);
    l4stat_counter_get(COUNTER_L2_UDE, &ude);
    
    /* Fill/fetch L2 cache ways */
    if ((cte > 0)||(ute > 0)||(ude > 0)){
        l4stat_counter_clear(COUNTER_L2_CTE);
        l4stat_counter_clear(COUNTER_L2_UTE);
        l4stat_counter_clear(COUNTER_L2_CDE);
        l4stat_counter_clear(COUNTER_L2_UDE);
        l2cache_fill_way(0, ADDR_WAY0, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK);
        l2cache_fill_way(1, ADDR_WAY1, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK);
        l2cache_fill_way(2, ADDR_WAY2, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK);
        l2cache_fill_way(3, ADDR_WAY3, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK);
        filled++;
    }

    return;
}

static int l2cache_setup()
{
    int ret;
    int status;
    int flush;

    /* Check L2 cache status */
    status = l2cache_status();
    if (L2CACHE_ENABLED(status)){
        flush = L2CACHE_OPTIONS_FLUSH_INV_WBACK;
    }else{
        flush = L2CACHE_OPTIONS_FLUSH_INVALIDATE;
    }

    /* Disable L2 cache */
    ret = l2cache_disable(flush);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache disable!\n");
        return -1;
    }

    /* Unlock ways */
    ret = l2cache_unlock();
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache unlock!\n");
        return -1;
    }

    /* Mask interrupts */
    ret = l2cache_interrupt_mask(L2CACHE_INTERRUPT_ALL);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache interrupt mask!\n");
        return -1;
    }

    /* Set writethrough */
    ret = l2cache_writethrough(L2CACHE_OPTIONS_FLUSH_NONE);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache writethrough!\n");
        return -1;
    }

    /* Enable EDAC */
    ret = l2cache_edac_enable(L2CACHE_OPTIONS_FLUSH_NONE);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache edac enable!\n");
        return -1;
    }

    /* Install ISR for corr and uncorr errors */
    ret = l2cache_isr_register(l2c_isr, NULL, L2CACHE_INTERRUPT_CORRERROR | L2CACHE_INTERRUPT_UNCORRERROR);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache isr!\n");
        return -1;
    }

    /* Lock ways and enable L2 cache */
    ret = l2cache_lock_way(ADDR_WAY3, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK, L2CACHE_OPTIONS_ENABLE);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache lock way!\n");
        return -1;
    }
    ret = l2cache_lock_way(ADDR_WAY2, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK, L2CACHE_OPTIONS_ENABLE);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache lock way!\n");
        return -1;
    }
    ret = l2cache_lock_way(ADDR_WAY1, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK, L2CACHE_OPTIONS_ENABLE);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache lock way!\n");
        return -1;
    }
    ret = l2cache_lock_way(ADDR_WAY0, L2CACHE_OPTIONS_VALID | L2CACHE_OPTIONS_FETCH , L2CACHE_OPTIONS_FLUSH_INV_WBACK, L2CACHE_OPTIONS_ENABLE);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache lock way!\n");
        return -1;
    }
    filled++;

    /* Enable Scrubber */
    ret = l2cache_scrub_enable(SCRUB_DELAY);
    if (ret != L2CACHE_ERR_OK){
        printf("Error: l2cache scrub enable!\n");
        return -1;
    }

    printf("-- L2cache core configured --\n");  
    printf(" -L2cache enabled\n");
    printf(" -Writethrough write policy\n");
    printf(" -Locked way[0]: Tag: 0x%08x\n", ADDR_WAY0);
    printf(" -Locked way[1]: Tag: 0x%08x\n", ADDR_WAY1);
    printf(" -Locked way[2]: Tag: 0x%08x\n", ADDR_WAY2);
    printf(" -Locked way[3]: Tag: 0x%08x\n", ADDR_WAY3);
    printf(" -EDAC enabled\n");
    printf(" -Scrub enabled, delay: %d\n", SCRUB_DELAY);

    return 0;
}

static int l4stat_setup()
{
    int ret;
    int i;

    /* Clear and disable counters */
    for (i=0; i<16; i++){
        ret = l4stat_counter_clear(i);
        if (ret != L4STAT_ERR_OK){
            printf("Error: l4stat clear!\n");
            return -1;
        }
        ret = l4stat_counter_disable(i);
        if (ret != L4STAT_ERR_OK){
            printf("Error: l4stat disable!\n");
            return -1;
        }
    }

    /* Enable counter for L2 Tag Corr error */
    ret = l4stat_counter_enable(COUNTER_L2_CTE, EVENT_L2_CTE, CPU_L2_CTE, 0);
    if (ret != L4STAT_ERR_OK){
        printf("Error: l4stat enable!\n");
        return -1;
    }

    /* Enable counter for L2 Tag Uncorr error */
    ret = l4stat_counter_enable(COUNTER_L2_UTE, EVENT_L2_UTE, CPU_L2_UTE, 0);
    if (ret != L4STAT_ERR_OK){
        printf("Error: l4stat enable!\n");
        return -1;
    }

    /* Enable counter for L2 Data Corr error */
    ret = l4stat_counter_enable(COUNTER_L2_CDE, EVENT_L2_CDE, CPU_L2_CDE, 0);
    if (ret != L4STAT_ERR_OK){
        printf("Error: l4stat enable!\n");
        return -1;
    }

    /* Enable counter for L2 Data Uncorr error */
    ret = l4stat_counter_enable(COUNTER_L2_UDE, EVENT_L2_UDE, CPU_L2_UDE, 0);
    if (ret != L4STAT_ERR_OK){
        printf("Error: l4stat enable!\n");
        return -1;
    }

    /* Enable counter for L2 hits */
    ret = l4stat_counter_enable(COUNTER_L2_HIT, EVENT_L2_HIT, CPU_L2_HIT, 0);
    if (ret != L4STAT_ERR_OK){
        printf("Error: l4stat enable!\n");
        return -1;
    }

    /* Enable counter for L2 misses */
    ret = l4stat_counter_enable(COUNTER_L2_MISS, EVENT_L2_MISS, CPU_L2_MISS, 0);
    if (ret != L4STAT_ERR_OK){
        printf("Error: l4stat enable!\n");
        return -1;
    }

    printf("-- L4stat core configured --\n");  
    printf(" -Counter[%d]: Event: %d [%s]\n",COUNTER_L2_CTE, EVENT_L2_CTE, "Correctable L2 Tag error");
    printf(" -Counter[%d]: Event: %d [%s]\n",COUNTER_L2_UTE, EVENT_L2_UTE, "Uncorrectable L2 Tag error");
    printf(" -Counter[%d]: Event: %d [%s]\n",COUNTER_L2_CDE, EVENT_L2_CDE, "Correctable L2 Data error");
    printf(" -Counter[%d]: Event: %d [%s]\n",COUNTER_L2_UDE, EVENT_L2_UDE, "Uncorrectable L2 Data error");
    printf(" -Counter[%d]: Event: %d [%s]\n",COUNTER_L2_HIT, EVENT_L2_HIT, "L2 hits");
    printf(" -Counter[%d]: Event: %d [%s]\n",COUNTER_L2_MISS, EVENT_L2_MISS, "L2 misses");

    return 0;
}

static rtems_task report_task_entry(rtems_task_argument ignored)
{
    int ret;
    uint32_t hit, miss, cte, ute, cde, ude;
    rtems_interval interval = rtems_clock_get_ticks_per_second()*POLLTIME;

    printf("-- Report task started --\n");  
    while (1) {
        /* Sleep for poll time */
        rtems_task_wake_after(interval);
        /* Poll counters */
        ret = l4stat_counter_get(COUNTER_L2_HIT, &hit);
        if (ret != 0){
            printf("[F] Fail when obtaining L2 hit counter\n");
            continue;
        }
        printf("[HIT] L2cache hits: %d\n", (unsigned int) hit);
        ret = l4stat_counter_get(COUNTER_L2_MISS, &miss);
        if (ret != 0){
            printf("[F] Fail when obtaining L2 miss counter\n");
            continue;
        }
        printf("[MISS] L2cache misses: %d\n", (unsigned int) miss);
        ret = l4stat_counter_get(COUNTER_L2_CTE, &cte);
        if (ret != 0){
            printf("[F] Fail when obtaining L2 counter\n");
            continue;
        }
        printf("[CTE] L2cache Corr. Tag errors: %d\n", (unsigned int) cte);
        ret = l4stat_counter_get(COUNTER_L2_UTE, &ute);
        if (ret != 0){
            printf("[F] Fail when obtaining L2 counter\n");
            continue;
        }
        printf("[UTE] L2cache Uncorr. Tag errors: %d\n", (unsigned int) ute);
        ret = l4stat_counter_get(COUNTER_L2_CDE, &cde);
        if (ret != 0){
            printf("[F] Fail when obtaining L2 counter\n");
            continue;
        }
        printf("[CDE] L2cache Corr. Data errors: %d\n", (unsigned int) cde);
        ret = l4stat_counter_get(COUNTER_L2_UDE, &ude);
        if (ret != 0){
            printf("[F] Fail when obtaining L2 counter\n");
            continue;
        }
        printf("[UDE] L2cache Uncorr. Data errors: %d\n", (unsigned int) ude);
        printf("[FILL] Number of times L2 filled: %d\n", (unsigned int) filled);
    }
}

rtems_task Init(rtems_task_argument ignored)
{
    int ret;
    int cpu_self;
    rtems_id report_task;

    printf("-- L2cache RTEMS test application --\n");  

    /* Get current CPU */
    cpu_self = (int) rtems_scheduler_get_processor();
    printf("-- Running in cpu: %d --\n", cpu_self);  

    /* Setup L4stat */
    ret = l4stat_setup();
    if (ret != 0){
        exit(1);
    }

    /* Setup L2cache */
    ret = l2cache_setup();
    if (ret != 0){
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
