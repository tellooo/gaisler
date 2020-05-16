/*
 * A simple rtems program designed to trap if compiled without -msoft-float
 */

#include <rtems.h>
#include <assert.h>
/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS             4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

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

  /* OPTIONAL FOR GRLIB SYSTEMS WITH APBUART AS SYSTEM CONSOLE.
   *
   * Assign a specific UART the system console (syscon=1) and debug output
   * debug_uart_index. Note that the default is to have APBUART0 as
   * system/debug console, one must override it by setting syscon=0 on the
   * first APBUART, then setting syscon=1 on APBUART that should be system
   * console.
   *
   * Note also that the debug console does not have to be on the same UART
   * as the system console.
   *
   * Determine if console driver should do be interrupt driven (mode=1)
   * or polling (mode=0).
   *
   * Example below:
   *   APBUART[0] in Interrupt mode
   *   APBUART[1] in Interrupt mode, System console and Debug console
   *   APBUART[N>=2] in Polling mode (not configured)
   */
  #if 0
   #include <grlib/ambapp_bus.h>
   /* APBUART0 */
   struct drvmgr_key grlib_drv_res_apbuart0[] =
   {
   	{"mode", KEY_TYPE_INT, {(unsigned int)1}},
   	{"syscon", KEY_TYPE_INT, {(unsigned int)0}},
   	DRVMGR_KEY_EMPTY
   };
   /* APBUART1 */
   struct drvmgr_key grlib_drv_res_apbuart1[] =
   {
  	{"mode", KEY_TYPE_INT, {(unsigned int)1}},
  	{"syscon", KEY_TYPE_INT, {(unsigned int)1}},
  	DRVMGR_KEY_EMPTY
   };
   /* LEON3 System with driver configuration for 2 APBUARTs, the
    * the rest of the AMBA device drivers use their defaults.
    */
   struct drvmgr_bus_res grlib_drv_resources =
   {
   	.next = NULL,
   	.resource = {
   	{DRIVER_AMBAPP_GAISLER_APBUART_ID, 0, &grlib_drv_res_apbuart0[0]},
   	{DRIVER_AMBAPP_GAISLER_APBUART_ID, 1, &grlib_drv_res_apbuart1[0]},
   	RES_EMPTY
   	}
   };

   /* Override defualt debug UART assignment.
    * 0 = Default APBUART. APBUART[0], but on MP system CPU0=APBUART0,
    *     CPU1=APBUART1...
    * 1 = APBUART[0]
    * 2 = APBUART[1]
    * 3 = APBUART[2]
    * ...
    */
   int debug_uart_index = 2; /* second UART -- APBUART[1] */
  #endif
 #endif

 #include <drvmgr/drvmgr_confdefs.h>
#endif

#include <stdio.h>
#include <stdlib.h>

 rtems_id   Task_id;         /* array of task ids */
 rtems_name Task_name;       /* array of task names */

float f1 = 0.75;
float f2 = 4.4;

 rtems_task Test_task(
   rtems_task_argument unused
 )
 {
         float fres = f1*f2;
         printf("%f * %f = %f\n", f1, f2, fres);
         exit(0);
 }

rtems_task Init(
  rtems_task_argument ignored
)
{
  rtems_status_code status;
  iprintf( "SOFT FLOAT SAMPLE\n" );
  iprintf( "This application is supposed to enter an error trap when not compiled with -msoft-float\n\n" );

  Task_name = rtems_build_name( 'T', 'A', '1', ' ' );

  status = rtems_task_create(
    Task_name, 1, RTEMS_MINIMUM_STACK_SIZE * 2, RTEMS_DEFAULT_MODES,
    RTEMS_NO_FLOATING_POINT, &Task_id
  );
  assert(status == RTEMS_SUCCESSFUL);

  status = rtems_task_start( Task_id, Test_task, 1 );
  assert(status == RTEMS_SUCCESSFUL);

  status = rtems_task_delete( RTEMS_SELF );
  assert(status == RTEMS_SUCCESSFUL);
}
