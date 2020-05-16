#include <rtems.h>
#include <inttypes.h>

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

/* Set some values if someone should modify the example. The shared IRQ layer
 * need one semaphore.
 */
#define CONFIGURE_MAXIMUM_TASKS             16
#define CONFIGURE_MAXIMUM_SEMAPHORES        32
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_DRIVERS 32
#define CONFIGURE_MAXIMUM_PERIODS             1

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES    RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT
#define CONFIGURE_EXTRA_TASK_STACKS         (40 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MICROSECONDS_PER_TICK     RTEMS_MILLISECONDS_TO_MICROSECONDS(2)

#ifdef RTEMS_SMP
/* Activate SMP mode */
#define CONFIGURE_MAXIMUM_PROCESSORS   4
/* Enable support for CPU affinity */
#define CONFIGURE_SCHEDULER_PRIORITY_AFFINITY_SMP
#endif

/* Configure PCI Library to auto configuration. This can be substituted with
 * a static configuration by setting PCI_LIB_STATIC, see pci/. Static
 * configuration can be generated automatically by print routines in PCI
 * library.
 */
#define RTEMS_PCI_CONFIG_LIB
/*#define CONFIGURE_PCI_LIB PCI_LIB_STATIC*/
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO

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
/*#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF*//* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_CPCI_GR740        /* GR-CPCI-GR740 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPW_ROUTER /* SpaceWire Router  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW2  /* SpaceWire packet driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRIOMMU  /* GRIOMMU driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_L2CACHE  /* L2CACHE driver  */


/*******************************************/

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "config.c"

#include "pci_test.h"

/*** END OF CONFIGURATION OPTIONS - start of test application ***/

rtems_task task_dma_process(rtems_task_argument devidx);

/* Set priority of GRSPW Packet driver worker task lower than application
 * task. When application task sleep the worker may do job.
 */
/*#define GRSPWPKT_WORKTASK_PRIO 11*/
#define GRSPWPKT_WORKTASK_PRIO -1
int grspw_work_task_priority = GRSPWPKT_WORKTASK_PRIO;

#define OS_SEM_TYPE rtems_id
#define OS_SEM_TAKE(_x, _timeout) rtems_semaphore_obtain(_x, RTEMS_WAIT, _timeout)
#define OS_SEM_GIVE(_x) rtems_semaphore_release(_x)
#define OS_SEM_FLUSH(_x) rtems_semaphore_flush(_x)
#define OS_MSGQ_TYPE rtems_id
#define OS_MSGQ_NULL 0
#define OS_MSGQ_SEND rtems_message_queue_send
#define OS_GRSPW_WORK_SPAWN_ERROR OBJECTS_ID_NONE

/*
 * Affinity settings
 * -----------------
 *  - CPU task affinity 
 *  - CPU interrupt affinity
 *
 */
/*#define SETUP_IRQ_AFFINITY_GR712RC*/
#define SETUP_IRQ_AFFINITY_GR740
#define SETUP_CPU_AFFINITY_GR740

rtems_id tids[4];
rtems_id pcitid;
rtems_id tid_caller;
rtems_id period;

rtems_task Init(
  rtems_task_argument ignored
)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	drvmgr_print_topo();

	spacewire_router_demo(0);
#if 0
	printf("\n\n\n\n\n=========== Second Test Round ==========\n\n\n\n\n");
	spacewire_router_demo(0);
#endif

	exit(0);
}

rtems_task grspw_demo_os_spacewire0_dma_task(rtems_task_argument ignored)
{
	test_app();
	spacewire_router_demo_exitcode = 0; /* always say successful - replace with test_app return value */
	rtems_task_resume(tid_caller);
	rtems_task_delete( RTEMS_SELF );
}

void grspw_demo_os_task_setup(void)
{
	/* Run SpaceWire Test application */
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '1' ),
			TASK_PRIO, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tids[0]);
#ifdef BLOCKING_DMA
	for (int i = 1; i < SPW_LINKS_USED; i++) {
		rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '1' + i ),
			TASK_PRIO, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_NO_FLOATING_POINT, &tids[i]);
	}
