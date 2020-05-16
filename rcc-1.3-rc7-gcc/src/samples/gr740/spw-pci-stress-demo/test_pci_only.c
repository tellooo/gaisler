#include <rtems.h>

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
#define CONFIGURE_SMP_MAXIMUM_PROCESSORS   4
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


#define TEST_LENGTH_SEC (5)
#define TASK_PRIO 10
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

#define DEMO_PROFILING

#ifdef TEST_NGMP
/* Address DMA regions setup has 8 different configurations:
 *
 * CFG1 - demo coherent system without L2-cache
 *   MMU disabled
 *   L2 cache disabled
 *   Masters via CPU-bus
 *
 * CFG2 - demo coherent system with L2-cache
 *   MMU disabled
 *   L2 cache enabled
 *   Masters via CPU-bus
 *
 * CFG3 - demo coherent MMU system without L2-cache
 *   MMU enabled
 *   L2 cache disabled
 *   Masters via CPU-bus
 *
 * CFG4 - demo coherent MMU system with L2-cache
 *   MMU enabled
 *   L2 cache enabled
 *   Masters via CPU-bus
 *
 * CFG5 - demo coherent system with L2-cache, DMA not cached
 *   MMU disabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters via CPU-bus
 *
 * CFG6 - demo coherent MMU system with L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters via CPU-bus
 *
 * CFG7 - demo non-coherent MMU system without L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache disabled
 *   Masters direct into MEM-bus
 *
 * CFG8 - demo non-coherent MMU system with L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters direct into MEM-bus
 *
 *
 * NOTE that bootloader/GRMON must configure stack 0x00ffffff to avoid conflict
 *      with DMA region 0x01000000 - 0x07ffffff. See stack GRMON comand
 *
 * NOTE Normal RTEMS behaviour is CFG2, or CFG8 when I/O trafic redirected
 */
int dma_cfg = 2; /* Config (override from GRMON with "wmem dma_cfg 8") */
#endif
#ifdef TEST_NGMP
#include "mmu_setup.h"
#endif

rtems_id pcitid;
rtems_id tid_caller;

void pci_demo_os_task_setup(void);
void test_app(void);

rtems_task Init(
  rtems_task_argument ignored
)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	drvmgr_print_topo();

	pci_demo_os_task_setup();

	exit(0);
}

void pci_demo_os_task_setup(void)
{
	rtems_task_create(
			rtems_build_name( 'T', 'P', 'C', 'I' ),
			TASK_PRIO, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &pcitid);
	rtems_task_ident(RTEMS_SELF, RTEMS_SEARCH_ALL_NODES, &tid_caller);
    /* Start PCI test task */
	rtems_task_start(pcitid, task_pci_test, 0);
	test_app();
}

#ifdef DEMO_PROFILING
#include <rtems/cpuuse.h>
#endif

void test_app(void)
{
	int shutdown, stop, loopcnt;
	struct timespec t0, t1;

#ifdef TEST_NGMP
	printf("DMA configuration number: %d\n", dma_cfg);
	switch (dma_cfg) {
	case 1:
		address_region_setup(
                MMU_DISABLE, 
                L2C_DISABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	default:
	case 2:
		address_region_setup(
                MMU_DISABLE, 
                L2C_ENABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 3:
		address_region_setup(
                MMU_ENABLE, 
                L2C_DISABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 4:
		address_region_setup(
                MMU_ENABLE, 
                L2C_ENABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 5:
		address_region_setup(
                MMU_DISABLE, 
                L2C_ENABLE | L2C_MTRR_ENABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 6:
		address_region_setup(
                MMU_ENABLE, 
                L2C_ENABLE | L2C_MTRR_ENABLE, 
                IOMMU_SPW_BUS_CPU|IOMMU_PCI_BUS_CPU|IOMMU_PREFETCH_ENABLE);
		break;
	case 7:
		address_region_setup(
                MMU_ENABLE, 
                L2C_DISABLE | L2C_MTRR_DISABLE, 
                IOMMU_SPW_BUS_MEM|IOMMU_PCI_BUS_MEM|IOMMU_PREFETCH_DISABLE);
		break;
	case 8:
		address_region_setup(
                MMU_ENABLE, 
                L2C_ENABLE | L2C_MTRR_ENABLE, 
                IOMMU_SPW_BUS_MEM|IOMMU_PCI_BUS_MEM|IOMMU_PREFETCH_DISABLE);
		break;
	}
#endif

#ifdef DEMO_PROFILING
	rtems_cpu_usage_reset();
#endif

	shutdown = 0;
	loopcnt = 0;
	rtems_clock_get_uptime(&t0);
	pci_test_start();
	while ( shutdown == 0 ) {
		/* Check if total seconds has gone now and then do stop sequence */
		if ( (stop == 0) && ((loopcnt & 0x7) == 0x7) ) {
			rtems_clock_get_uptime(&t1);
			if ( t1.tv_sec > (t0.tv_sec + TEST_LENGTH_SEC) ) {
				stop = 1;
				pci_test_stop();
				break;
			}
		}
		loopcnt++;
	}

	/* Stop time */
	rtems_clock_get_uptime(&t1);

#ifdef DEMO_PROFILING
	rtems_cpu_usage_report();
#endif

	/* Print PCI test results */
	pci_test_print_results();

	pci_test_task_stop();

	/* Let the other tasks shutdown, timeout must be longer than
	 * dma_rx_wait timeout plus time to cleanup.
	 */
	rtems_task_wake_after(40);

	return;
}
