/* add drvmgr initialization arrays */
#include <drvmgr/drvmgr_confdefs.h>
#include <rtems.h>
#include <rtems.h>
#include <sys/stat.h>

#ifdef CONFIGURE_INIT

extern rtems_device_driver console_initialize(rtems_device_major_number,rtems_device_minor_number,void *);
extern rtems_device_driver console_open(rtems_device_major_number,rtems_device_minor_number,void *);
extern rtems_device_driver console_close(rtems_device_major_number,rtems_device_minor_number,void *);
extern rtems_device_driver console_read(rtems_device_major_number,rtems_device_minor_number,void *);
extern rtems_device_driver console_write(rtems_device_major_number,rtems_device_minor_number,void *);
extern rtems_device_driver console_control(rtems_device_major_number,rtems_device_minor_number,void *);

rtems_driver_address_table Device_drivers[] = {
#ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  { console_initialize, console_open, console_close, console_read, console_write, console_control, _IFCHR | _IRTEMSEMUL }
#endif
};

int Device_drivers_cnt = sizeof(Device_drivers) / sizeof(rtems_driver_address_table);

#endif /* CONFIGURE_INIT */
