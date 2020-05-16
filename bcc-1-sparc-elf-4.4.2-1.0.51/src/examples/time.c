#include <sys/types.h>
#include <sys/time.h>
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/time.h>
#include <asm-leon/timer.h>
#include <asm-leon/clock.h>
volatile int didtick  = 0;
volatile int cnt  = 0;
volatile int dide  = 0;

struct timerevent e; /* note: this has to be a global variable, it is inserted in a linked list by addtimer()  */ 
void register_event();

int do_tick(struct leonbare_pt_regs *regs){
	didtick = 1;
	return 0;
}

int event(void *arg) {
	dide = 1;
	register_event();
	return 0;
}

void register_event() 
{
	/* add an event 1s 10us from now */
	do_gettimeofday(&e.expire);
	e.handler = event;
	e.expire.tv_sec += 1;
	e.expire.tv_nsec += 1 * NSEC_PER_USEC /* resolution is usecs */;
	addtimer(&e);
}

int main() {
	/* init the 100 hz ticker */
	leonbare_init_ticks();
	ticker_callback = do_tick;
	
	register_event();
	
	while(1) {
		if (dide) {
			printf("- 1s 10us Event -\n");
			dide = 0;
		}
		if (didtick) {
			printf("%d: 100Hz tick\n", ++cnt);
			didtick = 0;
		}
	}
}

