#include <stdlib.h>
#include <asm-leon/leonbare_kernel.h>

struct leonbare_thread thread1 __attribute__((aligned(8)));
struct leonbare_thread thread2 __attribute__((aligned(8)));
char stack1[14096];
char stack2[14096];

int thread_fn1(void *a) 
{
  while(1) {
    int i = 0; int j = 0;
    /* for (i = 0;i < 10000;i++) { */
/*       j++; */
/*     } */
    printf("threads1\n"); 
  }
}

int thread_fn2(void *a) 
{
  while(1) {
    int i = 0; int j = 0;
/*     for (i = 0;i < 10000;i++) { */
/*       j++; */
/*     } */
    printf("threads2\n"); 
  }
}


int main() {
  PDEBUG_FLAGS_SET(LBDEBUG_FNCALL_NR | LBDEBUG_FNEXIT_NR | LBDEBUG_SCHED_NR | LBDEBUG_QUEUE_NR);
  printf("Starting threads\n");
  leonbare_thread_init();
  
  thread1.th_func = thread_fn1;
  thread2.th_func = thread_fn2;
  thread1.th_name = "thread1";
  thread2.th_name = "thread2";
  leonbare_thread_create(&thread1, stack1, sizeof(stack1));
  leonbare_thread_create(&thread2, stack2, sizeof(stack2));
  

  
  while(1) {
  }
  printf("End threads\n");
  return 0;
}
