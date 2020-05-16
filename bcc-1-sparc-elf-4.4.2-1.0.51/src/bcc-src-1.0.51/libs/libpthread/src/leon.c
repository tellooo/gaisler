#include "internals.h"
#include "setjmp.h"
#include <fsu_pthread/asm.h>
#include <fsu_pthread/debug.h>
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/time.h>

int do_tick(struct pt_regs *regs);
struct pendingaction pending_tick = {{0,0},(pendinghandler)do_tick,0};
int do_tick(struct pt_regs *regs){
  if (!is_in_kernel) {
    pthread_t p = TAILQ_FIRST(&pthread_timeout_q);
    SET_KERNEL_FLAG;
    /* struct timespec tv; */
    /*   clock_gettime( CLOCK_REALTIME, &tv ); */
    /*   printf("%d,%d\n",tv.tv_sec,tv.tv_nsec); */
    if (p != NO_PTHREAD) {
      if (!GT_NTIME(p->tp, xtime)) {
	pthread_cancel_timed_sigwait(p, TRUE, ANY_TIME, p->queue != &ready);
      }
    }
    CLEAR_KERNEL_FLAG;
  } else {
    unsigned long old = leonbare_disable_traps();
    if (TAILQ_REMOVED(&pending_tick,next)) {
      add_pending(&pending_tick);
    }
    leonbare_enable_traps(old);
  }
  return 0;
}

int do_schedule(struct pt_regs *ptregs) {
  if (!is_in_kernel) {
    SET_KERNEL_FLAG;
    CLEAR_KERNEL_FLAG;
  }
}

void pthreadleon_init_ticks()
{
  int i;
  amba_apb_device dev[1];
  PTRACEIN;
  leonbare_init_ticks();
  schedule_callback = do_schedule;
  ticker_callback = do_tick;
}


