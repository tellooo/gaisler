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

#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>
#include <time.h>
#include <assert.h>
#include <asm-leon/clock.h>
#include <signal.h>
#include "testpthread.h"

/* c/src/tests/psxtests/psx05/ */

pthread_t        Init_id;
pthread_t        Task_id;
pthread_t        Task2_id;
pthread_t        Task3_id;
pthread_mutex_t  Mutex_id;
pthread_mutex_t  Mutex2_id;
pthread_mutex_t  Mutex3_id;

volatile int Signal_occurred;
volatile int Signal_count;


void *Task_1(
  void *argument
)
{
  int status;

  printf( "Task: pthread_mutex_trylock already locked\n" );
  status = pthread_mutex_trylock( &Mutex_id );
  if ( status != EBUSY )
    printf( "status = %d\n", status );
  assert( status == EBUSY );

  printf( "Task: pthread_mutex_lock unavailable\n" );
  status = pthread_mutex_lock( &Mutex_id );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

     /* switch to init */

  printf( "Task: mutex acquired\n" );

  printf( "Task: sleep for 2 seconds\n" );
  sleep( 2 );

     /* switch to init */

  printf( "Task: exit\n" );
  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}

void *Task_2(
  void *argument
)
{
  int status;

  printf( "Task 2: pthread_mutex_lock unavailable (inherit case)\n" );
  status = pthread_mutex_lock( &Mutex2_id );
  if ( status )
    printf( "status =%d\n", status );
  assert( !status );
  printf( "Task 2: mutex acquired\n" );

     /* switch to init */

  printf( "Task 2: unlock Mutex 2\n" );
  status = pthread_mutex_unlock( &Mutex2_id );
  printf("Status: %d\n",status);
  assert( !status );

  printf( "Task 2: exit\n" );
  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}

void *Task_3(
  void *argument
)
{
  int                 status;
  struct sched_param  param;
  int                 policy;

  printf( "Task 3: pthread_mutex_lock unavailable (inherit case)\n" );
  status = pthread_mutex_lock( &Mutex2_id );
  printf( "Task 3: mutex acquired\n" );
  assert( !status );

  printf( "Task 3: unlock Mutex 2\n" );
  status = pthread_mutex_unlock( &Mutex2_id );
  assert( !status );

  status = pthread_getschedparam( pthread_self(), &policy, &param );
  assert( !status );
  printf( "Task 3: pthread_getschedparam priority = %d\n", param.sched_priority );

  printf( "Task 3: exit\n" );
  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}


void Print_mutexattr(
  char                *msg,
  pthread_mutexattr_t *attr
)
{
  int status;
  int protocol;
  int prioceiling;
  int pshared;

  /* protocol */

  status = pthread_mutexattr_getprotocol( attr, &protocol );
  assert( !status );

  printf( "%smutex protocol is (%d) -- ", msg, protocol );
  switch ( protocol ) {
    case PTHREAD_PRIO_NONE:
      puts( "PTHREAD_PRIO_NONE" );
      break;
    case PTHREAD_PRIO_INHERIT:
      puts( "PTHREAD_PRIO_INHERIT" );
      break;
    case PTHREAD_PRIO_PROTECT:
      puts( "PTHREAD_PRIO_PROTECT" );
      break;
    default:
      puts( "UNKNOWN" );
      assert( 0 );
      break;
  }

  /* priority ceiling */

  status = pthread_mutexattr_getprioceiling( attr, &prioceiling );
  assert( !status );
  printf( "%smutex priority ceiling is %d\n", msg, prioceiling );


}

