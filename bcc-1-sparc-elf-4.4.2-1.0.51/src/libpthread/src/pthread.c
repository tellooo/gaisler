/* Copyright (C) 1992-2000 the Florida State University
   Distributed by the Florida State University under the terms of the
   GNU Library General Public License.

This file is part of Pthreads.

Pthreads is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation (version 2).

Pthreads is distributed "AS IS" in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with Pthreads; see the file COPYING.  If not, write
to the Free Software Foundation, 675 Mass Ave, Cambridge,
MA 02139, USA.

Report problems and direct all questions to:

  pthreads-bugs@ada.cs.fsu.edu

  @(#)pthread.c	3.14 11/8/00

*/

/*
 * Implementation of fork, join, exit, etc.
 */

#if defined (__FreeBSD__) || defined (_M_UNIX) || defined (__dos__)
# include<stdlib.h> /* why was this here for __linux__ ? */
#endif

#include "internals.h"
#include "setjmp.h"
#ifdef TDI_SUPPORT
#include "tdi.h"
#include "tdi-aux.h"
#include "tdi-dl.h"
#endif
#include <fsu_pthread/asm.h>
#include <fsu_pthread/debug.h>
#include <fsu_pthread/malloc.h>
#include <asm-leon/clock.h>

volatile int pthread_started = FALSE;
static volatile int n_pthreads = 0;
static volatile pthread_key_t n_keys = 0;
static void (*key_destructor[_POSIX_DATAKEYS_MAX])();

/*------------------------------------------------------------*/
/*
 * pthread_alloc - allocate pthread structure
 */
static pthread_t pthread_alloc(func, arg)
     pthread_func_t func;
     any_t arg;
{
  pthread_t t;
  
  t = (pthread_t) calloc(1, sizeof(struct pthread));
  if (t != NO_PTHREAD) {
    t->func = func;
    t->arg = arg;
    t->state = T_RUNNING | T_CONTROLLED;
  }
  PDEBUG(PDBG_ALLOC,"allocated: 0x%x",t);
  
  return(t);
}

/*------------------------------------------------------------*/
/*
 * pthread_self - returns immutable thread ID of calling thread
 */
pthread_t pthread_self()
{
  PTRACEIN;
  return(mac_pthread_self());
}

/*------------------------------------------------------------*/
/*
 * pthread_idle - idle function
 */
any_t pthread_idle(any_t __arg) {
  while(1);
}

void _pthread_init()
{
  pthread_t t;
  int i;
  threadctx_t env __attribute__((aligned(8)));
  PTRACEIN;

  PDEBUG(PDBG_START,"kernel: 0x%x, idle=0x%x",&pthread_kern,&k_idle);

  TAILQ_INIT(&ready);
  TAILQ_INIT(&all);
  TAILQ_INIT(&suspend_q);
  TAILQ_INIT(&pthread_timeout_q);

  if (!(pthread_tempstack = (kernel_stack_t) malloc(sizeof(struct kernel_stack))))
    return;
  
  pthread_init_signals();

  pthread_mutexattr_init(&pthread_mutexattr_default);
  pthread_condattr_init(&pthread_condattr_default);
  pthread_attr_init(&pthread_attr_default);

  t = pthread_alloc((pthread_func_t) 0, (any_t *) NULL);
  t->pt_name = "main";
  t->state |= T_MAIN | T_CONTROLLED;

  t->num_timers = 0;
  t->interval.tv_sec = 0;
  t->interval.tv_usec = 0;

  thread_setjmp(env, FALSE);

  process_stack_base = (char *) env[THREAD_JB_SP];
  pthread_stack_init(t);
  
  pthread_attr_init(&t->attr);
  t->attr.param.sched_priority = MIN_PRIORITY;
#ifdef _POSIX_THREADS_PRIO_PROTECT
  t->base_prio = MIN_PRIORITY;
#ifdef SRP
  t->max_ceiling_prio = NO_PRIO;
  t->new_prio = NO_PRIO;
#endif
#endif
#ifdef DEF_RR
  t->attr.sched = SCHED_RR;
#else
  t->attr.sched = SCHED_FIFO;
#endif
  /*
   * 1st thread inherits signal mask from process
   */
  pthread_sigcpyset2set(&t->mask, &proc_mask);
  t->pt_reentp = _impure_ptr;

  mac_pthread_self() = t;	
  pthread_q_all_enq(&all, t);
  pthread_q_primary_enq(&ready, t);
  state_change = FALSE;

  pthreadleon_init_ticks();
  
  pthread_started = TRUE;
#ifdef DEF_RR
  pthread_timed_sigwait(t, NULL, RR_TIME, NULL, t);
#endif

  
  n_pthreads = 1; /* main thread counts, fd_server does not count */

}


void pthread_init_attr(pthread_attr_t *attr) {
  
  PTRACEIN;
  if (pthread_started)
    return;
  
  _pthread_init();
  attr->name = "idle";
  pthread_create( &k_idle, attr, (void*)&pthread_idle, NULL);
}
  

/*------------------------------------------------------------*/
/*
 * pthread_init - initialize the threads package. This function is 
 * the first function that be called by any program using this package.
 * It initializes the main thread and sets up a stackspace for all the
 * threads to use. Its main purpose is to setup the mutexes and 
 * condition variables and allow one to access them.
 */

/*
 * the `constructor' attribute causes `pthread_init()' to be called 
 * automatically before execution enters `main()'.
 * (For all ports except SunOS and Solaris: see p_aux.S for init hook)
 */

