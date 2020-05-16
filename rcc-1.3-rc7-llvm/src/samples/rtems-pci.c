/*
 * A RTEMS sample application using the PCIF, GRPCI, GRPCI2 or AT697 PCI driver
 *
 * The example adds a custom PCI driver to the driver manager, the PCI
 * hardware ID must be changed in order for the driver to detect 
 * the board: PCIID_VENDOR_CUSTOM and PCIID_DEVICE_CUSTOM
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

/* Set some values if someone should modify the example. The shared IRQ layer
 * need one semaphore.
 */
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_DRIVERS 32

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

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
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_ADCDAC    /* GR-RASTA-ADCDAC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_LEON4_N2X       /* GR-CPCI-LEON4-N2X PCI Peripheral Driver */
#define CONFIGURE_DRIVER_PCI_GR_CPCI_GR740      /* GR-CPCI-GR740 PCI Peripheral Driver */


/******** ADD A CUSTOM PCI DRIVER **********/
/* Uncomment the line below to add the custom driver, don't forget to 
 * set PCIID_VENDOR_CUSTOM and PCIID_DEVICE_CUSTOM
 */
//#define CONFIGURE_DRIVER_CUSTOM1

#define DRIVER_CUSTOM1_REG custom_pci_board_register_drv
void custom_pci_board_register_drv(void);
/*******************************************/

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif

#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "config.c"

void *object_id = NULL;

extern void pci_cfg_print(void);

rtems_task Init(
  rtems_task_argument ignored
)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	drvmgr_print_topo();

	printf("\n\n ####### PCI CONFIGURATION #######\n\n");

	/* Print PCI bus resources */
	pci_print();

#if 0
#ifdef LEON3
	printf("\n\n ####### AMBA PnP CONFIGURATION #######\n\n");

	/* Print AMBA Bus */
	*ambapp_print(&ambapp_plb, 1);
#endif
#endif

	drvmgr_info_devs(OPTION_INFO_ALL);

	/* Set object_id from GRMON in order print info about a device */
	if (object_id)
		drvmgr_info(object_id, OPTION_INFO_ALL);

	/* Print Current configuration in C-code to terminal */
	/*pci_cfg_print();*/

	exit( 0 );
}

#if 1
/********** A CUSTOM PCI DRIVER ***********/
#include <pci.h>
#include <drvmgr/pci_bus.h>

/* CHANGE THIS !!! to match the DEVIC/VENDOR of your PCI board */
#define PCIID_VENDOR_CUSTOM 1
#define PCIID_DEVICE_CUSTOM 1

int custom_pci_board_init1(struct drvmgr_dev *dev);
int custom_pci_board_init2(struct drvmgr_dev *dev);
#define DRIVER_PCI_VENDOR_DEVICE_ID	DRIVER_PCI_ID(PCIID_VENDOR_CUSTOM, PCIID_DEVICE_CUSTOM)

struct drvmgr_drv_ops custom_pci_board_ops =
{
    .init = {custom_pci_board_init1, custom_pci_board_init2},
    .remove = NULL,
    .info = NULL
};

struct pci_dev_id_match custom_pci_board_ids[] =
{
    PCIID_DEVVEND(PCIID_VENDOR_CUSTOM, PCIID_DEVICE_CUSTOM),
    PCIID_END_TABLE        /* Mark end of table */
};

struct pci_drv_info custom_pci_board_info =
{
    {
    	DRVMGR_OBJ_DRV,
        NULL,                        /* Next driver */
        NULL,                        /* Device list */
        DRIVER_PCI_VENDOR_DEVICE_ID, /* Driver ID */
        "CUSTOM_PCI_DRV",            /* Driver Name */
        DRVMGR_BUS_TYPE_PCI,         /* Bus Type */
        &custom_pci_board_ops,       /* Driver operations */
	NULL,                        /* No custom Functions */
        0,                           /* No devices yet */
	0,                           /* DrvMgr need not to allocate private */
    },
    &custom_pci_board_ids[0]
};

void custom_pci_board_register_drv(void)
{
	printk("Registering CUSTOM PCI driver\n");
	drvmgr_drv_register(&custom_pci_board_info.general);
}

int custom_pci_board_init1(struct drvmgr_dev *dev)
{
	/* Nothing should be done here, unless we provide other Driver manager buses
	 * or other drivers in init2 depends upon functionality provided by this
	 * driver.
	 *
	 * One may want to reset the hardware here, as early as possible...
	 */

	return DRVMGR_OK;
}

int custom_pci_board_init2(struct drvmgr_dev *dev)
{
	/* Initialize the PCI board hardware and driver, register interrupt
	 * routines, etc.
	 */


	struct pci_dev_info *devinfo;
	uint32_t bars[6];
	int bus, device, func;
	uint8_t irqno;
	pci_dev_t pcidev;

	devinfo = (struct pci_dev_info *)dev->businfo;
	pcidev = devinfo->pcidev;

	bus = PCI_DEV_BUS(pcidev);
	device = PCI_DEV_SLOT(pcidev);
	func = PCI_DEV_FUNC(pcidev);

	printf("CUSTOM PCI @ [%x:%x:%x]\n", bus, device, func);

	memset(bars, 0, sizeof(bars));

	pci_cfg_r32(pcidev, PCIR_BAR(0), &bars[0]);
	pci_cfg_r32(pcidev, PCIR_BAR(1), &bars[1]);
	pci_cfg_r32(pcidev, PCIR_BAR(2), &bars[2]);
	pci_cfg_r32(pcidev, PCIR_BAR(3), &bars[3]);
	pci_cfg_r32(pcidev, PCIR_BAR(4), &bars[4]);
	pci_cfg_r32(pcidev, PCIR_BAR(5), &bars[5]);
	pci_cfg_r8(pcidev, PCIR_INTLINE, &irqno);

	printf("CUSTOM PCI: BAR0: 0x%08"PRIx32"\n", bars[0]);
	printf("CUSTOM PCI: BAR1: 0x%08"PRIx32"\n", bars[1]);
	printf("CUSTOM PCI: BAR2: 0x%08"PRIx32"\n", bars[2]);
	printf("CUSTOM PCI: BAR3: 0x%08"PRIx32"\n", bars[3]);
	printf("CUSTOM PCI: BAR4: 0x%08"PRIx32"\n", bars[4]);
	printf("CUSTOM PCI: BAR5: 0x%08"PRIx32"\n", bars[5]);
	printf("CUSTOM PCI: IRQ:  %d\n", irqno);

	return DRVMGR_OK;
}

/*******************************************/
#endif
