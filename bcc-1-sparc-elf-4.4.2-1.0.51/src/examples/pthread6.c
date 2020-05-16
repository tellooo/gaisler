#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>

void reader_function(void);
void writer_function(void);
  
char buffer = 0;
int buffer_has_item = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER(mutex);
struct timespec delay;

typedef struct idle_kernel_stack {
  char body[4096];
};
struct idle_kernel_stack idle_stack __attribute__((aligned(8)));

main() {
  pthread_t r;
  pthread_t w;
  pthread_attr_t attr;

  delay.tv_sec = 2;
  delay.tv_nsec = 0;

  printf("Using %x\n",attr.stack);
  
  pthread_init();
  pthread_create( &r, NULL, (void*)&reader_function, NULL);
  pthread_create( &w, NULL, (void*)&writer_function, NULL);
  pthread_join(r, NULL);
  pthread_join(w, NULL);
}
  
void writer_function(void) {
  int i = 0;
  pthread_self()->pt_name = "writer";
  while(1) {
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME , &now)) {
      printf("clock_gettime error\n");
    }
    printf("write-sleep1 ->(%ds %dns)\n",now.tv_sec, now.tv_nsec);
    sleep(5);
    clock_gettime(CLOCK_REALTIME , &now);
    printf("write-sleep1 <-(%ds %dns)\n",now.tv_sec, now.tv_nsec);
    i += 1;
  }
}
  
void reader_function(void) {
  int i = 0;
  pthread_self()->pt_name = "reader";
  while(1) {
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME , &now)) {
      printf("clock_gettime error\n");
    }
    printf("read-sleep1 ->(%ds %dns)\n",now.tv_sec, now.tv_nsec);
    sleep(2);
    clock_gettime(CLOCK_REALTIME , &now);
    printf("read-sleep1 <-(%ds %dns)\n",now.tv_sec, now.tv_nsec);
    i += 2;
  }
}