/*void pthread_init(void) __attribute__ ((constructor));*/

void pthread_init() {
  pthread_attr_t attr;
  PTRACEIN;
  pthread_attr_init(&attr);
  attr.stacksize = DEFAULT_STACKSIZE_IDLE;
  attr.param.sched_priority = MIN_PRIORITY;
  pthread_init_attr(&attr);
}

static void pthread_terminate();

/*------------------------------------------------------------*/
/*
 * pthread_body - base of the pthreads implementation.
 * Procedure invoked at the base of each pthread (as a wrapper).
 */
void pthread_body()
{
  pthread_t t = mac_pthread_self();
#ifdef REAL_TIME
  struct timespec tp;
#endif /* REAL_TIME */
#ifdef DEBUG
#ifdef STAND_ALONE
pthread_timer_q_t pthread_timer;              /* timer queue                 */
#else
pthread_timer_q pthread_timer;                /* timer queue                 */
#endif
#endif /* DEBUG */

  PTRACEIN;
  PDEBUG(PDBG_START,"body-args 0x%x",t->arg);
  
  CLEAR_KERNEL_FLAG;

#ifdef REAL_TIME
  NTIMERCLEAR(tp);
  if (ISNTIMERSET(t->attr.starttime)) {
    tp.tv_sec = t->attr.starttime.tv_sec;
    tp.tv_nsec = t->attr.starttime.tv_nsec;
  }

  if (ISNTIMERSET(t->attr.deadline)) {
    PDEBUG(PDBG_START,"REAL_TIME: wait deadline [0x%x,0x%x]",t->attr.deadline.ts_sec,t->attr.deadline.ts_nsec);
    SET_KERNEL_FLAG;
    pthread_timed_sigwait(t, &t->attr.deadline, ABS_TIME, pthread_exit, -1);
    CLEAR_KERNEL_FLAG;
  }

  if (ISNTIMERSET(tp)) {
    PDEBUG(PDBG_START,"REAL_TIME: wait startime [0x%x,0x%x]",tp.tv_sec,tp.tv_nsec);
    pthread_absnanosleep(&tp);
  }
  
  do {
    if (ISNTIMERSET(tp) && !LE0_NTIME(t->attr.period)) {
      PDEBUG(PDBG_START,"REAL_TIME: > LESS ZERO [0x%x,0x%x]",t->attr.period.ts_sec,t->attr.period.ts_nsec);
      PLUS_NTIME(tp, tp, t->attr.period);
      SET_KERNEL_FLAG;
      pthread_timed_sigwait(t, &tp, ABS_TIME, pthread_exit, -1);
      CLEAR_KERNEL_FLAG;
    }
    if (!thread_setjmp(t->body, FALSE))
#endif /* REAL_TIME */
      
      t->result = (*(t->func))(t->arg);

#ifdef REAL_TIME
    if (ISNTIMERSET(tp) && !LE0_NTIME(t->attr.period)) {
      PDEBUG(PDBG_START,"REAL_TIME: < LESS ZERO [0x%x,0x%x]",t->attr.period.ts_sec,t->attr.period.ts_nsec);
      SET_KERNEL_FLAG;
      pthread_cancel_timed_sigwait(t, FALSE, ALL_TIME, FALSE);
      CLEAR_KERNEL_FLAG;
      pthread_absnanosleep(&tp);
    }
  } while (!LE0_NTIME(t->attr.period));
#endif /* REAL_TIME */

  pthread_terminate();
}

/*------------------------------------------------------------*/
/*
 * pthread_terminate - terminate thread: call cleanup handlers and
 * destructor functions, allow no more signals, dequeue from ready,
 * and switch context
 */
static void pthread_terminate()
{
  register pthread_t p, t = mac_pthread_self();
  register pthread_key_t i;
  register pthread_cleanup_t new;
#ifdef CLEANUP_HEAP
  register pthread_cleanup_t old;
#endif
  sigset_t abs_all_signals;

  PTRACEIN;
  
  SET_KERNEL_FLAG;
  if (!(t->state & T_EXITING)) {
    t->state |= T_EXITING;
    CLEAR_KERNEL_FLAG;
  }
  else {
    CLEAR_KERNEL_FLAG;
    return;
  }

  /*
   * call cleanup handlers in LIFO
   */
#ifdef CLEANUP_HEAP
  for (old = (pthread_cleanup_t) NULL, new = t->cleanup_top; new;
       old = new, new = new->next) {
    (new->func)(new->arg);
    if (old)
      free(old);
  }
  if (old)
    free(old);
#else
  for (new = t->cleanup_top; new; new = new->next)
    (new->func)(new->arg);
#endif

  /*
   * call destructor functions for data keys (if both are defined)
   */
  for (i = 0; i < n_keys; i++)
    if (t->key[i] && key_destructor[i])
      (key_destructor[i])(t->key[i]);

  /*
   * No more signals, also remove from queue of all threads
   */
  SET_KERNEL_FLAG;
  pthread_sigcpyset2set(&abs_all_signals, &all_signals);
  sigaddset(&abs_all_signals, SIGCANCEL);
  if (!pthread_siggeset2set(&t->mask, &abs_all_signals)) {

    if (t->state & (T_SIGWAIT | T_SIGSUSPEND)) {
      t->state &= ~(T_SIGWAIT | T_SIGSUSPEND);
      sigemptyset(&t->sigwaitset);
    }

    TAILQ_REMOVE(&all,t,pt_qelem[K_QUEUES_ALL]);
    pthread_sigcpyset2set(&t->mask, &abs_all_signals);

  }
  PTRACE_ALLQUEUE;
  
  /*
   * dequeue thread and schedule someone else
   */

  if (t->state & (T_SYNCTIMER | T_ASYNCTIMER)) {
    pthread_cancel_timed_sigwait(t, FALSE, ALL_TIME, FALSE);
  }
  t->state &= ~T_RUNNING;
  t->state &= ~T_EXITING;
  t->state |= T_RETURNED;

  /*
   * Terminating thread has to be detached if anyone tries to join
   * but the memory is not freed until the dispatcher is called.
   * This is required by pthread_join().
   * The result is copied into the TCB of the joining threads to
   * allow the memory of the current thread to be reclaimed before
   * the joining thread accesses the result.
   */
  if (TAILQ_FIRST(&t->joinq)) {
    t->state |= T_DETACHED;
    TAILQ_FOREACH(p,&t->joinq,pt_qelem[PRIMARY_QUEUE]) {
      p->result = t->result;
    }
    pthread_q_wakeup_all(&t->joinq, PRIMARY_QUEUE);
  }
  
#ifdef STACK_CHECK
  if (!(t->state & T_MAIN))
    pthread_unlock_all_stack(t);
#endif

  /*
   * The last threads switches off the light and calls UNIX exit
   */
  SIM_SYSCALL(TRUE);
  if (--n_pthreads) {
    pthread_q_deq(&ready, t, PRIMARY_QUEUE);
    CLEAR_KERNEL_FLAG;
  }
  else {
#ifdef STAND_ALONE
    exit(0);
#else
    pthread_clear_sighandler();
    pthread_process_exit(0);
#endif
  }
  
}

