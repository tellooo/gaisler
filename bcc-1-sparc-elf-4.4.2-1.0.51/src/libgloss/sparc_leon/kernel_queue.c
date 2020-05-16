/* Written by Konrad Eisele. (C) 2004  Gaisler Research AB */

#include <sys/fsu_pthread_queue.h>
#include <asm-leon/contextswitch.h>
#include <asm-leon/leonbare_kernel.h>
#include <asm-leon/leonbare_debug.h>
#include <asm-leon/stack.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>

int leonbare_thread_getqueueidx(leonbare_thread_t thread) {
    LEONBARE_VERIFYSCHED();
    if (thread->th_flags & (LEONBARE_TH_TERMINATED | LEONBARE_TH_FINISHED)) {
	return LEONBARE_RUNQ_KILLED_IDX;
    } else if ((thread->th_flags & LEONBARE_TH_SUSPENDED)) {
	return LEONBARE_RUNQ_SUSPENDED_IDX;
    } else if (LEONBARE_RUNQ_ISREADY(thread->th_runq_idx)) {
	if (LEONBARE_KR_RUNQ_WHICH == thread->th_runq_which) {
	    return thread->th_runq_idx;
	} else {
	    return thread->th_runq_idx + LEONBARE_RUNQ_PREPARE_IDX;
	}
    }
    return -1;
}

