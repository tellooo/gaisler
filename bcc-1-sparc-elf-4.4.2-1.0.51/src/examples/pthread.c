#include <fsu_pthread.h>

void new_thread(int *arg) {
  printf("new thread argument = %d\n", *arg);
}
 
main() {
  pthread_t th;
  int i = 1;
  printf("starting pthread\n");
  pthread_init();
  printf("calling pthread create\n");
  pthread_create(&th, NULL, (pthread_func_t)new_thread, &i);
  printf("calling pthread join\n");
  pthread_join(th, NULL);
}

