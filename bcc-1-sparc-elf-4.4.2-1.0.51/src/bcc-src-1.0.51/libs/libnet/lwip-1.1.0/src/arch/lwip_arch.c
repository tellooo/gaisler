/* 
 * This file implements the leonbare's pthread specific sys_arch functions used by lwIP 
 */

#include "lwip/opt.h"
#include "arch/sys_arch.h"
#include "lwip/sys.h"
#include "lwip/def.h"


/* 
 * Timeout for threads which were not created by sys_thread_new
 * usually "main"
 */ 
struct sys_timeouts to;

/* 
 * Set up memory pools and threads
 */
void sys_init()
{
}

/*
 * Create a new mbox.If no memory is available return NULL 
 */
sys_mbox_t sys_mbox_new()
{
}

/*
 * Destroy the mbox and release the space it took up in the pool
 */
void sys_mbox_free(sys_mbox_t mbox)
{
}

/* 
 * cyg_mbox_put should not be passed a NULL otherwise the cyg_mbox_get will not
 * know if it's real data or error condition. But lwIP does pass NULL on occasion
 * in cases when maybe using a semaphore would be better. So this dummy_msg replaces
 * NULL data
 */

int dummy_msg = 1;

/* 
 * Post data to a mbox.
 */ 
void sys_mbox_post(sys_mbox_t mbox, void *data)
{
}

/* 
 * Fetch data from a mbox.Wait for at most timeout millisecs
 * Return -1 if timed out otherwise time spent waiting.
 */ 
u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **data, u32_t timeout)
{
}

/*
 * Create a new semaphore and initialize it.
 * If no memory is available return NULL 
 */
sys_sem_t sys_sem_new(u8_t count)
{
}

/* 
 * Wait on a semaphore for at most timeout millisecs
 * Return -1 if timed out otherwise time spent waiting.
 */ 
u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{
}

/*
 * Signal a semaphore
 */ 
void sys_sem_signal(sys_sem_t sem)
{
}

/* 
 * Destroy the semaphore and release the space it took up in the pool 
 */
void sys_sem_free(sys_sem_t sem)
{
}

/*
 * Create new thread 
 */
sys_thread_t sys_thread_new(void (*function) (void *arg), void *arg,int prio)
{
}

/* 
 * Return current thread's timeout info
 */
struct sys_timeouts *sys_arch_timeouts(void)
{
}
