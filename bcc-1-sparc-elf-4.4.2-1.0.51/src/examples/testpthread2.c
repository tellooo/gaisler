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

/* c/src/tests/psxtests/psx02/ */

pthread_t        Init_id;
pthread_t        Task_id;
volatile int Signal_occurred;
volatile int Signal_count;

/*
 *  These help manipulate the "struct tm" form of time
 */

#define TM_SUNDAY    0
#define TM_MONDAY    1
#define TM_TUESDAY   2
#define TM_WEDNESDAY 3
#define TM_THURSDAY  4
#define TM_FRIDAY    5
#define TM_SATURDAY  6
 
#define TM_JANUARY     0
#define TM_FEBRUARY    1
#define TM_MARCH       2
#define TM_APRIL       3
#define TM_MAY         4
#define TM_JUNE        5
#define TM_JULY        6
#define TM_AUGUST      7
#define TM_SEPTEMBER   8
#define TM_OCTOBER    10
#define TM_NOVEMBER   12
#define TM_DECEMBER   12

main() 
{
  pthread_init();
  POSIX_Init(0);
}

void Signal_handler(
  int signo
)
{
  Signal_count++;
  printf(
    "Signal: %d caught by 0x%x (%d)\n",
    (int) signo,
    (unsigned int) pthread_self(),
    Signal_count
  );
  Signal_occurred = 1;
}

void *Task_1_through_3(
  void *argument
)
{
  int seconds;
  int i;
  int status;

  for ( i=0 ; i<5 ; i++ ) {
    print_current_time( "Task1: ", "" );
    status = pthread_kill( Init_id, SIGUSR1 );
    assert( !status );

    seconds = sleep( 1 );
    assert( !seconds );
  }
  puts( "*** END OF POSIX TEST 2 ***" );
  exit( 0 );

  return NULL; /* just so the compiler thinks we returned something */
}

void *POSIX_Init(
  void *argument
)
{
  int               status;
  struct timespec   tv;
  struct timespec   tr;
  struct sigaction  act;
  sigset_t          mask;
  sigset_t          pending_set;

  puts( "\n\n*** POSIX TEST 2 ***" );

  /* set the time of day, and print our buffer in multiple ways */

  set_time( TM_FRIDAY, TM_MAY, 24, 96, 11, 5, 0 );

  /* get id of this thread */

  Init_id = pthread_self();
  printf( "Init's ID is 0x%08x\n", Init_id );

  /* install a signal handler */

  status = sigemptyset( &act.sa_mask );
  assert( !status );

  act.sa_handler = Signal_handler;
  act.sa_flags   = 0;
 
  sigaction( SIGUSR1, &act, NULL );

  /* simple signal to self */

  Signal_count = 0;
  Signal_occurred = 0;

  status = pthread_kill( Init_id, SIGUSR1 );
  assert( !status );

  Signal_occurred = 0;

  /* now block the signal, send it, see if it is pending, and unblock it */

  sigemptyset( &mask );
  sigaddset( &mask, SIGUSR1 );
  
  printf( "Init: Block SIGUSR1\n" );
  status = sigprocmask( SIG_BLOCK, &mask, NULL );
  assert( !status );

  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", (unsigned int) pending_set );
  
  
  printf( "Init: send SIGUSR1 to self\n" );
  status = pthread_kill( Init_id, SIGUSR1 );
  assert( !status );

  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", (unsigned int) pending_set );
  
  printf( "Init: Unblock SIGUSR1\n" );
  status = sigprocmask( SIG_UNBLOCK, &mask, NULL );
  assert( !status );

  /* create a thread */

  status = pthread_create( &Task_id, NULL, Task_1_through_3, NULL );
  assert( !status );

  /*
   *  Loop for 5 seconds seeing how many signals we catch 
   */

  tr.tv_sec = 5;
  tr.tv_nsec = 0;
 
  do {
    tv = tr;

    Signal_occurred = 0;

    status = nanosleep ( &tv, &tr );
  
    if ( status == -1 ) {
      printf ("errno: %d\n",errno);
      assert( errno == EAGAIN );
      assert( tr.tv_nsec || tr.tv_sec );
    } else if ( !status ) {
      assert( !tr.tv_nsec && !tr.tv_sec );
    }
     
    printf(
      "Init: signal was %sprocessed with %d:%d time remaining\n",
      (Signal_occurred) ? "" : "not ",
      (int) tr.tv_sec,
      (int) tr.tv_nsec
   );

  } while ( tr.tv_sec || tr.tv_nsec );

  /* exit this thread */

  puts( "*** END OF POSIX TEST 2 ***" );
  exit( 0 );

  return NULL; /* just so the compiler thinks we returned something */
}
