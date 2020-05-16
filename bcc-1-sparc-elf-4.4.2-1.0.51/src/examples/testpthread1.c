/* 
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  init.c,v 1.26.8.1 2003/09/04 18:46:26 joel Exp
 */

/* c/src/tests/psxtests/psx01/ */

#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>
#include <time.h>
#include <assert.h>
#include <asm-leon/clock.h>
#include "testpthread.h"

#define TOD_NANOSECONDS_PER_SECOND      (unsigned int)1000000000
pthread_t        Init_id;
pthread_t        Task_id;
void *Task_1_through_3(void *argument);

main() 
{
  pthread_init();
  POSIX_Init(0);
}

void *POSIX_Init(
  void *argument
)
{
  struct timespec tv;
  struct timespec tr;
  int             status;
  int             priority;
  pthread_t       thread_id;
  time_t          seconds;
  time_t          seconds1;
  time_t          remaining;
  struct tm       tm;
  
  puts( "\n\n*** POSIX TEST 1 ***" );

  /* error cases in clock_gettime and clock_settime */

  puts( "Init: clock_gettime - EINVAL (invalid clockid)" );
  status = clock_settime( -1, &tv );
  assert( status == -1 );
  assert( errno == EINVAL );

  puts( "Init: clock_settime - EINVAL (invalid clockid)" );
  status = clock_settime( -1, &tv );
  assert( status == -1 );
  assert( errno == EINVAL );

  /* exercise clock_getres */

  puts( "Init: clock_getres - EINVAL (invalid clockid)" );
  status = clock_getres( -1, &tv );
  assert( status == -1 );
  assert( errno == EINVAL );

  puts( "Init: clock_getres - EINVAL (NULL resolution)" );
  status = clock_getres( CLOCK_REALTIME, NULL );
  assert( status == -1 );
  assert( errno == EINVAL );

  puts( "Init: clock_getres - SUCCESSFUL" );
  status = clock_getres( CLOCK_REALTIME, &tv );
  printf( "Init: resolution = sec (%ld), nsec (%ld)\n", tv.tv_sec, tv.tv_nsec );
  assert( !status );

  /* set the time of day, and print our buffer in multiple ways */
  tv.tv_sec = mktime( &tm );
  //assert( tv.tv_sec != -1 );
  tv.tv_nsec = 0;

  /* now set the time of day */

  printf("\n");
  
  printf( asctime( &tm ) );
  printf( "Init: clock_settime - SUCCESSFUL" );
  status = clock_settime( CLOCK_REALTIME, &tv );
  assert( !status );

  printf( asctime( &tm ) );
  printf( ctime( &tv.tv_sec ) );

  /* use sleep to delay */

  remaining = sleep( 1 );
  assert( !remaining );

  /* print new times to make sure it has changed and we can get the realtime */

  status = clock_gettime( CLOCK_REALTIME, &tv );
  assert( !status );

  printf( ctime( &tv.tv_sec ) );

  seconds = time( NULL );
  printf( ctime( &seconds ) );

  /*  just to have the value copied out through the parameter */
  
  seconds = time( &seconds1 );
  assert( seconds == seconds1 );

  /* check the time remaining */

  printf( "Init: seconds remaining (%d)\n", (int)remaining );
  
  /* error cases in nanosleep */

  printf("\n");
  printf( "Init: nanosleep - EINVAL (NULL time)" );
  status = nanosleep ( NULL, &tr );
  assert( status == -1 );
  assert( errno == EINVAL );

  tv.tv_sec = 0;
  tv.tv_nsec = NSEC_PER_SEC * 2;
  printf( "Init: nanosleep - EINVAL (too many nanoseconds)" );
  status = nanosleep ( &tv, &tr );
  assert( status == -1 );
  assert( errno == EINVAL );

  /* this is actually a small delay or yield */
  tv.tv_sec = -1;
  tv.tv_nsec = 0;
  printf( "Init: nanosleep - negative seconds small delay " );
  status = nanosleep ( &tv, &tr );
  assert( !status );

  /* use nanosleep to yield */

  tv.tv_sec = 0; 
  tv.tv_nsec = 0; 

  printf( "Init: nanosleep - yield" ); 
  status = nanosleep ( &tv, &tr );
  assert( !status );
  assert( !tr.tv_sec );
  assert( !tr.tv_nsec );

  /* use nanosleep to delay */

  tv.tv_sec = 0; 
  tv.tv_nsec = 5000000; 

  printf( "Init: nanosleep - 0.5 seconds" ); 
  status = nanosleep ( &tv, &tr );
  
  /* print the current real time again */

  status = clock_gettime( CLOCK_REALTIME, &tv );
  assert( !status );
 
  printf( ctime( &tv.tv_sec ) );

  /* check the time remaining */

  printf( "Init: sec (%ld), nsec (%ld) remaining\n", tr.tv_sec, tr.tv_nsec );
  assert( !tr.tv_sec && !tr.tv_nsec );

  /* get id of this thread */

  Init_id = pthread_self();
  printf( "Init: ID is 0x%08x\n", Init_id );

  /* exercise get minimum priority */

  priority = sched_get_priority_min( SCHED_FIFO );
  printf( "Init: sched_get_priority_min (SCHED_FIFO) -- %d\n", priority );
  assert( priority != -1 );

  puts( "Init: sched_get_priority_min -- EINVAL (invalid policy)" );
  priority = sched_get_priority_min( -1 );
  assert( priority == -1 );
  assert( errno == EINVAL );

  /* exercise get maximum priority */
 
  priority = sched_get_priority_max( SCHED_FIFO );
  printf( "Init: sched_get_priority_max (SCHED_FIFO) -- %d\n", priority );
  assert( priority != -1 );

  puts( "Init: sched_get_priority_min -- EINVAL (invalid policy)" );
  priority = sched_get_priority_min( -1 );
  assert( priority == -1 );
  assert( errno == EINVAL );

  /* print the round robin time quantum */
 
  status = sched_rr_get_interval( getpid(), &tr );
  printf( 
    "Init: Round Robin quantum is %ld seconds, %ld nanoseconds\n",
    tr.tv_sec,
    tr.tv_nsec
  );
  assert( !status );
  
  /* create a thread */

  puts( "Init: pthread_create - SUCCESSFUL" );
  status = pthread_create( &thread_id, NULL, Task_1_through_3, NULL );
  assert( !status );

  /* too may threads error */

  /*puts( "Init: pthread_create - EAGAIN (too many threads)" );
  status = pthread_create( &thread_id, NULL, Task_1_through_3, NULL );
  assert( status == EAGAIN );*/

  puts( "Init: sched_yield to Task_1" );
  status = sched_yield();
  assert( !status );

    /* switch to Task_1 */

  /* exit this thread */

  puts( "Init: pthread_exit" );
  while(1);
  
  pthread_exit( NULL );

    /* switch to Task_1 */

  return NULL; /* just so the compiler thinks we returned something */
}


