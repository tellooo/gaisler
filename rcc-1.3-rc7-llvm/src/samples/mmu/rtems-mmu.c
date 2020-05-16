/* LEON SRMMU example. See README for an overview */

#include <rtems.h>
#include <rtems/bspIo.h>

#define CONFIGURE_INIT
#include <bsp.h> /* for device driver prototypes */

rtems_task Init(rtems_task_argument argument);	/* forward declaration needed */

/* configure RTEMS kernel */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             24
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (64 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_INIT_TASK_PRIORITY	100
#define CONFIGURE_MAXIMUM_DRIVERS 16

#include <rtems/confdefs.h>

/* We require only UART and Timer */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER

/* Configure Driver Manager */
#include <drvmgr/drvmgr_confdefs.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <ctype.h>
#include <bsp.h>
#include <stdio.h>
#include <stdlib.h>

/* Include driver configurations and system initialization */
#include "config.c"

extern void mmu_setup(void);

/* Called from srmmu_bad_trap() trap handler */
void trap_returnfunc(int num)
{
	printf(" CALL test should end up in this function instead\n");
}

/* Test for CPU0, try to access different areas testing the MMU protection.
 * RTEMS[1] running on CPU[1] is protected from this.
 */
void test_mmu_1(void)
{
	unsigned int value;
	void (*f)(void (*retfun)(int));
	void (*retfun)(int num);

	/* Test[00] Read MMU Tables (no access) */
	printf("\n\nTest[00]: reading 0x40380000 (MMU Tables, should not be allowed)\n");
	rtems_task_wake_after(20);
	fflush(NULL);
	value = 0;
	value = *(volatile unsigned int *)0x40380000;
	fflush(NULL);
	printf("Read[0x40380000]: 0x%08x\n", value);

	/* Test[01] Write MMU Tables (no access) */
	printf("\n\nTest[01]: write 0x40381000 (MMU Tables, should not be allowed)\n");
	rtems_task_wake_after(20);
	fflush(NULL);
	*(volatile unsigned int *)0x40381000 = 0x12345678;
	fflush(NULL);

	/* Test[02] Read PROM region (R) */
	printf("\n\nTest[02]: reading 0x00000000 (PROM, should be allowed)\n");
	rtems_task_wake_after(20);
	fflush(NULL);
	value = 0;
	value = *(volatile unsigned int *)0x00000000;
	fflush(NULL);
	printf("Read[0x02200000]: 0x%08x\n", value);

	/* Test[03] Write PROM region (R) */
	printf("\n\nTest[03]: write 0x00000000 (PROM, should not be allowed)\n");
	rtems_task_wake_after(20);
	fflush(NULL);
	*(volatile unsigned int *)0x00000000 = 0x12345678;
	fflush(NULL);

	/* Test[04] JUMP to Registers region 0x80000000 (R/W)
	 *
	 * Note that the trap handler use reg.i0 as an address to jump to
	 * (which is a bit odd, but this is just an example..). 
	 */
	printf("\n\nTest[04]: 0x40300000 (call read-only area should not be allowed)\n");
	rtems_task_wake_after(20);
	fflush(NULL);
	f = (void *)0x80000000;
	retfun = trap_returnfunc;
	f(retfun);
	fflush(NULL);
}

/* ========================================================= 
 * initialisation - entry point
 */
rtems_task Init(
  rtems_task_argument ignored
)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	printf("******** Running SRMMU test ********\n\n");

	/* Set up MMU contexts and tables, initialize and enable MMU
	 * registers
	 */
	mmu_setup();

	/* Run MMU test to test MMu table set up */
	test_mmu_1();

	printf("\n\n\n  CPU MMU TEST DONE\n\n\n");

	exit(0);
}
