/*
 *  Input/Output Manager - Dynamically Register Device Driver
 *
 *  COPYRIGHT (c) 1989-2007.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id: ioregisterdriver.c,v 1.4 2008/09/04 17:46:39 ralf Exp $
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/system.h>
#include <rtems/io.h>

extern int _IO_Manager_drivers_inititalized;

/*
 *  rtems_io_register_driver
 *
 *  Register a driver into the device driver table.
 *
 *  Input Paramters:
 *    major            - device major number (0 means allocate
 *                       a number)
 *    driver_table     - driver callout function table
 *    registered_major - the major number which is registered
 *
 *  Output Parameters:
 *    RTEMS_SUCCESSFUL - if successful
 *    error code       - if unsuccessful
 */

rtems_status_code rtems_io_register_driver(
  rtems_device_major_number   major,
  const rtems_driver_address_table *driver_table,
  rtems_device_major_number  *registered_major
)
{

  /*
   *  Validate the pointer data and contents passed in
   */
  if ( !driver_table )
    return RTEMS_INVALID_ADDRESS;

  if ( !registered_major )
    return RTEMS_INVALID_ADDRESS;

  if ( !driver_table->initialization_entry && !driver_table->open_entry )
    return RTEMS_INVALID_ADDRESS;

  *registered_major = 0;

  /*
   *  The requested major number is higher than what is configured.
   */
  if ( major >= _IO_Number_of_drivers )
    return RTEMS_INVALID_NUMBER;

  /*
   * Test for initialise/open being present to indicate the driver slot is
   * in use.
   */

  if ( major == 0 ) {
    bool found = false;
    for ( major = _IO_Number_of_drivers - 1 ; major ; major-- ) {
      if ( !_IO_Driver_address_table[major].initialization_entry &&
           !_IO_Driver_address_table[major].open_entry ) {
        found = true;
        break;
      }
    }

    if ( !found )
      return RTEMS_TOO_MANY;
  }

  if ( _IO_Driver_address_table[major].initialization_entry ||
       _IO_Driver_address_table[major].open_entry )
    return RTEMS_RESOURCE_IN_USE;


  _IO_Driver_address_table[major] = *driver_table;
  *registered_major               = major;

  if ( _IO_Manager_drivers_inititalized ) {
    /* Other drivers have already been initialized, we initialize
     * the driver directly.
     */
    return rtems_io_initialize( major, 0, NULL );
  } else {
    /* The driver will be initialized together with all other drivers
     * in a later stage by _IO_Initialize_all_drivers().
     */
    return RTEMS_SUCCESSFUL;
  }
}