/*  Task_1_through_3
 *
 *  This routine serves as a test task.  It verifies the basic task
 *  switching capabilities of the executive.
 *
 *  Input parameters:
 *    argument - task argument
 *
 *  Output parameters:  NONE
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  task.c,v 1.13.2.1 2003/09/04 18:46:26 joel Exp
 */


void Test_init_routine( void )
{
  puts( "Test_init_routine: invoked" );
}


void *Task_1_through_3(
  void *argument
)
{
  int            status;
  pthread_once_t once = PTHREAD_ONCE_INIT(once);

  puts( "Task_1: sched_yield to Init" );
  status = sched_yield();
  assert( !status );
 
    /* switch to Task_1 */

  /* now do some real testing */

  printf("\n");

  /* get id of this thread */

  Task_id = pthread_self();
  printf( "Task_1: ID is 0x%08x\n", Task_id );

  /* exercise pthread_equal */

  status = pthread_equal( Task_id, Task_id );
  if ( status )
    puts( "Task_1: pthread_equal - match case passed" );
  assert( status );

  status = pthread_equal( Init_id, Task_id );
  if ( !status )
    puts( "Task_1: pthread_equal - different case passed" );
  assert( !status );

  puts( "Task_1: pthread_equal - first id bad" );
  status = pthread_equal( -1, Task_id );
  assert( !status );

  puts( "Task_1: pthread_equal - second id bad" );
  status = pthread_equal( Init_id, -1 );
  assert( !status );

  /* exercise pthread_once */

  puts( "Task_1: pthread_once - EINVAL (NULL once_control)" );
  status = pthread_once( NULL, Test_init_routine );
  assert( status ==  EINVAL );

  puts( "Task_1: pthread_once - EINVAL (NULL init_routine)" );
  status = pthread_once( &once, NULL );
  assert( status == EINVAL );

  puts( "Task_1: pthread_once - SUCCESSFUL (init_routine executes)" );
  status = pthread_once( &once, Test_init_routine );
  assert( !status );

  puts( "Task_1: pthread_once - SUCCESSFUL (init_routine does not execute)" );
  status = pthread_once( &once, Test_init_routine );
  assert( !status );

  puts( "*** END OF POSIX TEST 1 ***" );
  exit( 0 );
  return NULL; /* just so the compiler thinks we returned something */
}
