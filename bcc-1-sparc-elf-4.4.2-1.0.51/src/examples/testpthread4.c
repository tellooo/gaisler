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

/* c/src/tests/psxtests/psx04/ */

#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>
#include <time.h>
#include <assert.h>
#include <asm-leon/clock.h>
#include <signal.h>
#include "testpthread.h"

pthread_t        Init_id;
pthread_t        Task1_id;
pthread_t        Task2_id;
pthread_t        Task3_id;

volatile int Signal_occurred;
volatile int Signal_count;

void *Task_1(
  void *argument
)
{
  int seconds;

  printf( "Task_1: sleeping for 5 seconds\n" );

  seconds = sleep( 5 );
  printf( "Task_1: %d seconds left\n", seconds );
  assert( seconds );

     /* switch to Init */

  printf( "Task_1: exit\n" );
  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}

void *Task_2(
  void *argument
)
{
  int status;

  printf( "Task_2: sending SIGUSR1\n" );
  status = pthread_kill( Init_id, SIGUSR1 );
  assert( !status );

     /* switch to Init */

  printf( "Task_2: exit\n" );
  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}

void *Task_3(
  void *argument
)
{
  int           status;
  int           sig;
  union p_sigval  value;
  sigset_t      mask;
  siginfo_t     info;

  value.sival_int = SIGUSR1;

  printf( "Task_3: sigqueue SIGUSR1 with value= %d\n", value.sival_int );
  status = sigqueue( getpid(), SIGUSR1, value );
  assert( !status );

     /* catch signal with sigwaitinfo */

  empty_line();

  status = sigemptyset( &mask );
  assert( !status );

  status = sigaddset( &mask, SIGUSR1 );
  assert( !status );

  printf( "Task_3: sigwaitinfo SIGUSR1 with value= %d\n", value.sival_int );
  status = sigwaitinfo( &mask, &info );

     /* switch to Init */

  assert( !(status==-1) );
  printf(
    "Task_3: si_signo= %d si_code= %d value= %d\n",
    info.si_signo,
    info.si_code,
    info.si_value.sival_int
  );

     /* catch signal with sigwait */

  empty_line();

  status = sigemptyset( &mask );
  assert( !status );
 
  status = sigaddset( &mask, SIGUSR1 );
  assert( !status );
 
  printf( "Task_3: sigwait SIGUSR1\n" );
  status = sigwait( &mask, &sig );
 
     /* switch to Init */
 
  assert( !status );
  printf( "Task_3: signo= %d\n", sig );

     /* catch signal with pause */
 
  empty_line();

  status = sigemptyset( &mask );
  assert( !status );
 
  status = sigaddset( &mask, SIGUSR1 );
  assert( !status );
 
  printf( "Task_3: pause\n" );
  status = pause( );
 
     /* switch to Init */
 
  assert( !(status==-1) );
  printf( "Task_3: pause= %d\n", status );
 

     /* send signal to Init task before it has pended for a signal */

  empty_line();

  printf( "Task_3: sending SIGUSR2\n" );
  status = pthread_kill( Init_id, SIGUSR2 );
  assert( !status );

  printf( "Task_3: sleep so the Init task can reguest a signal\n" ); 
  status = sleep( 1 );
  assert( !status );

     /* end of task 3 */
  printf( "Task_3: exit\n" );
  pthread_exit( NULL );

     /* switch to Init */

  return NULL; /* just so the compiler thinks we returned something */
}

void Signal_handler(
  int signo
)
{
  Signal_count++;
  printf(
    "############################## Signal: %d caught by 0x%x (%d)\n",
    signo,
    pthread_self(),
    Signal_count
  );
  Signal_occurred = 1;
}

void Signal_info_handler(
  int        signo,
  siginfo_t *info,
  void      *context
)
{
  Signal_count++;
  printf(
    "############################## Signal_info: %d caught by 0x%x (%d) si_signo= %d si_code= %d value= %d\n",
    signo,
    pthread_self(),
    Signal_count,
    info->si_signo,
    info->si_code,
    info->si_value.sival_int
  );
  Signal_occurred = 1;
}

