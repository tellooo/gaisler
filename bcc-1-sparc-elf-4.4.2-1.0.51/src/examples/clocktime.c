#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <asm-leon/param.h>
#include "dhry.h"

main() {
	struct tms      time_info;
	while(1) {
		int c = clock();
		int c2;
		
		times (&time_info);
		c2 = (long) time_info.tms_utime;
		printf("%i %i\n",c/CLK_TCK, c2/HZ );
	}
	
}