void *POSIX_Init(
  void *argument
)
{
  int                  status;
  pthread_mutexattr_t  attr;
  pthread_mutexattr_t  destroyed_attr;
  struct timespec      times;
  struct sched_param   param;
  int                  pshared;
  int                  policy;
  int                  protocol;
  int                  ceiling;
  int                  old_ceiling;

  puts( "\n\n*** POSIX TEST 5 ***" );

  /* set the time of day, and print our buffer in multiple ways */

  set_time( TM_FRIDAY, TM_MAY, 24, 96, 11, 5, 0 );

  /* get id of this thread */

  Init_id = pthread_self();
  printf( "Init's ID is 0x%08x\n", Init_id );
  
  /* tes pthread_mutex_attr_init */

  puts( "Init: pthread_mutexattr_init - EINVAL (NULL attr)" );
  status = pthread_mutexattr_init( NULL );
  assert( status == EINVAL );

  puts( "Init: pthread_mutexattr_init - SUCCESSFUL" );
  status = pthread_mutexattr_init( &attr );
  assert( !status );

  Print_mutexattr( "Init: ", &attr );

  /* create an "uninitialized" attribute structure */

  status = pthread_mutexattr_init( &destroyed_attr );
  assert( !status );

  puts( "Init: pthread_mutexattr_destroy - SUCCESSFUL" );
  status = pthread_mutexattr_destroy( &destroyed_attr );
  assert( !status );

  puts( "Init: pthread_mutexattr_destroy - EINVAL (NULL attr)" );
  status = pthread_mutexattr_destroy( NULL );
  assert( status == EINVAL );

  puts( "Init: pthread_mutexattr_destroy - EINVAL (not initialized)" );
  status = pthread_mutexattr_destroy( &destroyed_attr );
  assert( status == EINVAL );


  /* error cases for set and get protocol attribute */

  empty_line();

  puts( "Init: pthread_mutexattr_getprotocol - EINVAL (NULL attr)" );
  status = pthread_mutexattr_getprotocol( NULL, &protocol );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_getprotocol - EINVAL (NULL protocol)" );
  status = pthread_mutexattr_getprotocol( &attr, NULL );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_getprotocol - EINVAL (not initialized)" );
  status = pthread_mutexattr_getprotocol( &destroyed_attr, &protocol );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_setprotocol - EINVAL (NULL attr)" );
  status = pthread_mutexattr_setprotocol( NULL, PTHREAD_PRIO_NONE );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_setprotocol - EINVAL (invalid protocol)" );
  status = pthread_mutexattr_setprotocol( &attr, -1 );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_setprotocol - EINVAL (not initialized)" );
  status = pthread_mutexattr_setprotocol( &destroyed_attr, -1 );
  assert( status == EINVAL );

  /* error cases for set and get prioceiling attribute */

  empty_line();
 
  puts( "Init: pthread_mutexattr_getprioceiling - EINVAL (NULL attr)" );
  status = pthread_mutexattr_getprioceiling( NULL, &ceiling );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_getprioceiling - EINVAL (NULL prioceiling)" );
  status = pthread_mutexattr_getprioceiling( &attr, NULL );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_getprioceiling - EINVAL (not initialized)" );
  status = pthread_mutexattr_getprioceiling( &destroyed_attr, &ceiling );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_setprioceiling - EINVAL (NULL attr)" );
  status = pthread_mutexattr_setprioceiling( NULL, 128 );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_setprioceiling - EINVAL (invalid priority)" );
  status = pthread_mutexattr_setprioceiling( &attr, 512 );
  if ( status != EINVAL )
    printf( "status = %d\n", status );
  assert( status == EINVAL );
 
  puts( "Init: pthread_mutexattr_setprioceiling - EINVAL (not initialized)" );
  status = pthread_mutexattr_setprioceiling( &destroyed_attr, -1 );
  assert( status == EINVAL );

  /* now try some basic mutex operations */

  empty_line();

  puts( "Init: pthread_mutex_init - EINVAL (NULL mutex_id)" );
  status = pthread_mutex_init( NULL, &attr );
  assert( status == EINVAL );

  puts( "Init: pthread_mutex_init - EINVAL (not initialized attr)" );
  status = pthread_mutex_init( &Mutex_id, &destroyed_attr );
  assert( status == EINVAL );

  /* must get around error checks in attribute set routines */
  attr.protocol = -1;

  puts( "Init: pthread_mutex_init - EINVAL (bad protocol)" );
  status = pthread_mutex_init( &Mutex_id, &attr );
  assert( status == EINVAL );

  /* must get around error checks in attribute set routines */
  attr.protocol = PTHREAD_PRIO_INHERIT;
  attr.prioceiling = -1;

  puts( "Init: pthread_mutex_init - EINVAL (bad priority ceiling)" );
  status = pthread_mutex_init( &Mutex_id, &attr );
  assert( status == EINVAL );

  /* now set up for a success pthread_mutex_init */

  puts( "Init: Resetting mutex attributes" );
  status = pthread_mutexattr_init( &attr );
  assert( !status );

  puts( "Init: Changing mutex attributes" );
  status = pthread_mutexattr_setprotocol( &attr, PTHREAD_PRIO_PROTECT );
  assert( !status );
 
  status = pthread_mutexattr_setprioceiling( &attr, 101 );
  assert( !status );
 
 
  Print_mutexattr( "Init: ", &attr );
 
  puts( "Init: Resetting mutex attributes" );
  status = pthread_mutexattr_init( &attr );
  assert( !status );

  /*
   *  Set the protocol to priority ceiling so the owner check happens
   *  and the EPERM test (later) will work.
   */

/*   status = pthread_mutexattr_setprotocol( &attr, PTHREAD_PRIO_INHERIT ); */
/*   assert( !status ); */
 
  puts( "Init: pthread_mutex_init - SUCCESSFUL" );
  status = pthread_mutex_init( &Mutex_id, &attr );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

  /*
   *  This is not required to be an error and when it is, there are
   *  behavioral conflicts with other implementations.
   */
  puts( "Init: pthread_mutex_init - EBUSY (reinitialize an existing mutex) - skipped" );

#if 0
  status = pthread_mutex_init( &Mutex_id, &attr );
  if ( !status )
    printf( "status = %d\n", status );
  assert( status == EBUSY );
#endif

  puts( "Init: pthread_mutex_trylock - SUCCESSFUL" );
  status = pthread_mutex_trylock( &Mutex_id );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

  puts( "Init: pthread_mutex_trylock - EDEADLK (already locked)" );
  status = pthread_mutex_trylock( &Mutex_id );
  if ( status != EDEADLK )
    printf( "status = %d\n", status );
  assert( status == EDEADLK );

  puts( "Init: pthread_mutex_lock - EDEADLK (already locked)" );
  status = pthread_mutex_lock( &Mutex_id );
  if ( status != EDEADLK )
    printf( "status = %d\n", status );
  assert( status == EDEADLK );

  /* create a thread */

  status = pthread_create( &Task_id, NULL, Task_1, NULL );
  assert( !status );

  puts( "Init: Sleep 1 second" );

  sleep( 1 );
  
     /* switch to task 1 */

  puts( "Init: pthread_mutex_unlock - SUCCESSFUL" );
  status = pthread_mutex_unlock( &Mutex_id );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

/*   puts( "Init: pthread_mutex_unlock - EPERM (not owner)" ); */
/*   status = pthread_mutex_unlock( &Mutex_id ); */
/*   if ( status != EPERM ) */
/*     printf( "status = %d\n", status ); */
/*   assert( status == EPERM ); */

/*   times.tv_sec = 0; */
/*   times.tv_nsec = 500000000; */
/*   puts( "Init: pthread_mutex_timedlock - time out in 1/2 second" ); */
/*   status = pthread_mutex_timedlock( &Mutex_id, &times ); */
/*   if ( status != EAGAIN ) */
/*     printf( "status = %d\n", status ); */
/*   assert( status == EAGAIN ); */

     /* switch to idle */

  puts( "Init: pthread_mutex_timedlock - EAGAIN (timeout)" );

  /* destroy a mutex */

  empty_line();

  puts( "Init: pthread_mutex_init - SUCCESSFUL" );
  status = pthread_mutex_init( &Mutex2_id, &attr );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

  puts( "Init: pthread_mutexattr_destroy - SUCCESSFUL" );
  status = pthread_mutexattr_destroy( &attr );
  assert( !status );

  puts( "Init: pthread_mutex_destroy - SUCCESSFUL" );
  status = pthread_mutex_destroy( &Mutex2_id );
  assert( !status );
 
  /* destroy a busy mutex */
 
  empty_line();
 
  puts( "Init: pthread_mutexattr_init - SUCCESSFUL" );
  status = pthread_mutexattr_init( &attr );
  assert( !status );

  puts( "Init: pthread_mutex_init - SUCCESSFUL" );
  status = pthread_mutex_init( &Mutex2_id, &attr );
  assert( !status );
 
  puts( "Init: pthread_mutex_trylock - SUCCESSFUL" );
  status = pthread_mutex_trylock( &Mutex2_id );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

  puts( "Init: pthread_mutex_destroy - EBUSY (already locked)" );
  status = pthread_mutex_destroy( &Mutex2_id );
  if ( status != EBUSY )
    printf( "status = %d\n", status );
  assert( status == EBUSY );

  puts( "Init: pthread_mutex_unlock - SUCCESSFUL" );
  status = pthread_mutex_unlock( &Mutex2_id );
  assert( !status );

  puts( "Init: pthread_mutex_destroy - SUCCESSFUL" );
  status = pthread_mutex_destroy( &Mutex2_id );
  assert( !status );

  /* priority inherit mutex */

  empty_line();
 
  puts( "Init: pthread_mutexattr_init - SUCCESSFUL" );
  status = pthread_mutexattr_init( &attr );
  assert( !status );

  puts(
    "Init: pthread_mutexattr_setprotocol - SUCCESSFUL (PTHREAD_PRIO_INHERIT)"
  );
  status = pthread_mutexattr_setprotocol( &attr, PTHREAD_PRIO_INHERIT );

  puts( "Init: pthread_mutex_init - SUCCESSFUL" );
  status = pthread_mutex_init( &Mutex2_id, &attr );
  assert( !status );

  puts( "Init: pthread_mutex_trylock - SUCCESSFUL" );
  status = pthread_mutex_trylock( &Mutex2_id );
  assert( !status );

  /* create a thread at a lower priority */
 
  status = pthread_create( &Task2_id, NULL, Task_2, NULL );
  assert( !status );
 
  /* set priority of Task2 to highest priority */
 
  param.sched_priority = 254;
 
  puts( "Init: pthread_setschedparam - Setting Task2 priority to highest" );
  status = pthread_setschedparam( Task2_id, SCHED_FIFO, &param );
  assert( !status );


  status = pthread_getschedparam( pthread_self(), &policy, &param );
  assert( !status );
  printf( "Init: pthread_getschedparam - priority = %d\n", param.sched_priority);

  puts( "Init: pthread_mutex_unlock - SUCCESSFUL" );
  status = pthread_mutex_unlock( &Mutex2_id );
  assert( !status );
  
  /* switching to Task2 */
  status = pthread_mutex_lock( &Mutex2_id );
  status = pthread_mutex_unlock( &Mutex2_id );
  
  puts( "Init: pthread_mutexattr_destroy - SUCCESSFUL" );
  status = pthread_mutexattr_destroy( &attr );
  assert( !status );

  puts( "Init: pthread_mutex_destroy - SUCCESSFUL" );
  status = pthread_mutex_destroy( &Mutex2_id );
  
  /* priority ceiling mutex */
 
  empty_line();
 
  puts( "Init: pthread_mutexattr_init - SUCCESSFUL" );
  status = pthread_mutexattr_init( &attr );
  assert( !status );
 
  puts(
    "Init: pthread_mutexattr_setprotocol - SUCCESSFUL (PTHREAD_PRIO_PROTECT)"
  );
  status = pthread_mutexattr_setprotocol( &attr, PTHREAD_PRIO_PROTECT );
  assert( !status );
 
  puts( "Init: pthread_mutex_init - SUCCESSFUL" );
  status = pthread_mutex_init( &Mutex2_id, &attr );
  printf("status: %d\n",status);
  
  assert( !status );
 
  puts( "Init: pthread_mutex_getprioceiling - EINVAL (NULL ceiling)" );
  status = pthread_mutex_getprioceiling( &Mutex2_id, NULL );
  assert( status == EINVAL );

  status = pthread_mutex_getprioceiling( &Mutex2_id, &ceiling );
  assert( !status );
  printf( "Init: pthread_mutex_getprioceiling - %d\n", ceiling );
 
  puts( "Init: pthread_mutex_setprioceiling - EINVAL (illegal priority)" );
  status = pthread_mutex_setprioceiling( &Mutex2_id, 512, &old_ceiling );
  assert( status == EINVAL );

  puts( "Init: pthread_mutex_setprioceiling - EINVAL (NULL ceiling)" );
  status = pthread_mutex_setprioceiling( &Mutex2_id, 128, NULL );
  assert( status == EINVAL );

  /* normal cases of set priority ceiling */

  puts( "Init: pthread_mutex_setprioceiling - new ceiling = 200" );
  status = pthread_mutex_setprioceiling( &Mutex2_id, 200, &old_ceiling );
  printf("status: %d\n",status);
  printf(
    "Init: pthread_mutex_setprioceiling - old ceiling = %d\n",old_ceiling
  );
 
  status = pthread_getschedparam( pthread_self(), &policy, &param );
  assert( !status );
  printf(
    "Init: pthread_getschedparam - priority = %d\n", param.sched_priority
  );

  puts( "Init: pthread_mutex_trylock - SUCCESSFUL" );
  status = pthread_mutex_trylock( &Mutex2_id );
  assert( !status );
 
  status = pthread_getschedparam( pthread_self(), &policy, &param );
  assert( !status );
  printf(
    "Init: pthread_getschedparam - priority = %d\n", param.sched_priority
  );

  /* create a thread at a higher priority */
 
  status = pthread_create( &Task3_id, NULL, Task_3, NULL );
  assert( !status );
 
  /* set priority of Task3 to highest priority */
 
  param.sched_priority = 199;
 
  status = pthread_setschedparam( Task3_id, SCHED_FIFO, &param );
  assert( !status );
  puts( "Init: pthread_setschedparam - set Task3 priority to highest" );
 
  /* DOES NOT SWITCH to Task3 */

  puts( "Init: Sleep 1 second" );
  assert( !status );
  sleep( 1 );
  
  /* switch to task 3 */
 
  puts( "Init: pthread_mutex_unlock - SUCCESSFUL" );
  status = pthread_mutex_unlock( &Mutex2_id );
  assert( !status );
 
  status = pthread_mutex_getprioceiling( &Mutex2_id, &ceiling );
  assert( !status );
  printf( "Init: pthread_mutex_getprioceiling- ceiling = %d\n", ceiling );
 
  /* set priority of Init to highest priority */
 
  param.sched_priority = 254;
 
  status = pthread_setschedparam( Init_id, SCHED_FIFO, &param );
  assert( !status );
  puts( "Init: pthread_setschedparam - set Init priority to highest" );
 
  puts( "Init: pthread_mutex_lock - EINVAL (priority ceiling violation)" );
  status = pthread_mutex_lock( &Mutex2_id );
  if ( status != EINVAL )
    printf( "status = %d\n", status );
  assert( status == EINVAL );

  puts( "*** END OF POSIX TEST 5 ***" );
  exit( 0 );

  return NULL; /* just so the compiler thinks we returned something */
}


main() 
{
  pthread_init();
  POSIX_Init(0);
}

