/* Written by Konrad Eisele. (C) 2004  Gaisler Research AB */

#include <sys/fsu_pthread_queue.h>
#include <asm-leon/contextswitch.h>
#include <asm-leon/leonbare_kernel.h>
#include <asm-leon/leonbare_debug.h>
#include <asm-leon/stack.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>
 
leonbare_mutex_t leonbare_mutex_init(leonbare_mutex_t m)
{
    m->mx_owner_cnt = 0;
    m->mx_owner = 0;
    LBTAILQ_INIT(&(m->mx_threads));
    
    LEONBARE_PROTECT_KERNEL_START();
    {
        LBTAILQ_INSERT_TAIL(LEONBARE_KR_ALLM, m, mx_allm);
    }
    LEONBARE_PROTECT_KERNEL_END();

}

int _self__leonbare_mutex_lock(leonbare_mutex_t m, int wait)
{
    int ret = LEONBARE_MUTEX_LOCK_OK; leonbare_thread_t c;
    
    LEONBARE_PROTECT_MUTEXSTRUCT_START(m);
    while(1) {
	if (LEONBARE_MUTEX_OWNER_GET(m) == 0) {
	    LEONBARE_MUTEX_OWNER_SET(m,LEONBARE_KR_CURRENT);
	    LEONBARE_MUTEX_OWNER_CNT_SET(m,0);
	    LBTAILQ_INSERT_TAIL(&c->th_mutex_locked, m, mx_locked);
	    ret = LEONBARE_MUTEX_LOCK_OK;
	    break; 
	} else if (m->mx_owner == LEONBARE_KR_CURRENT) {
	    m->mx_owner_cnt++;
	    ret = LEONBARE_MUTEX_LOCK_OK;
	    break;
	}
	LBTAILQ_INSERT_TAIL(&m->mx_threads, c, th_mutex);
	current_suspend();
    }
    LEONBARE_PROTECT_MUTEXSTRUCT_END(m);
    return ret;
}

int leonbare_mutex_unlock(leonbare_mutex_t m)
{
    int ret = LEONBARE_MUTEX_UNLOCK_ERROR; leonbare_thread_t c, h;
    
    LEONBARE_PROTECT_MUTEXSTRUCT_START(m);
    {
	c = LEONBARE_KR_CURRENT;
        if (m->mx_owner != c) {
            ret = LEONBARE_MUTEX_UNLOCK_OK;
        } else if (m->mx_owner == c && m->mx_owner_cnt) {
            m->mx_owner_cnt--;
            ret = LEONBARE_MUTEX_UNLOCK_OK;
        } else if ((h = LBTAILQ_FIRST(&m->mx_threads))) {
            LBTAILQ_REMOVE(&m->mx_threads, h, th_mutex);
	    leonbare_thread_resume(h);
            ret = LEONBARE_MUTEX_UNLOCK_OK;
        } 
    }
    LEONBARE_PROTECT_MUTEXSTRUCT_END(m);
    return ret;
}

