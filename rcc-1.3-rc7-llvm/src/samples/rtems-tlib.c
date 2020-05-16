/*
 * A RTEMS sample application using the timer library TLIB provided by the
 * LEON3 BSP.
 *
 * The timer library access the LEON3/4 onchip GPTIMERs and GRLIB PCI board's
 * GPTIMERs (AMBA-over-PCI). With AMBA-over-PCI (as in many RASTA systems)
 * LEON2 can also use TLIB to access timers, currently the LEON2 timers are not
 * supported by TLIB.
 *
 * The sample has been run on GR712RC LEON3 and AT697(LEON2)+GR-RASTA-IO. When
 * the system clock tick is 100ticks/sec the resulting number of ISR calls
 * (timer ticks) shuold be 49,50 or 51.
 *
 * The timer prescaler of the timers are configured using driver resources
 * from the config*.c files.
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
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF    /* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_ADCDAC    /* GR-RASTA-ADCDAC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_LEON4_N2X       /* GR-CPCI-LEON4-N2X has multiple timers */

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif

/* Add timer driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "config.c"

#include <grlib/tlib.h>

/* GPTIMER driver structure */
extern struct amba_drv_info gptimer_drv_info;

/* Determine which timer to use in test
 *
 * -2 = Try all timers in system (GRMON: wmem timer_index 0xfffffffe)
 * -1 = Auto select one timer
 * 0..N = Use timer N
 */
int timer_index = -1;

/* Number of Timer IRQs */
volatile int timer_irq_count;

void timer_test_isr(void *data)
{
	timer_irq_count++;
}

int timer_test(int tidx)
{
	int i, cnt, ntimers = tlib_ntimer();
	void *handle;
	unsigned int basefreq, tickrate, counter;

	if (ntimers < 1) {
		puts("No TLIB timers in system");
		return -1;
	}

	printf("%d Timers in system\n", ntimers);

	if (tidx == -1) {
		/* Take first timer available. Timer0 is usually the System
		 * Clock timer.
		 */
		handle = NULL;
		for (i=0; i<ntimers; i++) {
			handle = tlib_open(i);
			if (handle != NULL)
				break;
		}
		if (handle == NULL) {
			puts("Failed to find any free timers");
			return -2;
		}
		tidx = i;
	} else {
		handle = tlib_open(tidx);
		if (handle == NULL) {
			printf("Failed to open timer%d\n", tidx);
			return -3;
		}
	}
	printf("Opened Timer%d for testing\n", tidx);

	tlib_get_freq(handle, &basefreq, &tickrate);
	tlib_get_counter(handle, &counter);
	printf("Base frequency: %uHz\n", basefreq);
	printf("Current tickrate: %uHz\n", tickrate);
	printf("Current counter value: %u\n", counter);

	printf("\n Resetting timer ...\n\n");

	tlib_reset(handle);
	tlib_get_freq(handle, &basefreq, &tickrate);
	tlib_get_counter(handle, &counter);
	printf("Base frequency: %uHz\n", basefreq);
	printf("Current tickrate: %uHz\n", tickrate);
	printf("Current counter value: %u\n", counter);

	/* Try setting tick rate to 10 ticks per second */
	tickrate = basefreq / 10;
	printf("Set %u clock \"clicks\" per tick (10 ticks/sec)\n",
		tickrate);

	if (tlib_set_freq(handle, tickrate)) {
		puts("Failed to set requested timer tick rate (10ticks/sec)");
		return -5;
	}

	/* Do one tick only */
	puts("Waiting for one tick");
	fflush(stdout);
	tlib_start(handle, 1);
	rtems_task_wake_after(50); /* more than 1 timer tick */
	tlib_get_counter(handle, &counter);
	printf("Current counter value: %u\n", counter);

	/* Register a IRQ handler for every timer tick. The ISR will be
	 * given the timer handle as first argument
	 */
	tlib_irq_register(handle, timer_test_isr, handle, 0);
	timer_irq_count = 0;

	puts("Testing Interrupt routine over 500 System clock ticks");

	/* Start Timer repetitive */
	tlib_start(handle, 0);
	rtems_task_wake_after(500); /* wait 5s when 100 System ticks/sec */
	tlib_stop(handle);
	cnt = timer_irq_count; /* Sample current value */
	if (cnt == 0) {
		puts("Timer did not generate interrupt calls to ISR");
		return -10;
	}
	puts("Timer is now stopped");
	tlib_get_counter(handle, &counter);
	tlib_get_freq(handle, NULL, &tickrate);
	printf("Current counter value: %u\n", counter);
	printf("Current tickrate: %u\n", tickrate);
	printf("Number of interrupts (ticks): %u\n", cnt);
	fflush(stdout);

	/* Check that timer has really been stopped */
	rtems_task_wake_after(40); /* wait 400ms when 100 System ticks/sec */
	if (cnt != timer_irq_count) {
		puts("Timer still generates interrupt calls to ISR after stop");
		return -11;
	}

	tlib_irq_unregister(handle);
	tlib_close(handle);

	return 0; /* Success */
}

rtems_task Init(
  rtems_task_argument ignored
)
{
	struct drvmgr_dev *pdev;
	int i, j, status;

	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	drvmgr_print_topo();

	if (pci_bus_count() > 0) {
		printf("\n\n ####### PCI CONFIGURATION #######\n\n");

		/* Print PCI bus resources */
		pci_print();
	} else {
		printf("\n ### NO PCI BUS PRESENT\n\n");
	}

	/* List information about the GPTIMER driver */
	drvmgr_info(&gptimer_drv_info, OPTION_INFO_ALL);

	/* List information about all GPTIMER devices */
	i = 0;
	while (drvmgr_get_dev(&gptimer_drv_info.general, i, &pdev) == 0) {
		puts("");
		puts("");
		drvmgr_info(pdev, OPTION_INFO_ALL);
		i++;
	}

	/* Start demo */
	puts("");
	puts("");

	if (timer_index == -2) {
		puts("TEST ALL TIMERS IN SYSTEM");
#ifdef LEON3
		puts("NOTE THAT IT IS EXPECTED THAT TIMER0 TEST WILL FAIL");
		puts("SINCE THE TIMER IS ALREADY USED AS SYSTEM CLOCK");
#endif
		puts("");
		puts("");

		/* Test all timers in TLIB */
		for (j = tlib_ntimer() - 1; j >= 0; j--) {
			status = timer_test(j);
			if (status) {
				printf("Timer %d test failed: %d\n", j, status);
			} else {
				printf("Timer %d test OK\n", j);
			}
			puts("");
			puts("");
		}
	} else {
		status = timer_test(timer_index);
		if (status)
			printf("Timer test failed %d\n", status);
		else
			printf("Timer test OK\n");
	}

	exit( 0 );
}