void *POSIX_Init(
  void *argument
)
{
  int               status;
  struct sigaction  act;
  sigset_t          mask;
  sigset_t          pending_set;
  sigset_t          oset;
  struct timespec   timeout;
  siginfo_t         info;

  puts( "\n\n*** POSIX TEST 4 ***" );

  /* set the time of day, and print our buffer in multiple ways */

  set_time( TM_FRIDAY, TM_MAY, 24, 96, 11, 5, 0 );

  /* get id of this thread */

  Init_id = pthread_self();
  printf( "Init's ID is 0x%08x\n", Init_id );

/* install a signal handler for SIGUSR1 */

  status = sigemptyset( &act.sa_mask );
  printf( "Init: sigemptyset -  set= 0x%08x\n", (unsigned int) act.sa_mask );

  /* test sigfillset following the above sigemptyset */

  status = sigfillset( &act.sa_mask );
  printf( "Init: sigfillset -  set= 0x%08x\n", (unsigned int) act.sa_mask );

  /* test sigdelset */

  status = sigdelset( &act.sa_mask, SIGUSR1 );
  printf( "Init: sigdelset - delete SIGUSR1 set= 0x%08x\n",
      (unsigned int) act.sa_mask );

  /* test sigismember - FALSE */

  status = sigismember( &act.sa_mask, SIGUSR1 );
  assert( !status );
  puts( "Init: sigismember - FALSE since SIGUSR1 is not a member" );

  /* test sigismember - TRUE */

  status = sigismember( &act.sa_mask, SIGUSR2 );
  assert( status );
  puts( "Init: sigismember - TRUE since SIGUSR2 is a member" );

  /* return the set to empty */

  act.sa_handler = Signal_info_handler;
  act.sa_flags   = 0;
  act.sa_mask = 0;
   
  sigaction( SIGUSR1, &act, NULL );

  /* simple signal to process */

  Signal_count = 0;
  Signal_occurred = 0;

  puts( "Init: send SIGUSR1 to process" ); 
  status = pthread_kill( Init_id, SIGUSR1 );
  assert( !status );

/* end of install a signal handler for SIGUSR1 */

  Signal_occurred = 0;

  /* now block the signal, send it, see if it is pending, and unblock it */

  empty_line();

  status = sigemptyset( &mask );
  
  status = sigaddset( &mask, SIGUSR1 );
  
  puts( "Init: Block SIGUSR1" );
  act.sa_handler = Signal_info_handler;
  act.sa_flags   = 0;
  
  sigaction( SIGUSR1, &act, NULL );

  /* simple signal to process */

  Signal_count = 0;
  Signal_occurred = 0;

  puts( "Init: send SIGUSR1 to process" );
  status = kill( getpid(), SIGUSR1 );
  assert( !status );

  Signal_occurred = 0;

  /* now block the signal, send it, see if it is pending, and unblock it */

  empty_line();

  status = sigemptyset( &mask );
  
  status = sigaddset( &mask, SIGUSR1 );
  
  puts( "Init: Block SIGUSR1" );
  status = sigprocmask( SIG_BLOCK, &mask, NULL );
  assert( !status );

  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", (unsigned int) pending_set );
  
  puts( "Init: send SIGUSR1 to process" );
  status = kill( getpid(), SIGUSR1 );
  assert( !status );

  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", (unsigned int) pending_set );
  
  puts( "Init: Unblock SIGUSR1" );
  status = sigprocmask( SIG_UNBLOCK, &mask, NULL );
  assert( !status );

  /* now let another task get interrupted by a signal */

  empty_line();

  puts( "Init: create a thread interested in SIGUSR1" );
  status = pthread_create( &Task1_id, NULL, Task_1, NULL );
  assert( !status );

  puts( "Init: Block SIGUSR1" );
  status = sigprocmask( SIG_BLOCK, &mask, NULL );
  assert( !status );
 
  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", (unsigned int) pending_set );

  puts( "Init: sleep so the other task can block" ); 
  status = sleep( 1 );
  assert( !status );

     /* switch to task 1 */

  puts( "Init: send SIGUSR1 to process" );
  status = pthread_kill( Task1_id, SIGUSR1 );
  assert( !status ); 
 
  status = sigpending( &pending_set );
  assert( !status );
  printf( "Init: Signals pending 0x%08x\n", (unsigned int) pending_set );

  puts( "Init: sleep so the other task can catch signal" ); 
  status = sleep( 1 );
  assert( !status );

     /* switch to task 1 */

  /* test alarm */

  empty_line();



    /* exit this thread */

  puts( "*** END OF POSIX TEST 4 ***" );
  exit( 0 );

  return NULL; /* just so the compiler thinks we returned something */
}


main() 
{
  pthread_init();
  POSIX_Init(0);
}

