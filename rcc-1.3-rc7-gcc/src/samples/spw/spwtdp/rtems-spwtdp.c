/*
 * A RTEMS sample application using the RTEMS shell, one can use this
 * sample to find information about the current system and it's driver
 * configuration using the drvmgr and pci commands, try:
 *  # drvmgr --help
 *  # pci --help
 *
 */


#include <rtems.h>

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_MAXIMUM_TASKS             20
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_DRIVERS 32

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (24 * RTEMS_MINIMUM_STACK_SIZE)

/* PCI auto configuration library */
#define RTEMS_PCI_CONFIG_LIB
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO
/*
 #define CONFIGURE_PCI_LIB PCI_LIB_STATIC
 #define CONFIGURE_PCI_LIB PCI_LIB_READ
*/

#include <rtems/confdefs.h>


/* Configure Driver manager */
#if defined(RTEMS_DRVMGR_STARTUP) && defined(LEON3) /* if --drvmgr was given to configure */
 /* Add UART Driver for this example */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
 #endif
#endif
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF    /* GRLIB PCIF Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW2	/* SpaceWire Packet driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_AHBSTAT

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
  /*#define ENABLE_NETWORK_SMC_LEON2*/
#endif

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>

/* no network in this example */
#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3
#undef ENABLE_NETWORK_SMC_LEON2

#include "config.c"

extern int example_init(void);
extern int example_run(void);

rtems_task Init(
  rtems_task_argument ignored
)
{
	int status;

	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */	
	drvmgr_print_topo();

	/* Set object_id from GRMON in order print info about a device, the
	 * object ID can be seen in the topology printout.
	 */
	if (object_id != NULL)
		drvmgr_info(object_id, OPTION_INFO_ALL);

	/* Print PCI configuration */
	/*pci_cfg_print();*/

	status = example_init();
	if (status) {
		printf("Example initialization failed %d\n", status);
	}

	status = example_run();
	if (status) {
		printf("Example start failed %d\n", status);
	}

	rtems_task_delete(RTEMS_SELF);
}

/****** CONFIGURATION OF SPWTDP EXAMPLE *****
 *
 *
 *
 */
#define R_SPWTDP_BASEADR 0x80100700	/* Base address of SPWTDP on Remote */
#define R_SPWTDP_SPWDEV  0		/* SpaceWire device connector */
#define R_SPWTDP_MAPPING 6		/* Mapping of Time-Code to T-Field */

/* Initialization of software example
 *  A. Initialize SpW interface and DMA task
 *  B. Setup RMAP stack for access to remote system
 *  C. 
 */
int example_init(void)
{


	return 0;
}
