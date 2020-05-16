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

/* c/src/tests/psxtests/psx03/ */

#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>
#include <time.h>
#include <assert.h>
#include <asm-leon/clock.h>
#include <signal.h>
#include "testpthread.h"

pthread_t        Init_id;
pthread_t        Task_id;
volatile int Signal_occurred;
volatile int Signal_count;


void *Task_1(
  void *argument
)
{
  int status;

  /* send SIGUSR2 to Init which is waiting on SIGUSR1 */

  print_current_time( "Task_1: ", "" );

  printf( "Task_1: pthread_kill - SIGUSR2(%d) to Init\n",SIGUSR2 );
  status = pthread_kill( Init_id, SIGUSR2 );
  assert( !status );

  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}

void *Task_2(
  void *argument
)
{
  int status;

  /* send SIGUSR1 to Init which is waiting on SIGUSR1 */
 
  print_current_time( "Task_2: ", "" );

  printf( "Task_2: pthread_kill - SIGUSR1(%d) to Init\n",SIGUSR1 );
  status = pthread_kill( Init_id, SIGUSR1 );
  assert( !status );
 
  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}


volatile int Signal_occurred;
volatile int Signal_count;

void Signal_handler(
  int signo
)
{
  Signal_count++;
  printf(
    "##################### Signal: %d caught by 0x%x (%d) ################ \n",
    signo,
    pthread_self(),
    Signal_count
  );
  Signal_occurred = 1;
}

void *POSIX_Init(
  void *argument
)
{
  int               status;
  struct timespec   timeout;
  struct sigaction  act;
  sigset_t          mask;
  sigset_t          waitset;
  int               signo;
  siginfo_t         siginfo;

  puts( "\n\n*** POSIX TEST 3 ***" );

  /* set the time of day, and print our buffer in multiple ways */

  set_time( TM_FRIDAY, TM_MAY, 24, 96, 11, 5, 0 );

  /* get id of this thread */

  Init_id = pthread_self();
  printf( "Init's ID is 0x%08x\n", Init_id );

  /* install a signal handler */

  sigemptyset( &act.sa_mask );
  
  act.sa_handler = Signal_handler;
  act.sa_flags   = 0;
 
  sigaction( SIGUSR1, &act, NULL );

  /* initialize signal handler variables */

  Signal_count = 0;
  Signal_occurred = 0;

  //--------------------------------------------------------
  /*
   *  wait on SIGUSR1 for 0.5 seconds, will timeout 
   */

  /* initialize the signal set we will wait for to SIGUSR1 */

  sigemptyset( &waitset );

  sigaddset( &waitset, SIGUSR1 );
  
  timeout.tv_sec = 0;
  timeout.tv_nsec = 5000000;

  puts( "Init: waiting on any signal for 0.5 seconds: (should timeout)" );
  signo = sigtimedwait( &waitset, &siginfo, &timeout );
  printf( "signo: %d\n",signo );
  assert( signo == -1 );

  if ( errno == EAGAIN ) 
    puts( "Init: correctly timed out waiting for SIGUSR1." );
  else
    printf( "sigtimedwait returned wrong errno - %d\n", errno );

  Signal_occurred = 0;

  //--------------------------------------------------------
  /*
   *  wait on SIGUSR1 for 0.9 seconds, will timeout because Task_1 sends SIGUSR2
   */

  empty_line();

  /* initialize a mask to block SIGUSR2 */

  sigemptyset( &mask );
  
  sigaddset( &mask, SIGUSR2 );

  printf( "Init: Block SIGUSR2\n" );
  status = sigprocmask( SIG_BLOCK, &mask, NULL );
  assert( !status );

  /* create a thread */

  status = pthread_create( &Task_id, NULL, Task_1, NULL );
  assert( !status );

  /* signal handler is still installed, waitset is still set for SIGUSR1 */
   
  timeout.tv_sec = 0;
  timeout.tv_nsec = 9000000;
 
  puts( "Init: waiting on any signal for 0.9 seconds. (should timeout)" );
  signo = sigtimedwait( &waitset, &siginfo, &timeout );

     /* switch to Task 1 */

  if ( errno == EAGAIN )
    puts( "Init: correctly timed out waiting for SIGUSR1." );
  else
    printf( "sigtimedwait returned wrong errno - %d\n", errno );
  assert( signo == -1 );

  //--------------------------------------------------------
  /*
   *  wait on SIGUSR1 for 3 seconds, Task_2 will send it to us
   */

  empty_line();

  /* create a thread */

  status = pthread_create( &Task_id, NULL, Task_2, NULL );
  assert( !status );

  /* signal handler is still installed, waitset is still set for SIGUSR1 */
 
  /* wait on SIGUSR1 for 3 seconds, will receive SIGUSR1 from Task_2 */
 
  timeout.tv_sec = 3;
  timeout.tv_nsec = 0;
 
  /* just so we can check that these were altered */

  siginfo.si_code = -1;
  siginfo.si_signo = -1;
  siginfo.si_value.sival_int = -1;

  puts( "Init: waiting on any signal for 3 seconds. (should receive)" );
  signo = sigtimedwait( &waitset, &siginfo, &timeout ); 
 
  /* exit this thread */

  puts( "*** END OF POSIX TEST 3 ***" );
  exit( 0 );

  return NULL; /* just so the compiler thinks we returned something */
}

main() 
{
  pthread_init();
  POSIX_Init(0);
}

