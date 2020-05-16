/*
 * A simple rtems program to show how to use configuration templates
 */

#include <rtems.h>
/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS             4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS	32


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
   #include <drvmgr/ambapp_bus.h>
   /* APBUART0 */
   struct drvmgr_key grlib_drv_res_apbuart0[] =
   {
   	{"mode", DRVMGR_KT_INT, {(unsigned int)1}},
   	{"syscon", DRVMGR_KT_INT, {(unsigned int)0}},
   	DRVMGR_KEY_EMPTY
   };
   /* APBUART1 */
   struct drvmgr_key grlib_drv_res_apbuart1[] =
   {
  	{"mode", DRVMGR_KT_INT, {(unsigned int)1}},
  	{"syscon", DRVMGR_KT_INT, {(unsigned int)1}},
  	DRVMGR_KEY_EMPTY
   };
   /* LEON3 System with driver configuration for 2 APBUARTs, the
    * the rest of the AMBA device drivers use their defaults.
    */
   struct drvmgr_bus_res grlib_drv_resources =
   {
     .next = NULL,
     .resource = {
   	/* DRIVER_ID, DEVICE_INSTANCE, CONFIGURATION_ARRAY */
   	{DRIVER_AMBAPP_GAISLER_APBUART_ID, 0, &grlib_drv_res_apbuart0[0]},
   	{DRIVER_AMBAPP_GAISLER_APBUART_ID, 1, &grlib_drv_res_apbuart1[0]},
   	DRVMGR_RES_EMPTY
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

/* Control baud rate */
#define BAUD_RATE B38400

#if 1
/* Print Hello World on all consoles and debug console */
rtems_task Init(
  rtems_task_argument ignored
)
{
	int fd, len;
	struct termios term;
	char console;
	char buf[50];

	printf( "\nHello World on System Console\n\n" );
	fflush(NULL);
	printk( "\nHello World on Debug Console\n\n" );

	for(console='b'; console<'z'; console++) {

		sprintf(buf, "/dev/console_%c", console);
		fd = open(buf, O_RDWR);
		if ( fd < 0 ) {
			printf("Failed to open %s.\nNumber of consoles available: %d\nCause open failed, ERRNO: %d = %s\n",buf,(console-'b')+1,errno,strerror(errno));
			break;
		}

		/* Get current configuration */
		tcgetattr(fd, &term);

		/* Set Console baud, default is 38400 */
		cfsetospeed(&term, BAUD_RATE);
		cfsetispeed(&term, BAUD_RATE);

		/* Update driver's settings */
		tcsetattr(fd, TCSANOW, &term);

		len = sprintf(buf, "\n\nHello World on /dev/console_%c\n\n", console);
		write(fd, buf, len);
		fflush(NULL);
	}

	exit( 0 );
}

#else

/* Setup loop back UART connections. Designed fo system with 5 UARTs where
 * four UARTs (2 pairs) are used in test below.
 */
int loop_back_uarts['z'-'a'] = 
{ 
	-1,	/* UART0: System Console not in loop back */
	2,	/* UART1: connected to UART2 */
	1,	/* UART2: connected to UART1 */
	4,	/* UART3: connected to UART4 */
	3,	/* UART4: connected to UART3 */
	-1	/* UART5: not in loop back */
};

/* Test all console devices by read/write to them */
rtems_task Init(
  rtems_task_argument ignored
)
{
	int fds['z'-'a'], len, n_consoles;
	struct termios term;
	char console;
	char buf[50], bufRx[50];
	int fd, fdTx, fdRx, i;
	int iterations, console_nr, loop_back_uart;

	printf( "\nHello World on System Console\n\n" );
	fflush(NULL);
	printk( "\nHello World on Debug Console\n\n" );

	n_consoles = 1; /* assume system console */
	for(console='b'; console<'z'; console++) {
		sprintf(buf, "/dev/console_%c", console);
		fd = fds[console-'a'] = open(buf, O_RDWR);
		if ( fd < 0 ) {
			printf("Failed to open %s.\nNumber of consoles available: %d\nCause open failed, ERRNO: %d = %s\n\n\n",buf,(console-'b')+1,errno,strerror(errno));
			break;
		}

		/* Get current configuration */
		tcgetattr(fd, &term);

		/* Set Console baud, default is 38400 */
		cfsetospeed(&term, BAUD_RATE);
		cfsetispeed(&term, BAUD_RATE);

		/* Do not echo chars */
		term.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ECHOPRT|ECHOCTL|ECHOKE);

		/* Turn off flow control */
		term.c_cflag |= CLOCAL;

		/* Update driver's settings */
		tcsetattr(fd, TCSANOW, &term);

		n_consoles++;
	}
	fflush(NULL);

	for(console_nr=0; console_nr<n_consoles; console_nr++) {
		fdTx = fds[console_nr];
		
		loop_back_uart = loop_back_uarts[console_nr];
		if ( loop_back_uart < 0 ) {
			/* Not in use, skip in test */
			printf("Skipping /dev/console_%c\n",console_nr+'a');
			continue;
		}

		printf("/dev/console_%c TX connected to /dev/console_%c RX\n", 'a'+console_nr, 'a'+loop_back_uart);

		fdRx = fds[loop_back_uart];
		len = sprintf(buf, "Hello World on /dev/console_%c\n\n", 'a'+console_nr);
		for(i=0; i<len; i++) {
			write(fdTx, &buf[i], 1);
			fflush(NULL);
			iterations = 0;
			while(read(fdRx, &bufRx[i], 1) < 1) {
				rtems_task_wake_after(1);
				iterations++;
				if ( iterations > 9 ) {
					printf("Did not get char even after 10 ticks (UART%d -> UART%d)\n",console_nr,loop_back_uart);
					exit(0);
				}
			}
			if ( bufRx[i] != buf[i] ) {
				printf("Unexpected char, got 0x%x expected 0x%x\n",(unsigned int)bufRx[i],(unsigned int)buf[i]);
				exit(0);
			}
		}
		printf("Test OK.\n");
		fflush(NULL);
	}

	exit(0);
}
#endif
