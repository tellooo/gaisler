#ifndef RTEMS_H
#define RTEMS_H

#include "rtemscompat.h"
#include <stddef.h>
#include <string.h>

typedef int rtems_id;

#define RTEMS_SUCCESSFUL 0
#define RTEMS_NO_MEMORY  26     
#define RTEMS_WAIT      0x00000000
#define RTEMS_NO_WAIT   0x00000001
#define RTEMS_NO_TIMEOUT 0
#define RTEMS_DEFAULT_ATTRIBUTES  0x00000000
#define RTEMS_INVALID_NUMBER 10

extern void rtems_fatal_error_occurred(int the_error );
extern char *rtems_build_name(char a0,char a1,char a2,char a3 );
extern int rtems_semaphore_release(rtems_id id);
extern int rtems_semaphore_create(char *name, int count, int attribute_set, int priority_ceiling, rtems_id *id );
extern int rtems_semaphore_obtain(rtems_id id, int option_set, int timeout );

typedef rtems_device_driver ( *rtems_device_driver_entry )(
                 rtems_device_major_number,
                 rtems_device_minor_number,
                 void *
             );

typedef struct {
  rtems_device_driver_entry initialization_entry; /* initialization procedure */
  rtems_device_driver_entry open_entry;        /* open request procedure */
  rtems_device_driver_entry close_entry;       /* close request procedure */
  rtems_device_driver_entry read_entry;        /* read request procedure */
  rtems_device_driver_entry write_entry;       /* write request procedure */
  rtems_device_driver_entry control_entry;     /* special functions procedure */
	int typ; /* leonbare addition */
}   rtems_driver_address_table;

typedef void (*BSP_output_char_function_type)(char c);
typedef char (*BSP_polling_getchar_function_type )(void);
	
#endif /* RTEMS_H */