/*------------------------------------------------------------*/
/*
 * pthread_create - Create a new thread of execution. the thread 
 * structure and a queue and bind them together.
 * The completely created pthread is then put on the active list before
 * it is allowed to execute. Caution: The current implementation uses 
 * pointers to the thread structure as thread ids. If a thread is not 
 * valid it's pointer becomes a dangling reference but may still be 
 * used for thread operations. It's up to the user to make sure he 
 * never uses dangling thread ids. If, for example, the created thread
 * has a higher priority than the caller of pthread_create() and the 
 * created thread does not block, the caller will suspend until the 
 * child has terminated and receives a DANGLING REFERENCE as the 
 * thread id in the return value of pthread_create()! This 
 * implementation could be enhanced by modifying the type pthread_t of
 * a pointer to the thread control block and a serial number which had
 * to be compared with the serial number in the thread control block 
 * for each thread operation. Also, threads had to be allocated from a
 * fixed-size thread control pool or the serial number would become a 
 * "magic number".
 */
int pthread_create(thread, attr, func, arg)
     pthread_t *thread;
     pthread_attr_t *attr;
     pthread_func_t func;
     any_t arg;
{
  register pthread_t t;
  pthread_t parent_t = mac_pthread_self();
  PTRACEIN;

  if (!attr)
    attr = &pthread_attr_default;

  if (!attr->flags || thread == NULL)
    return(EINVAL);

#ifdef REAL_TIME
  {
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    if ((ISNTIMERSET(attr->starttime) && !GTEQ_NTIME(attr->starttime, now)) ||
	(ISNTIMERSET(attr->deadline)  && !GTEQ_NTIME(attr->deadline , now)) ||
	(ISNTIMERSET(attr->period) && !ISNTIMERSET(attr->starttime)))
      return(EINVAL);
  }
#endif /* REAL_TIME */

  t = pthread_alloc(func, arg);
  if (t == NO_PTHREAD)
    return(EAGAIN);

  t->attr.stacksize = attr->stacksize;
  t->attr.stack = attr->stack;
  t->attr.flags = attr->flags;
#ifdef _POSIX_THREADS_PRIO_PROTECT
#ifdef SRP
  t->max_ceiling_prio = NO_PRIO;
  t->new_prio = NO_PRIO;
#endif
#endif
  if (attr->inheritsched == PTHREAD_EXPLICIT_SCHED) {
    t->attr.contentionscope = attr->contentionscope;
    t->attr.inheritsched = attr->inheritsched;
    t->attr.sched = attr->sched;
    t->attr.param.sched_priority = attr->param.sched_priority;
#ifdef _POSIX_THREADS_PRIO_PROTECT
    t->base_prio = attr->param.sched_priority;
#endif
  }
  else {
    t->attr.contentionscope = parent_t->attr.contentionscope;
    t->attr.inheritsched = parent_t->attr.inheritsched;
    t->attr.sched = parent_t->attr.sched;
#ifdef _POSIX_THREADS_PRIO_PROTECT
    t->attr.param.sched_priority = t->base_prio = parent_t->base_prio;
#else
    t->attr.param.sched_priority = parent_t->attr.param.sched_priority;
#endif
  }
#ifdef REAL_TIME
  t->attr.starttime.tv_sec = attr->starttime.tv_sec;
  t->attr.starttime.tv_nsec = attr->starttime.tv_nsec;
  t->attr.deadline.tv_sec = attr->deadline.tv_sec;
  t->attr.deadline.tv_nsec = attr->deadline.tv_nsec;
  t->attr.period.tv_sec = attr->period.tv_sec;
  t->attr.period.tv_nsec = attr->period.tv_nsec;
#endif /* REAL_TIME */
#ifdef DEF_RR
  t->num_timers = 0;
  t->interval.tv_sec = 0;
  t->interval.tv_usec = 0;
#endif
  pthread_queue_init(&t->joinq);
  pthread_queue_init(&t->pt_active_timers);
  pthread_queue_init(&t->pt_free_timers);
  t->pt_name = attr->name;
  t->pt_reentp = &(t->pt_reent);
  _REENT_INIT_PTR(t->pt_reentp);
  
  /*
   * inherit the parent's signal mask
   */
  pthread_sigcpyset2set(&t->mask, &parent_t->mask);
  if (attr->detachstate == PTHREAD_CREATE_DETACHED)
    t->state |= T_DETACHED;
  *thread= t;

  ++n_pthreads; 
  if (!pthread_alloc_stack(t)) {
    return(ENOMEM);
  }
  
  pthread_initialize(t);

  SET_KERNEL_FLAG;
  pthread_q_all_enq(&all, t);
  pthread_q_primary_enq(&ready, t);
  CLEAR_KERNEL_FLAG;
	
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_suspend_internal() - suspend thread indefinitely,
 * assumes SET_KERNEL_FLAG
 */
void pthread_suspend_internal(t)
     pthread_t t;
{
  PTRACEIN;
  t->suspend.state = t->state & (T_RUNNING | T_BLOCKED);
  t->suspend.queue = t->queue;
  t->suspend.cond = t->cond;
  
  if (t->state & (T_RUNNING | T_BLOCKED))
    pthread_q_deq(t->queue, t, PRIMARY_QUEUE);
  if (t->cond) {
    if (!--t->cond->waiters)
      t->cond->mutex = NO_MUTEX;
    t->cond = NO_COND;
  }
  t->state |= T_SUSPEND;
  t->state &= ~(T_RUNNING | T_BLOCKED);
  pthread_q_primary_enq(&suspend_q, t);
}

/*------------------------------------------------------------*/
/*
 * pthread_suspend_np() - suspend thread indefinitely
 */
int pthread_suspend_np(t)
     pthread_t t;
{
  if (is_in_kernel)
    return(EINVAL);
  PTRACEIN;

  SET_KERNEL_FLAG;
  if (!t || t->state & (T_SUSPEND | T_EXITING | T_RETURNED)) {
    CLEAR_KERNEL_FLAG;
    return(EINVAL);
  }

  if (t->state & T_RUNNING) {
    pthread_pending_sigaction.func = pthread_suspend_internal;
    pthread_pending_sigaction.arg = t;
    /* issue a SIGALRM to force a suspend on the current thread, if requested */
#ifdef TDI_SUPPORT
    __pthread_debug_TDI_ignored_signals |= 0x1 << (SIGALRM-1);
#endif    
  }
  else
    pthread_suspend_internal(t);
  CLEAR_KERNEL_FLAG;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_resume_np() - resume suspended thread in saved state
 */
int pthread_resume_np(t)
     pthread_t t;
{
  int was_in_kernel = is_in_kernel;
  PTRACEIN;

  if (was_in_kernel && mac_pthread_self() != NULL)
    return(EINVAL);

  if (!was_in_kernel)
    SET_KERNEL_FLAG;
  if (!t || !(t->state & T_SUSPEND)) {
    if (!was_in_kernel)
      CLEAR_KERNEL_FLAG;
    return(EINVAL);
  }

 /*
  * assumption: Q ist 1st component in pthread_mutex_t structure !!!
  * check that cond is still associated with mutex
  */
  if (t->suspend.cond) {
    pthread_mutex_t *mutex = (pthread_mutex_t *) t->queue;
    if (t->suspend.cond->mutex != mutex) {
      CLEAR_KERNEL_FLAG;
      return(EINVAL);
    }
    t->suspend.cond->waiters++;
  }      

  pthread_q_deq(t->queue, t, PRIMARY_QUEUE);
  t->state = t->suspend.state;
  t->queue = t->suspend.queue;
  t->cond = t->suspend.cond;
  pthread_q_primary_enq(t->queue, t);
  if (!was_in_kernel)
    CLEAR_KERNEL_FLAG;
  if (!TAILQ_HASTWO(&ready, pt_qelem[PRIMARY_QUEUE]))
    kill(getpid(), SIGALRM); /* ignore signal but re-check ready queue */
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_equal() - Cmpares two threads. Returns
 *      0 if t1 <> t2
 *      1 if t1 == t2
 */
int pthread_equal(t1, t2)
     pthread_t t1, t2;
{
  PTRACEIN;
  return(t1 == t2);
}

/*------------------------------------------------------------*/
/*
 * pthread_detach - Detaching a running thread simply consists of 
 * marking it as such. If the thread has returned then the resources 
 * are also freed.
 */
int pthread_detach(thread)
     pthread_t thread;
{
  PTRACEIN;
  if (thread == NO_PTHREAD)
    return(EINVAL);

  SET_KERNEL_FLAG;

  if (thread->state & T_DETACHED) {
    CLEAR_KERNEL_FLAG;
    return(ESRCH);
  }
  
  thread->state |= T_DETACHED;

  if (thread->state & T_RETURNED) {
    free(thread->stack_base);
    free(thread);
    SIM_SYSCALL(TRUE);
  }

  CLEAR_KERNEL_FLAG;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_join - Wait for a thread to exit. If the status parameter is
 * non-NULL then that threads exit status is stored in it.
 */
int pthread_join(thread, status)
     pthread_t thread;
     any_t *status;
{
  register pthread_t p = mac_pthread_self();
  PTRACEIN;

  if (thread == NO_PTHREAD)
    return(EINVAL);
  
  if (thread == p)
    return(EDEADLK);

  SET_KERNEL_FLAG;
  
  if (thread->state & T_RETURNED) {
    if (thread->state & T_DETACHED) {
      CLEAR_KERNEL_FLAG;
      return(ESRCH);
    }

    if (status)
      *status = thread->result;

    thread->state |= T_DETACHED;

    free(thread->stack_base);
    free(thread);
    
    CLEAR_KERNEL_FLAG;
    return(0);
  }
  
  /*
   * clear error number before suspending
   */
  set_errno(0);

  pthread_q_sleep(&thread->joinq, PRIMARY_QUEUE);
  p->state |= T_INTR_POINT;
  if (sigismember(&p->pending, SIGCANCEL) &&
      !sigismember(&p->mask, SIGCANCEL))
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  else {
    SIM_SYSCALL(TRUE);
    CLEAR_KERNEL_FLAG;
  }
  PDEBUG(PDBG_RUN,"<-join returned");
  
  if (get_errno() == EINTR)
    return(EINTR);

  /*
   * status was copied into result field of current TCB by other thread
   */
  if (status)
    *status = p->result;
  return(0);
}

/*------------------------------------------------------------*/
/* Function:
 *    sched_yield -  Yield the processor to another thread.
 *    The current process is taken off the queue and another executes
 *    Simply put oneself at the tail of the queue.
 */
int sched_yield(void)
{
  PTRACEIN;
  SET_KERNEL_FLAG;
  if (TAILQ_HASTWO(&ready,pt_qelem[PRIMARY_QUEUE])) {
    /*
     * Place ourself at the end of the ready queue.
     * This allows the other ready processes to execute thus
     * in effect yielding the processor.
     */
    pthread_q_primary_enq(&ready, pthread_q_deq_head(&ready, PRIMARY_QUEUE));
    SIM_SYSCALL(TRUE);
  }
  CLEAR_KERNEL_FLAG;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_exit -  Save the exit status of the thread so that other 
 * threads joining with this thread can find it.
 */
void pthread_exit(status)
     any_t status;
{
  register pthread_t t = mac_pthread_self();
  PTRACEIN;
  t->result = status;
  pthread_terminate();
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_init - Initializes the attr (thread attribute object)
 * with the default values.
 */
int pthread_attr_init(attr)
     pthread_attr_t *attr;
{
  PTRACEIN;
  if (!attr)
    return(EINVAL);

  attr->flags = TRUE;
  attr->contentionscope = PTHREAD_SCOPE_PROCESS;
  attr->inheritsched = PTHREAD_EXPLICIT_SCHED;
  attr->detachstate = PTHREAD_CREATE_JOINABLE;
#ifdef DEF_RR
  attr->sched = SCHED_RR;
#else
  attr->sched = SCHED_FIFO;
#endif
  attr->stacksize = DEFAULT_STACKSIZE;
  attr->stack = 0;
  attr->name = 0;
  attr->param.sched_priority = DEFAULT_PRIORITY;
#ifdef REAL_TIME
  NTIMERCLEAR(attr->starttime);
  NTIMERCLEAR(attr->deadline);
  NTIMERCLEAR(attr->period);
#endif /* REAL_TIME */
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_destroy - Destroys the thread attribute object.
 */
int pthread_attr_destroy(attr)
     pthread_attr_t *attr;
{
  PTRACEIN;
  if (!attr || !attr->flags) {
    return(EINVAL);
  }
  attr->flags = FALSE;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_getschedparam - get the thread scheduling policy and params
 */
int pthread_getschedparam(thread, policy, param)
     pthread_t thread;
     int *policy;
     struct sched_param *param;
{
  PTRACEIN;
  if (thread == NO_PTHREAD) {
    return(ESRCH);
  }
  
  if (!policy || !param)
    return(EINVAL);

  *policy = thread->attr.sched;
#ifdef _POSIX_THREADS_PRIO_PROTECT
  param->sched_priority = thread->base_prio;
#else
  param->sched_priority = thread->attr.param.sched_priority;
#endif
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_setschedparam - Set the thread specific scheduling policy and params
 */
int pthread_setschedparam(thread, policy, param)
     pthread_t thread;
     int policy;
     struct sched_param *param;
{
  pthread_t p = mac_pthread_self();
  pthread_queue_t q;
  int oldsched, run_prio;
  PTRACEIN;

  if (thread == NO_PTHREAD) 
    return(ESRCH);

  if (!param)
    return(EINVAL);
  
#ifdef DEF_RR
  if (policy != SCHED_FIFO && policy != SCHED_RR) {
#else
  if (policy != SCHED_FIFO) {
#endif
    return(ENOTSUP);
  }

  if (param->sched_priority < MIN_PRIORITY ||
      param->sched_priority > MAX_PRIORITY)
    return(EINVAL);

#ifdef REAL_TIME
  if (ISNTIMERSET(thread->attr.starttime) ||
      ISNTIMERSET(thread->attr.deadline) ||
      ISNTIMERSET(thread->attr.period))
    return(EINVAL);
#endif /* REAL_TIME */

  SET_KERNEL_FLAG;
 
  if (thread->state & T_RETURNED) {
    CLEAR_KERNEL_FLAG;
    return(EINVAL);
  }

  oldsched = thread->attr.sched;
  thread->attr.sched = policy;
#ifdef DEF_RR
  if (policy != oldsched && p == thread)
    switch (oldsched) {
    case SCHED_FIFO:
      pthread_timed_sigwait(thread, NULL, RR_TIME, NULL, thread);
      break;
    case SCHED_RR:
      pthread_cancel_timed_sigwait(thread, FALSE, RR_TIME,
					 thread->queue != &ready);
      break;
    default:
      ;
    }
#endif
#ifdef _POSIX_THREADS_PRIO_PROTECT
#ifdef SRP
  if (thread->new_prio == MUTEX_WAIT) {
    thread->new_prio = param->sched_priority;
    CLEAR_KERNEL_FLAG;
    return (0);
  }
#endif
  run_prio = thread->attr.param.sched_priority;
  thread->base_prio = param->sched_priority;
  if (thread->max_ceiling_prio != NO_PRIO)
    thread->attr.param.sched_priority =
      MAX(param->sched_priority, thread->max_ceiling_prio);
  else
    thread->attr.param.sched_priority = param->sched_priority;
#else
  run_prio = thread->attr.param.sched_priority;
  thread->attr.param.sched_priority =
    param->sched_priority;
#endif
  q = thread->queue;
  if (TAILQ_FIRST(q) != thread ||
      (TAILQ_HASTWO(q,pt_qelem[PRIMARY_QUEUE]) &&
       thread->attr.param.sched_priority < run_prio &&
       TAILQ_NEXT(thread,pt_qelem[PRIMARY_QUEUE]) &&
       thread->attr.param.sched_priority <
       TAILQ_NEXT(thread,pt_qelem[PRIMARY_QUEUE])->attr.param.sched_priority)) {
    pthread_q_deq(q, thread, PRIMARY_QUEUE);
    pthread_q_primary_enq_first(q, thread);
  }
 
  SIM_SYSCALL(TRUE);
  CLEAR_KERNEL_FLAG;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setstacksize - Aligns the "stacksize" to double
 * word boundary and then sets the size of the stack to "stacksize"
 * in thread attribute object "attr".
 */
int pthread_attr_setstacksize(attr, stacksize)
     pthread_attr_t *attr;
     size_t stacksize;
{
  PTRACEIN;
  if (!attr || !attr->flags)
    return(EINVAL);

  attr->stacksize = SA(stacksize); /* stack align, see asm_linkage.h */
  return(0);    
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getstacksize - gets the stacksize from an pthread 
 * attribute object.
 */
int pthread_attr_getstacksize(attr, stacksize)
     pthread_attr_t *attr;
     size_t *stacksize;
{
  PTRACEIN;
  if (!attr || !attr->flags)
    return(EINVAL);

  *stacksize = attr->stacksize;    
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setscope - Set the contentionscope attribute in a 
 * thread attribute object.
 */
int pthread_attr_setscope(attr,contentionscope)
     pthread_attr_t *attr;
     int contentionscope;
{
  PTRACEIN;
  if (!attr || !attr->flags) {
    return(EINVAL);
  }
  if (contentionscope == PTHREAD_SCOPE_PROCESS) {
    attr->contentionscope=contentionscope;
    return(0);    
  }
  else
    return(EINVAL);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setinheritsched - Set the inheritsched attribute.
 */
int pthread_attr_setinheritsched(attr, inheritsched)
     pthread_attr_t *attr;
     int inheritsched;
{
  PTRACEIN;
  if (!attr || !attr->flags)
    return(EINVAL);
  if (inheritsched == PTHREAD_INHERIT_SCHED ||
      inheritsched == PTHREAD_EXPLICIT_SCHED) {
    attr->inheritsched = inheritsched;
    return(0);    
  }
  else
    return(EINVAL);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setschedpolicy - set the sched attribute
 */
int pthread_attr_setschedpolicy(attr, policy)
     pthread_attr_t *attr;
     int policy;
{
  PTRACEIN;
  if (!attr || !attr->flags)
    return(EINVAL);

#ifdef DEF_RR
  if (policy == SCHED_FIFO || policy == SCHED_RR) {
#else
  if (policy == SCHED_FIFO) {
#endif
    attr->sched = policy;
    return(0);    
  }
  else
    return(EINVAL);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setschedparam - set the sched param attribute
 */
int pthread_attr_setschedparam(attr, param)
     pthread_attr_t *attr;
     struct sched_param *param;
{
  PTRACEIN;
  if (!attr || !attr->flags || !param)
    return(EINVAL);

  if (param->sched_priority >= MIN_PRIORITY &&
      param->sched_priority <= MAX_PRIORITY) {
    attr->param.sched_priority = param->sched_priority;
    return(0);    
  }
  else
    return(EINVAL);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getscope - return contentionscope attribute in "contentionscope"
 */
int pthread_attr_getscope(attr, contentionscope)
     pthread_attr_t *attr;
     int *contentionscope;
{
  PTRACEIN;
  if (!attr || !attr->flags || !contentionscope)
    return(EINVAL);
  *contentionscope = attr->contentionscope;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getinheritsched - return inheritsched attr in "inheritsched"
 */
int pthread_attr_getinheritsched(attr, inheritsched)
     pthread_attr_t *attr;
     int *inheritsched;
{
  PTRACEIN;
  if (!attr || !attr->flags || !inheritsched)
    return(EINVAL);
  *inheritsched = attr->inheritsched;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getschedpolicy - get the sched attribute
 */
int pthread_attr_getschedpolicy(attr, policy)
     pthread_attr_t *attr;
     int *policy;
{
  PTRACEIN;
  if (!attr || !attr->flags || !policy)
    return(EINVAL);
  *policy = attr->sched;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getschedparam - return the sched param attr in "param"
 */
int pthread_attr_getschedparam(attr, param)
     pthread_attr_t *attr;
     struct sched_param *param;
{
  PTRACEIN;
  if (!attr || !attr->flags || !param)
    return(EINVAL);
  param->sched_priority = attr->param.sched_priority;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setstarttime_np - set the absolute start time
 */
int pthread_attr_setstarttime_np(attr, tp)
     pthread_attr_t *attr;
     struct timespec *tp;
{
  PTRACEIN;
#ifndef REAL_TIME
  return(ENOSYS);
#else /* REAL_TIME */
  if (!attr || !attr->flags || !tp || !ISNTIMERSET(*tp))
    return(EINVAL);

  attr->starttime.tv_sec = tp->tv_sec;
  attr->starttime.tv_nsec = tp->tv_nsec;

  return(0);
#endif /* REAL_TIME */
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getstarttime_np - get the absolute start time
 */
int pthread_attr_getstarttime_np(attr, tp)
     pthread_attr_t *attr;
     struct timespec *tp;
{
  PTRACEIN;
#ifndef REAL_TIME
  return(ENOSYS);
#else /* REAL_TIME */
  if (!attr || !attr->flags || !tp || !ISNTIMERSET(attr->starttime))
    return(EINVAL);

  tp->tv_sec = attr->starttime.tv_sec;
  tp->tv_nsec = attr->starttime.tv_nsec;

  return(0);
#endif /* REAL_TIME */
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setdeadline_np - set the absolute deadline (XOR period)
 */
int pthread_attr_setdeadline_np(attr, tp, func)
     pthread_attr_t *attr;
     struct timespec *tp;
     pthread_func_t func;
{
  PTRACEIN;
#ifndef REAL_TIME
  return(ENOSYS);
#else /* REAL_TIME */
  extern struct sigaction pthread_user_handler[NNSIG];

  if (!attr || !attr->flags || !tp || !ISNTIMERSET(*tp) ||
      ISNTIMERSET(attr->period))
    return(EINVAL);

  attr->deadline.tv_sec = tp->tv_sec;
  attr->deadline.tv_nsec = tp->tv_nsec;
  sigaddset(&handlerset, TIMER_SIG);
  pthread_user_handler[TIMER_SIG].sa_handler = (void(*)()) func;

  return(0);
#endif /* REAL_TIME */
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getdeadline_np - get the absolute deadline
 */
int pthread_attr_getdeadline_np(attr, tp)
     pthread_attr_t *attr;
     struct timespec *tp;
{
  PTRACEIN;
#ifndef REAL_TIME
  return(ENOSYS);
#else /* REAL_TIME */
  if (!attr || !attr->flags || !tp || !ISNTIMERSET(attr->deadline))
    return(EINVAL);

  tp->tv_sec = attr->deadline.tv_sec;
  tp->tv_nsec = attr->deadline.tv_nsec;

  return(0);
#endif /* REAL_TIME */
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setperiod_np - set the relative period (XOR deadline)
 */
int pthread_attr_setperiod_np(attr, tp, func)
     pthread_attr_t *attr;
     struct timespec *tp;
     pthread_func_t func;
{
  PTRACEIN;
#ifndef REAL_TIME
  return(ENOSYS);
#else /* REAL_TIME */
  extern struct sigaction pthread_user_handler[NNSIG];

  if (!attr || !attr->flags || !tp || !ISNTIMERSET(*tp) ||
      ISNTIMERSET(attr->deadline))
    return(EINVAL);

  attr->period.tv_sec = tp->tv_sec;
  attr->period.tv_nsec = tp->tv_nsec;
  sigaddset(&handlerset, TIMER_SIG);
  pthread_user_handler[TIMER_SIG].sa_handler = (void(*)()) func;

  return(0);
#endif /* REAL_TIME */
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getperiod_np - get the relative period
 */
int pthread_attr_getperiod_np(attr, tp)
     pthread_attr_t *attr;
     struct timespec *tp;
{
  PTRACEIN;
#ifndef REAL_TIME
  return(ENOSYS);
#else /* REAL_TIME */
  if (!attr || !attr->flags || !tp || !ISNTIMERSET(attr->period))
    return(EINVAL);

  tp->tv_sec = attr->period.tv_sec;
  tp->tv_nsec = attr->period.tv_nsec;

  return(0);
#endif /* REAL_TIME */
}

/*------------------------------------------------------------*/
/*
 * pthread_key_create - creates a new global key and spefies destructor call
 * returns 0 or upon error ENOMEM if name space exhausted,
 *                         EAGAIN if insufficient memory.
 */
int pthread_key_create(key, destructor)
     pthread_key_t *key;
     void (*destructor)();
{
  PTRACEIN;
  SET_KERNEL_FLAG;

  if (n_keys >= _POSIX_DATAKEYS_MAX) {
    CLEAR_KERNEL_FLAG;
    return(ENOMEM);
  }

  key_destructor[n_keys] = destructor;
  *key = n_keys++;
  CLEAR_KERNEL_FLAG;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_key_delete - deletes a thread-specific data key previously
 * returned by pthread_key_create().
 */
int pthread_key_delete(key)
  pthread_key_t key;
{
  PTRACEIN;
  if (key < 0 || key >= n_keys) {
    return(EINVAL);
  }

  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_setspecific - associate a value with a data key for some thread
 * return 0 or upon error EINVAL if the key is invalid.
 */
int pthread_setspecific(key, value)
     pthread_key_t key;
     any_t value;
{
  PTRACEIN;
  if (key < 0 || key >= n_keys) {
    return(EINVAL);
  }

  mac_pthread_self()->key[key] = value;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_getspecific - retrieve a value from a data key for some thread
 * returns NULL upon error if the key is invalid.
 */
any_t pthread_getspecific(key)
     pthread_key_t key;
{
  PTRACEIN;
  if (key < 0 || key >= n_keys) {
    return(NULL);
  }

  return mac_pthread_self()->key[key];
}

/*------------------------------------------------------------*/
/*
 * pthread_cleanup_push - push function on current thread's cleanup stack
 * and execute it with the specified argument when the thread.
 * Notice: pthread_cleanup_push_body() receives the address of the first
 * cleanup structure in an additional parameter "new".
 * (a) exits;
 * (b) is cancelled;
 * (c) calls pthread_cleanup_pop(0 with a non-zero argument;
 * (d) NOT IMPLEMENTED, CONTROVERSIAL: when a longjump reaches past the scope.
 */
#ifdef CLEANUP_HEAP
void pthread_cleanup_push(func, arg)
#else
void pthread_cleanup_push_body(func, arg, new)
#endif
     void (*func)();
     any_t arg;
#ifdef CLEANUP_HEAP
{
  pthread_cleanup_t new;
#else
     pthread_cleanup_t new;
{
#endif
  pthread_t p = mac_pthread_self();
  PTRACEIN;

  if (!func) {
    return;
  }

#ifdef CLEANUP_HEAP
  if (!(new = (pthread_cleanup_t) malloc(sizeof(*new))))
    return;
#endif

  new->func = func;
  new->arg = arg;

  SET_KERNEL_FLAG;
  new->next = p->cleanup_top;
  p->cleanup_top = new;
  CLEAR_KERNEL_FLAG;

  return;
}


/*------------------------------------------------------------*/
/*
 * pthread_cleanup_pop - pop function off current thread's cleanup stack
 * and execute it with the specified argument if non-zero
 */
#ifdef CLEANUP_HEAP
void pthread_cleanup_pop(execute)
#else
void pthread_cleanup_pop_body(execute)
#endif
     int execute;
{
  pthread_t p = mac_pthread_self();
  pthread_cleanup_t new;
  PTRACEIN;

  SET_KERNEL_FLAG;
  if (!(new = p->cleanup_top)) {
    CLEAR_KERNEL_FLAG;
    return;
  }
  p->cleanup_top = new->next;
  CLEAR_KERNEL_FLAG;

  if (execute)
    (new->func)(new->arg);
#ifdef CLEANUP_HEAP
  free(new);
#endif

  return;
}

/*------------------------------------------------------------*/
/*
 * sched_get_priority_max - inquire maximum priority value (part of .4)
 */
int sched_get_priority_max(policy)
     int policy;
{
  PTRACEIN;
  switch (policy) {
  case SCHED_RR:
  case SCHED_FIFO:
    return(MAX_PRIORITY);
    break;
  default:
    set_errno(EINVAL);
    return(-1);
  };
  return(MAX_PRIORITY);
}

/*------------------------------------------------------------*/
/*
 * sched_get_priority_min - inquire minimum priority value (part of .4)
 */
int sched_get_priority_min(policy)
     int policy;
{
  PTRACEIN;
  switch (policy) {
  case SCHED_RR:
  case SCHED_FIFO:
    return(MIN_PRIORITY);
    break;
  default:
    set_errno(EINVAL);
    return(-1);
  };
  return(MIN_PRIORITY);
}

/*------------------------------------------------------------*/
/*
 * sched_rr_get_interval - timeslice interval
 */
int sched_rr_get_interval(pid,interval)
     pid_t             pid;
     struct timespec  *interval;
{
  PTRACEIN;
  if ( pid && pid != getpid() ) {
    return(ESRCH);
  }
  interval->tv_sec  = TIME_SLICE / USEC_PER_SEC;
  interval->tv_nsec = (TIME_SLICE % USEC_PER_SEC) * NSEC_PER_USEC;
  return 0;
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_setdetachstate - Sets the detachstate attribute in 
 *                               attr object
 */
int pthread_attr_setdetachstate(attr, detachstate)
     pthread_attr_t *attr;
     int detachstate;
{
  PTRACEIN;
  if (!attr || !attr->flags || detachstate > PTHREAD_CREATE_DETACHED || detachstate < PTHREAD_CREATE_JOINABLE)
    return(EINVAL);
  attr->detachstate = detachstate;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_attr_getdetachstate - Gets the value of detachstate attribute
 *                               from attr object
 */
int pthread_attr_getdetachstate(attr, detachstate)
     pthread_attr_t *attr;
     int *detachstate;
{
  PTRACEIN;
  if (!attr || !attr->flags || !detachstate)
    return(EINVAL);
  *detachstate = attr->detachstate; 
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_once - The first call to this will call the init_routine()
 *                with no arguments. Subsequent calls to pthread_once()
 *                will not call the init_routine.
 */
int pthread_once(once_control, init_routine)
     pthread_once_t *once_control;
     void (*init_routine)();
{
  PTRACEIN;
  SET_KERNEL_FLAG;

  if (!once_control || !init_routine)
      return EINVAL;
      
  if (!once_control->init) {
    once_control->init = TRUE;
    pthread_mutex_init(&once_control->mutex, NULL);
  }
  CLEAR_KERNEL_FLAG;

  pthread_mutex_lock(&once_control->mutex);
  if (!once_control->exec) {
    once_control->exec = TRUE;
    (*init_routine)();
  }
  pthread_mutex_unlock(&once_control->mutex);
  return(0);
}

