#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>

void reader_function(void);
void writer_function(void);
  
char buffer = 0;
int buffer_has_item = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER(mutex);
struct timespec delay;
  
main() {
  pthread_t r;
  pthread_t w;
  
  delay.tv_sec = 2;
  delay.tv_nsec = 0;

  printf ("pthread_init:\n");
  pthread_init();
  printf ("pthread_create1:\n");
  pthread_create( &r, NULL, (void*)&reader_function, NULL);
  printf ("pthread_create2:\n");
  pthread_create( &w, NULL, (void*)&writer_function, NULL);
  printf ("pthread_join1:\n");
  pthread_join(r, NULL);
  printf ("pthread_join2:\n");
  pthread_join(w, NULL);
  printf ("exit\n");
}
  
void writer_function(void) {
  while(1) {
    pthread_mutex_lock( &mutex );
    if ( buffer_has_item == 0 ) {
      buffer++;
      buffer_has_item = 1;
    }
    pthread_mutex_unlock( &mutex );
    //pthread_delay_np( &delay );
  }
}
  
void reader_function(void) {
  while(1) {
    pthread_mutex_lock( &mutex );
    if ( buffer_has_item == 1) {
      printf("char:%x\n",buffer);
      buffer_has_item = 0;
    }
    pthread_mutex_unlock( &mutex );
    //pthread_delay_np( &delay );
  }
}

