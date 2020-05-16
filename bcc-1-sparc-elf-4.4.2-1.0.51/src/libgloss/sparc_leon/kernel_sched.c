/* Written by Konrad Eisele. (C) 2004  Gaisler Research AB */

#include <asm-leon/contextswitch.h>
#include <asm-leon/leonbare_kernel.h>
#include <asm-leon/leonbare_debug.h>
#include <asm-leon/stack.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>

unsigned int reschedule() {
    leonbare_sched_update();
    return leonbare_sched();
}

unsigned int leonbare_sched()
{
    unsigned int ret = 0;
    volatile leonbare_thread_t old = LEONBARE_KR_CURRENT, new =
        LEONBARE_KR_NEXT;
    LBDEBUG_FNCALL;
    LBDEBUG_HEADER_PRINTF(LBDEBUG_SCHED_NR, "switch %s[%x]->%s[%x]\n",
                          LEONBARE_TH_NAME_DBG(old), old,
                          LEONBARE_TH_NAME_DBG(new), new);
    
    LBPASSERT((old != new),"leonbare_sched should only be called with reschedule work to do",0);
    
    LEONBARE_KR_CURRENT = new;
    
    /* to be able to programm symetrically on kernel level each thread
       saves it's spinlock on mutexes and kernel and irq flags in its
       own save region. On a kernel switch they are released until the
       thread is reawakened. Then the locks will be reaquired (and finally
       released when the codeblock exits). The locking can be recursive. */
    if (old->th_prot.krp_k_depth) {
	LEONBARE_SMP_SPINLOCK_RELEASE(LEONBARE_KR_LOCK);
    }
    if (old->th_prot.krp_m_depth) {
	LEONBARE_SMP_SPINLOCK_RELEASE(old->th_prot.krp_m);
    }
    
    ret = _leonbare_kernel_switchto(old,new);
    optbarrier();
    
    if (new->th_prot.krp_m_depth) {
	LEONBARE_SMP_SPINLOCK_AQUIRE(new->th_prot.krp_m);
    }
    if (old->th_prot.krp_k_depth) {
	LEONBARE_SMP_SPINLOCK_AQUIRE(LEONBARE_KR_LOCK);
    }
    
    return ret;
}
