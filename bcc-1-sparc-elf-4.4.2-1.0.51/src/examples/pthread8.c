#include <stdio.h>
#include <stdlib.h>
#include <fsu_pthread.h>

void func1_function(void);
void func2_function(void);
  
main() {
  pthread_t r;
  pthread_t w;
  pthread_attr_t attr;
  pthread_init();
  pthread_create( &r, NULL, (void*)&func1_function, NULL);
  pthread_create( &w, NULL, (void*)&func2_function, NULL);
  pthread_join(r, NULL);
  pthread_join(w, NULL);
}
int errors = 0;

static void func() {
  int i = 0;
  while(1) {
    void *p[100];
    for (i = 0; i < 100; i++) {
      printf("\nMalloc %d\n",i);
      p[i] = malloc(i*1000+10);  
    }
    for (i = 0; i < 100; i++) {
      free(p[i]);
    }
  }
}

#define FP1_COUNT 10
void func1_function( void )
{
  static double fpt_values[FP1_COUNT];
  printf("Starting malloc test 1\n");
  pthread_self()->pt_name = "func1";
  while(1) {
    func( );
  }
}
  
#define FP2_COUNT 10
void func2_function(void) {
  static double fpt_values[FP2_COUNT];
  printf("Starting malloc test 2\n");
  pthread_self()->pt_name = "func2";
  while(1) {
    func( );
  }
}


