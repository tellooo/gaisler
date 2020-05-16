/*
 * A RTEMS sample application using the RTEMS shell, one can use this
 * sample to find information about the current system and it's driver
 * configuration using the drvmgr and pci commands, try:
 *  # drvmgr --help
 *  # pci --help
 *
 */

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3
#undef ENABLE_NETWORK_SMC_LEON2

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
#define CONFIGURE_MAXIMUM_POSIX_KEYS 4

#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
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

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
#define CONFIGURE_SHELL_COMMAND_DRVMGR
#define CONFIGURE_SHELL_COMMAND_PCI


#include <rtems/shellconfig.h>


/* Configure Driver manager */
#if defined(RTEMS_DRVMGR_STARTUP) && defined(LEON3) /* if --drvmgr was given to configure */
 /* Add UART Driver for this example */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
 #endif
#endif
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF    /* GRLIB PCIF Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_ADCDAC    /* GR-RASTA-ADCDAC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_SPW_ROUTER/* GR-RASTA-SPW-ROTER PCI Peripheral Driver */
#define CONFIGURE_DRIVER_PCI_GR_LEON4_N2X       /* GR-CPIC-LEON4-N2X PCI Peripheral Driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW2
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_OCCAN
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRGPIO
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_AHBSTAT
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GR1553B
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GR1553BC
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GR1553BM
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GR1553RT
#ifdef ENABLE_NETWORK
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRETH   /* This driver works for LEON2 also.. */
#endif

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
#include <string.h>
#include <errno.h>
#include <termios.h>

#include <rtems/shell.h>

#include "config.c"

void *object_id = NULL;

extern void pci_cfg_print(void);

void writeFile(
  const char *name,
  int         mode,
  const char *contents
)
{
  int sc;
  sc = setuid(0);
  if ( sc ) {
    printf( "setuid failed: %s:\n", strerror(errno) );
  }

  rtems_shell_write_file( name, contents );

  sc = chmod ( name, mode );
  if ( sc ) {
    printf( "chmod %s: %s:\n", name, strerror(errno) );
  }
}


void init_filesystem(void)
{
  int sc;

  sc = mkdir("/etc", 0777);
  if ( sc ) {
    printf( "mkdir /etc: %s:\n", strerror(errno) );
  }

  printf(
    "Creating /etc/passwd and group with three useable accounts\n"
    "root/pwd , test/pwd, rtems/NO PASSWORD\n"
  );

  writeFile(
    "/etc/passwd",
    0644,
    "root:7QR4o148UPtb.:0:0:root::/:/bin/sh\n"
    "rtems:*:1:1:RTEMS Application::/:/bin/sh\n"
    "test:8Yy.AaxynxbLI:2:2:test account::/:/bin/sh\n"
    "tty:!:3:3:tty owner::/:/bin/false\n"
  );
  writeFile(
    "/etc/group",
    0644,
    "root:x:0:root\n"
    "rtems:x:1:rtems\n"
    "test:x:2:test\n"
    "tty:x:3:tty\n"
  );

}

rtems_task Init(
  rtems_task_argument ignored
)
{
	struct termios term;

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

	printf(" You can use the shell commands drvmgr and pci to find out\n"
	       " more about the system\n\n");

	init_filesystem();

	/* Read out the default parameters of the APBUART driver, then
	 * set them. This is to let the termios layer know about the current
	 * baud rate
	 */
	tcgetattr(STDIN_FILENO, &term);
	tcsetattr(STDIN_FILENO, TCSADRAIN, &term);
	fflush(NULL);

	rtems_shell_init(
		"SHLL",
		RTEMS_MINIMUM_STACK_SIZE*4,
		10,
		"dev/console",
		0,
		1,
		NULL
	);

	rtems_task_delete(RTEMS_SELF);
}
