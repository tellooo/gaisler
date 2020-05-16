#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>


pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER(count_mutex);
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER(condition_mutex);
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER(condition_cond);

void *functionCount1();
void *functionCount2();
int  count = 0;
#define COUNT_DONE  10
#define COUNT_HALT1  3
#define COUNT_HALT2  6

main()
{
   pthread_t thread1, thread2;
   printf("init\n");
   pthread_init();
   pthread_self()->pt_name = "main";

   PTHREAD_MUTEX_DBGSTART(condition_mutex,"cond-mutex");
   PTHREAD_MUTEX_DBGSTART(count_mutex,"count-mutex");
   PTHREAD_COND_DBGSTART(condition_cond,"cond");
   
   printf("create1\n");
   pthread_create( &thread1, NULL, &functionCount1, NULL);
   printf("create2\n");
   pthread_create( &thread2, NULL, &functionCount2, NULL);
   printf("join1\n");
   pthread_join( thread1, NULL);
   printf("join2\n");
   pthread_join( thread2, NULL);
}

void *functionCount1()
{
   pthread_self()->pt_name = "c1";
   for(;;)
   {
      pthread_mutex_lock( &condition_mutex );
      while( count >= COUNT_HALT1 && count <= COUNT_HALT2 )
      {
         pthread_cond_wait( &condition_cond, &condition_mutex );
      }
      pthread_mutex_unlock( &condition_mutex );

      pthread_mutex_lock( &count_mutex );
      count++;
      printf("Counter value functionCount1: %d\n",count);
      pthread_mutex_unlock( &count_mutex );

      if(count >= COUNT_DONE) {
	printf("Return 1\n");
	return(NULL);
      }
    }
}

void *functionCount2()
{
   pthread_self()->pt_name = "c2";
    for(;;)
    {
       pthread_mutex_lock( &condition_mutex );
       if( count < COUNT_HALT1 || count > COUNT_HALT2 )
       {
          pthread_cond_signal( &condition_cond );
       }
       pthread_mutex_unlock( &condition_mutex );

       pthread_mutex_lock( &count_mutex );
       count++;
       printf("Counter value functionCount2: %d\n",count);
       pthread_mutex_unlock( &count_mutex );

       if(count >= COUNT_DONE) {
	 printf("Return 2\n");
	 return(NULL);
       }
       
    }

}

