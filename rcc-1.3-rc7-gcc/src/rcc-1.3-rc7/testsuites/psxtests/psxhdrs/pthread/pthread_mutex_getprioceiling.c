/*
* @file
* @brief pthread_mutex_getprioceiling() API Conformance Test
*/

/*
* COPYRIGHT (c) 1989-2009.
* On-Line Applications Research Corporation (OAR).
*
* The license and distribution terms for this file may be
* found in the file LICENSE in this distribution or at
* http://www.rtems.org/license/LICENSE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>

#ifndef _POSIX_THREAD_PRIO_INHERIT
#error "rtems is supposed to have pthread_mutex_getprioceiling"
#endif
#ifndef _POSIX_THREAD_PRIO_PROTECT
#error "rtems is supposed to have pthread_mutex_getprioceiling"
#endif

int test( void );

int test( void )
{
  pthread_mutex_t mutex;
  int             prioceiling;
  int             result;

  result = pthread_mutex_getprioceiling( &mutex, &prioceiling );

  return result;
}
