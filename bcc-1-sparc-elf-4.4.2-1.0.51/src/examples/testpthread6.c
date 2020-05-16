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

/* c/src/tests/psxtests/psx06/ */

pthread_t        Init_id;
pthread_t        Task_id;
pthread_t        Task2_id;
pthread_key_t    Key_id;
unsigned int Data_array[ 256 ];
unsigned int Destructor_invoked;

void *Task_1(
  void *argument
)
{
  int               status;
  unsigned int  *key_data; 

  printf( "Task_1: Setting the key(0x%x) to %d\n", Key_id, 1 );
  status = pthread_setspecific( Key_id, &Data_array[ 1 ] );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );
 
  key_data = pthread_getspecific( Key_id );
  printf( "Task_1: Got the key(0x%x) value of %ld\n",Key_id,
          (unsigned long) ((unsigned int *)key_data - Data_array) );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

  puts( "Task_1: exitting" );
  pthread_exit( NULL );

     /* switch to task 2 */

  return NULL; /* just so the compiler thinks we returned something */
}

void *Task_2(
  void *argument
)
{
  int               status;
  unsigned int *key_data;
 
  printf( "Destructor invoked %d times\n", Destructor_invoked );

  printf( "Task_2: Setting the key to %d\n", 2 );
  status = pthread_setspecific( Key_id, &Data_array[ 2 ] );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );
 
  key_data = pthread_getspecific( Key_id );
  printf( "Task_2: Got the key value of %ld\n",
          (unsigned long) ((unsigned int *)key_data - Data_array) );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );
 
  puts( "Task2: exitting" );
  pthread_exit( NULL );
 
     /* switch to init task */
 
  return NULL; /* just so the compiler thinks we returned something */
}

void Key_destructor(
   void *key_data
)
{
  Destructor_invoked++;

  /*
   *  This checks out that we only run the destructor multiple times
   *  when the key data is non null.
   */

  if ( Destructor_invoked == 5 )
     (void) pthread_setspecific( Key_id, NULL );
}

void *POSIX_Init(
  void *argument
)
{
  int               status;
  unsigned int      remaining;
  unsigned int *key_data;

  puts( "\n\n*** POSIX TEST 6 ***" );

  /* set the time of day, and print our buffer in multiple ways */

  set_time( TM_FRIDAY, TM_MAY, 24, 96, 11, 5, 0 );

  /* get id of this thread */

  Init_id = pthread_self();
  printf( "Init's ID is 0x%08x\n", Init_id );
  
  /* create a key */

  empty_line();

  Destructor_invoked = 0;
  puts( "Init: pthread_key_create - SUCCESSFUL" );
  status = pthread_key_create( &Key_id, Key_destructor );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

  printf( "Destructor invoked %d times\n", Destructor_invoked );

  printf( "Init: Setting the key(0x%x) to %d\n", Key_id, 0 );
  status = pthread_setspecific( Key_id, &Data_array[ 0 ] );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );


  /* create a couple of threads */

  status = pthread_create( &Task_id, NULL, Task_1, NULL );
  assert( !status );

  status = pthread_create( &Task2_id, NULL, Task_2, NULL );
  assert( !status );


  /* switch to task 1 */

  key_data = pthread_getspecific( Key_id );
  printf( "Init: Got the key(0x%x) value of %ld\n",Key_id,
          (unsigned long) ((unsigned int *)key_data - Data_array) );

  remaining = sleep( 1 );
  if ( remaining )
     printf( "seconds remaining = %d\n", remaining );
  assert( !remaining );

     /* switch to task 1 */

  /* delete the key */

  puts( "Init: pthread_key_delete - SUCCESSFUL" );
  status = pthread_key_delete( Key_id );
  if ( status )
    printf( "status = %d\n", status );
  assert( !status );

  printf( "Destructor invoked %d times\n", Destructor_invoked );

  puts( "*** END OF POSIX TEST 6 ***" );
  exit( 0 );
  return 0;
  
}


main() 
{
  pthread_init();
  POSIX_Init(0);
}

