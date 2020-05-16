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

static void func( double *values,
		  int count,
		  int loops) {
  while(1) {
    
    unsigned int i, j;
    // volatiles necessary to force
    // values to 64 bits for comparison
    volatile double sum = 1.0;
    volatile double last_sum;
    unsigned int seed;
    
#define V(__i) (values[(__i)%count])
#define CALC ((V(i-1)*V(i+1))*(V(i-2)*V(i+2))*(V(i-3)*sum))

    seed = ((unsigned int)&i)*count;
    
    // Set up an array of values...
    for( i = 0; i < count; i++ )
        values[i] = (double)rand( )/(double)0x7fffffff;

    // Now calculate something from them...
    for( i = 0; i < count; i++ )
        sum += CALC;
    last_sum = sum;
    
    // Now recalculate the sum in a loop and look for errors
    for( j = 0; j < loops ; j++ ) {
      sum = 1.0;
      for( i = 0; i < count; i++ )
	sum += CALC;
      
      if( sum != last_sum ){
	errors++;
	printf("Sum mismatch! %d sum=[%08x:%08x] last_sum=[%08x:%08x]\n",
	       j,
	       ((unsigned int *)&sum)[0],((unsigned int *)&sum)[1],
	       ((unsigned int *)&last_sum)[0],((unsigned int *)&last_sum)[1]
	       );
      }
    }
  }
}

#define FP1_COUNT 10
void func1_function( void )
{
  static double fpt_values[FP1_COUNT];
  printf("Starting FP testthread 1\n");
  pthread_self()->pt_name = "func1";
  func( fpt_values, FP1_COUNT, 10 );
}
  
#define FP2_COUNT 10
void func2_function(void) {
  static double fpt_values[FP2_COUNT];
  printf("Starting FP testthread 2\n");
  pthread_self()->pt_name = "func2";
  func( fpt_values, FP2_COUNT, 10 );
}


