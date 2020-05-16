#include <rtems.h>
#include <assert.h>
/* configuration information */

#define CONFIGURE_INIT

rtems_task Init (rtems_task_argument argument);
rtems_isr handleTrap (rtems_vector_number vector);

#include <bsp.h> /* for device driver prototypes */

rtems_task Init (rtems_task_argument argument);
rtems_isr handleExternalIrq (rtems_vector_number vector);

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
 #endif

 #include <drvmgr/drvmgr_confdefs.h>
#endif


#include <stdlib.h>
#include <stdio.h>

unsigned int address = 0x00001;

rtems_task
Init
(
    rtems_task_argument argument
)
{
    rtems_status_code status;
    rtems_isr_entry old_handle;
    int tmp = 0;

    status = rtems_interrupt_catch (handleTrap, 
        SPARC_SYNCHRONOUS_TRAP(0x07), &old_handle);
    assert(status == RTEMS_SUCCESSFUL);
    status = rtems_interrupt_catch (handleTrap, 
    	SPARC_SYNCHRONOUS_TRAP(0x90), &old_handle);
    assert(status == RTEMS_SUCCESSFUL);

    tmp = *((volatile int *) address); /* cause trap 0x07 */

    asm ("ta 0x10");		/* cause trap 0x90 */

    exit(tmp);
}

rtems_isr
handleTrap
(
    rtems_vector_number vector
)
{
    unsigned int trap = SPARC_REAL_TRAP_NUMBER(vector);
    printk ("Caught synchronous trap with vector 0x%02x\n", trap);
}				/* handleTrap */

