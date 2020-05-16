/*
 * This file is the SP part of the SP/SMP demonstration. It is created using
 * RTEMS (no SMP) but could also be created with some other OS/toolchain.
 */
#include <rtems.h>
#include <stdio.h>
#include <time.h>

#ifndef CFG_ISMASTER
 #error CFG_ISMASTER not set
#endif

const int STARTNEXT = CFG_ISMASTER;

/* functions */

rtems_task Init(
  rtems_task_argument argument
);

/* configuration information */

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
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
  struct drvmgr_bus_res grlib_drv_resources =
  {
    .next = NULL,
    .resource = {
    {DRIVER_AMBAPP_GAISLER_APBUART_ID, 1, NULL}, /* Do not use APBUART1 */
    {DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 1, NULL}, /* Do not use GPTIMER1 */
    DRVMGR_RES_EMPTY
    }
  };
 #endif

 #include <drvmgr/drvmgr_confdefs.h>
#endif

void printloop(void)
{
  time_t ptime = 1234;

  while(1) {
    time_t now;
    char *c;

    do {
      now = time(NULL);
    } while (ptime == now);
    c = ctime(&now);
    printf("SP   - time - %s", c);
    ptime = now;
  }
}

void startmanually(void) {
  printf("SP:   - Waking up CPU[1]...\n");
  /* ...using multiprocessor status register of interrupt controller 1. */
  *(volatile unsigned int *) 0xff905010 = (1<<1);
  rtems_task_wake_after( 2 * rtems_clock_get_ticks_per_second() );
}

rtems_task Init(
  rtems_task_argument argument
)
{
  (void) sizeof argument;

  if (STARTNEXT) {
    printf("SP:    I am the master\n");
    startmanually();
  } else {
    printf("SP:    I am the slave\n");
  }
  printloop();
}

