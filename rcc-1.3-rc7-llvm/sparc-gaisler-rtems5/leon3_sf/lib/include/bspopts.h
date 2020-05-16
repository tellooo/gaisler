/* BSP dependent options file */
/* automatically generated -- DO NOT EDIT!! */

#ifndef __BSP_OPTIONS_H
#define __BSP_OPTIONS_H

/* include/bspopts.tmp.  Generated from bspopts.h.in by configure.  */
/* bspopts.h.in.  Generated from configure.ac by autoheader.  */

/* If defined, then the BSP Framework will put a non-zero pattern into the
   RTEMS Workspace and C program heap. This should assist in finding code that
   assumes memory starts set to zero. */
#define BSP_DIRTY_MEMORY 0

/* Always defined when on a LEON3 to enable the LEON3 support for determining
   the CPU core number in an SMP configuration. */
#define BSP_LEON3_SMP 1

/* If defined, CPU is powered down on fatal exit. Otherwise generate system
   error which will hand over to debugger, simulator, etc. */
/* #undef BSP_POWER_DOWN_AT_FATAL_HALT */

/* If defined to a non-zero value, print a message and wait until pressed
   before resetting board when application exits. */
#define BSP_PRESS_KEY_FOR_RESET 0

/* If defined to a non-zero value, prints the exception context when an
   unexpected exception occurs. */
#define BSP_PRINT_EXCEPTION_CONTEXT 1

/* If defined to a non-zero value, reset the board when the application exits.
   */
#define BSP_RESET_BOARD_AT_EXIT 1

/* If defined to a non-zero value, prints the some information in case of a
   fatal error. */
#define BSP_VERBOSE_FATAL_EXTENSION 1

/* The leon3 console driver can operate in either polled or interrupt mode.
   Under the simulator (especially when FAST_UART is defined), polled seems to
   operate better. */
#define CONSOLE_USE_INTERRUPTS 0







/* Defines the version of RCC */
#define RCC_VERSION "1.3-rc7"

/* The RTEMS BSP name */
#define RTEMS_BSP leon3_sf

#endif /* __BSP_OPTIONS_H */