#endif
	rtems_task_create(
			rtems_build_name( 'T', 'P', 'C', 'I' ),
			TASK_PRIO, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &pcitid);
	rtems_task_ident(RTEMS_SELF, RTEMS_SEARCH_ALL_NODES, &tid_caller);
	rtems_task_start(tids[0], grspw_demo_os_spacewire0_dma_task, 0);
    /* Start PCI test task */
	rtems_task_start(pcitid, task_pci_test, 0);
}

#ifdef DEMO_PROFILING
#include <rtems/cpuuse.h>
#endif

#ifdef RTEMS_SMP
/* Set up LEON BSP Interrupt affinity for SpaceWire IRQs. This configures
 * CPU will handle which Interrupt
 * 
 * By default all IRQs are handled by CPU0. Configure this based on
 * your system's configuration (see GRMON "info sys grspw0" to find IRQ)
 */
#ifdef SETUP_IRQ_AFFINITY_GR712RC /* define this manually */
/* GR712RC has SpW0,SpW1,SpW2,SpW3,SpW4,SpW5=IRQ22,23,24,25,26,27 
 * and two CPUs. To distribute the CPU load:
 *   - SpW0 handled by CPU0,
 *   - SpW1 handled by CPU1,
 *   - SpW2 handled by CPU0,
 *   - ..
 */
const unsigned char LEON3_irq_to_cpu[32] = {
		/* IRQ00..03 */ 0, 0, 0, 0,
		/* IRQ04..07 */ 0, 0, 0, 0,
		/* IRQ08..11 */ 0, 0, 0, 0,
		/* IRQ12..15 */ 0, 0, 0, 0,
		/* IRQ16..19 */ 0, 0, 0, 0,
		/* IRQ20..23 */ 0, 0, 1, 0, /* SpW0, SpW1 IRQs */
		/* IRQ24..27 */ 1, 0, 1, 0, /* SpW2..SpW5 IRQs */
		/* IRQ28..31 */ 0, 0, 0, 0,
};
#endif
#ifdef SETUP_IRQ_AFFINITY_GR740 /* define this manually */
/* GR740 has SpW0,SpW1,SpW2,SpW3=IRQ20,21,22,23 
 * and four CPUs. To distribute the CPU load:
 *   - SpW0 handled by CPU0,
 *   - SpW1 handled by CPU1,
 *   - SpW2 handled by CPU2,
 *   - SpW2 handled by CPU3,
 */
const unsigned char LEON3_irq_to_cpu[32] = {
		/* IRQ00..03 */ 0, 0, 0, 0,
		/* IRQ04..07 */ 0, 0, 0, 0,
		/* IRQ08..11 */ 0, 0, 0, 0,
		/* IRQ12..15 */ 0, 0, 0, 0,
		/* IRQ16..19 */ 0, 0, 0, 0,
		/* IRQ20..23 */ 0, 1, 2, 3, /* SpW0..SpW3 IRQs */
		/* IRQ24..27 */ 0, 0, 0, 0,
		/* IRQ28..31 */ 0, 0, 0, 0,
};
#endif
#ifdef SETUP_CPU_AFFINITY_GR740
/* Setup CPU affinity of work-tasks. One work-task per CPU, same CPU
 * that handles the Interrupt of device will also execute the work-task
 */
#define SETUP_CPU_AFFINITY_WT
void grspw_demo_os_set_affinity_wt(rtems_id tid, int index)
{
	cpu_set_t cpuset;
	if (index < 0 || index >= 4 || index >= SPW_LINKS_USED)
		return;
	CPU_ZERO(&cpuset);
	CPU_SET(index, &cpuset); /* assumes all 4 CPUs are used */
	if (rtems_task_set_affinity(tid, sizeof(cpuset), &cpuset) != RTEMS_SUCCESSFUL) {
		printf(" Failed to set CPU affinity for work-task tid=%08" PRIx32 "\n", tid);
	} else {
		printf(" Set CPU affinity for work-task tid=%08" PRIx32 "\n", tid);
	}
	if (rtems_task_set_affinity(tids[index], sizeof(cpuset), &cpuset) != RTEMS_SUCCESSFUL) {
		printf(" Failed to set CPU affinity for dma-task  tid=%08" PRIx32 "\n", tid);
	} else {
		printf(" Set CPU affinity for dma-task  tid=%08" PRIx32 "\n", tids[index]);
	}
}
#endif
#endif
