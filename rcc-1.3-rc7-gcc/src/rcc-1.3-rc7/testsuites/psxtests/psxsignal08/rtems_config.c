/*
 * COPYRIGHT (c) 2019.
 * On-Line Applications Research Corporation (OAR).
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <rtems.h>
#include <stdlib.h>

int main(int argc, char **argv);

static char *argv_list[] = {
  "report",
  ""
};
static rtems_task Init(rtems_task_argument arg)
{
  (void) arg;  /* deliberately ignored */

  /*
   * Initialize optional services
   */

  /*
   * Could get arguments from command line or have a static set.
   */
  (void) main(1, argv_list);

  exit(0);
}

#include <bsp.h> /* for device driver prototypes */

/* NOTICE: the clock driver is explicitly disabled */
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
