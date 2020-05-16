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
 * Pthreads interface internals

  pthreads-bugs@ada.cs.fsu.edu
  Gaisler Research, Konrad Eisele<eiselekd@web.de>
  @(#)internals.h	3.14 11/8/00

*/

#ifndef _pthread_pthread_internals_h
#define _pthread_pthread_internals_h

#include <fsu_pthread/config.h>
#include <../src/config_internals.h>
#include <asm-leon/leonstack.h>
#include <fsu_pthread/debug.h>

#include "signal_internals.h"


/*
 * We needed for MINFRAME and WINDOWSIZE. 
 */
# define WINDOWSIZE (SF_REGS_SZ)
# define MINFRAME   (WINDOWSIZE + (7 * 4))
# define STACK_ALIGN 4
# define SA(X)       (((X)+(STACK_ALIGN-1)) & ~(STACK_ALIGN-1))
  typedef void* malloc_t;

#include <fsu_pthread.h>

#ifndef SRP
#ifdef _POSIX_THREADS_PRIO_PROTECT
 ERROR: undefine _POSIX_THREADS_PRIO_PROTECT in unistd.h when SRP is undefined!
#endif 
#else /* SRP */
#ifndef _POSIX_THREADS_PRIO_PROTECT
 ERROR: define _POSIX_THREADS_PRIO_PROTECT in unistd.h when SRP is defined!
#endif
#endif

/*#if defined(ASM_SETJMP) && defined(C_CONTEXT_SWITCH)
 ERROR: either ASM_SETJMP or C_CONTEXT_SWITCH can be defined, not both
#endif
*/

#ifdef STAND_ALONE
#define MAX_THREADS 200
#endif /* STAND_ALONE */

/* Other Program Specific Constants */
#define MAX_PRIORITY        256
#define MIN_PRIORITY        0
#define DEFAULT_PRIORITY    MIN_PRIORITY+1
#define DEFAULT_STACKSIZE   12288
#define DEFAULT_STACKSIZE_IDLE        DEFAULT_STACKSIZE
#define MAX_STACKSIZE       2097152
#define PTHREAD_BODY_OFFSET 200

#ifdef DEF_RR
#define TIMER_QUEUE   0
#else
#define TIMER_QUEUE   2
#endif

#ifdef SOLARIS
#define READ  _read
#define WRITE _write
#else /* !SOALRIS */
#define READ  read
#define WRITE write
#endif /* !SOLARIS */

/*
 * page alignment
 */
#define PA(X) ((((int)X)+((int)pthread_page_size-1)) & \
				~((int)pthread_page_size-1))

#include <sys/param.h>
#ifndef MAX
#define MAX(x, y) ((x > y)? x : y)
#endif

/*
 * timer queue
 */
#ifdef DEF_RR
typedef struct timer_ent {
        struct timeval tp;                     /* wake-up time                */
        pthread_t thread;                      /* thread                      */
        int mode;                              /* mode of timer (ABS/REL/RR)  */
        struct timer_ent *next[TIMER_QUEUE+1]; /* next request in the queue   */
} *timer_ent_t;
#else
//typedef pthread_t timer_ent_t;
#endif
 
#ifdef DEF_RR
typedef struct pthread_timer_q_s {
	struct timer_ent *head;
	struct timer_ent *tail;
} pthread_timer_q;
#elif defined(STAND_ALONE)
//typedef struct pthread_queue pthread_timer_q;
#else
//typedef struct pthread_queue pthread_timer_q;
#endif
//typedef pthread_timer_q *pthread_timer_q_t;

#define NO_QUEUE          ((pthread_queue_t) NULL)

#define NO_TIMER_QUEUE    ((pthread_timer_q_t) NULL)

#define NO_TIMER	  ((timer_ent_t) NULL)

#define NO_QUEUE_INDEX    0

#define	NO_QUEUE_ITEM	  ((struct pthread *) NULL)

#define	QUEUE_INITIALIZER { NO_QUEUE_ITEM, NO_QUEUE_ITEM }

#define	pthread_queue_init(q) \
	TAILQ_INIT(q)

#ifdef STAND_ALONE
/* #define	pthread_timer_queue_init(q) \ */
/* 	(q = NO_TIMER_QUEUE) */
#else
#define	pthread_timer_queue_init(q) \
	((q)->head = (q)->tail = NO_TIMER)
#endif

pthread_mutexattr_t pthread_mutexattr_default;

#ifdef _POSIX_THREADS_PRIO_PROTECT 
#ifdef SRP 
#define MUTEX_WAIT -2
#define NO_PRIO -1
#endif
#endif

#define	MUTEX_INITIALIZER	{ 0, QUEUE_INITIALIZER, 0, 1 }
#define NO_MUTEX      ((pthread_mutex_t *)0)
#define MUTEX_VALID 0x1

pthread_condattr_t pthread_condattr_default;

#define	CONDITION_INITIALIZER	{ QUEUE_INITIALIZER, 1, 0 }
#define NO_COND       ((pthread_cond_t *) 0)
#define COND_VALID 0x1

/*
 * SIGKILL, SIGSTOP cannot be masked; therefore reused as masks
 */
#define TIMER_SIG       SIGKILL
#define AIO_SIG         SIGSTOP

#ifndef TIMER_MAX
#define TIMER_MAX   _POSIX_TIMER_MAX+2
#endif

#define	NO_PTHREAD	((pthread_t) 0)

pthread_attr_t pthread_attr_default;

#define NO_ATTRIBUTE ((pthread_attr_t *)0)

struct pthread_action {
  void (*func)();
  any_t arg;
};

#ifdef PTHREAD_KERNEL
struct pthread_action pthread_pending_sigaction = { NULL, NULL};
#else
extern struct pthread_action  pthread_pending_sigaction;
#endif

struct pthread_cleanup {
  void (*func)();
  any_t arg;
  struct pthread_cleanup *next;
};

typedef struct kernel_stack {
  char body[TEMPSTACK_SIZE];
  char stack[SA(MINFRAME)+8];
} * kernel_stack_t;
#define pthread_tempstack_top SA(((unsigned long)&(pthread_tempstack->stack)))

TAILQ_HEAD(kqueue_t,pthread);

typedef struct kernel {
  pthread_t k_idle;
  pthread_t k_pthread_self;            /* thread that is currently running   */
  volatile int k_is_in_kernel;         /* flag to test if in kernel          */
  int k_is_updating_timer;             /* flag to test if timeout is handled */
  volatile int k_state_change;         /* dispatcher state (run q/signals)   */
  volatile sigset_t k_new_signals;     /* bit set of new signals to handle   */
  sigset_t k_pending_signals;          /* bit set of pending signals         */
  sigset_t k_all_signals;              /* mask of all (maskable) signals     */
  sigset_t k_no_signals;               /* mask of no signals                 */
  sigset_t k_cantmask;                 /* mask of signals (cannot be caught) */
  char *k_process_stack_base;          /* stack base of process              */
  
  struct pthread_queue k_ready;        /* ready queue                        */
  struct pthread_queue k_all;          /* queue of all threads               */
  struct pthread_queue k_suspend_q;    /* queue of suspended threads         */
  struct pthread_queue k_timeout_q;    /* queue of timed waiting threads     */

  TAILQ_HEAD(kdbg_mutex_t,pthread_cond) k_dbglist_cond;
  TAILQ_HEAD(kdbg_cond_t,pthread_mutex) k_dbglist_mutex;

  kernel_stack_t k_tempstack;
  
  sigset_t k_handlerset;               /* set of signals with user handler   */
  char *k_set_warning;                 /* pointer to set warning message     */
  char *k_clear_warning;               /* pointer to clear warning message   */
  char *k_prio_warning;                /* pointer to prio warning message    */
                                       /* for STAND_ALONE                    */
  sigset_t k_proc_mask;		       /* Mask for process                   */
  int  k_cur_heap;		       /* current break                      */
  volatile struct timespec k_timeofday;/* Time of Day                        */
#if defined(IO) && !defined(__FreeBSD__) && !defined(__dos__)
#if defined(USE_POLL)
  volatile int k_gnfds;                /* global width                       */
  volatile int k_gmaxnfds;		/* global max. number fds in k_gfds  */
  struct pollfd* k_gfds;		/* global poll events wait set */
  struct pollfd* k_gafds[2];		/* global poll events wait set */
#else /* !USE_POLL */
  volatile int k_gwidth;               /* global width                       */
  fd_set k_greadfds;                   /* global read file descriptor set    */
  fd_set k_gwritefds;                  /* global write file descriptor set   */ 
  fd_set k_gexceptfds;                 /* global except file descriptor set  */ 
#endif /* !USE_POLL */
#endif /* IO && !__FreeBSD__ && !__dos__ */
} kernel_t __attribute__((aligned(8)));

#ifdef PTHREAD_KERNEL
kernel_t pthread_kern;
#else
extern kernel_t pthread_kern;
#endif

/* Internal Functions */

/*
 * changed for speed-up and interface purposes -
 * pthread_self() is now a function, but mac_pthread_self() is used internally
 * #define pthread_self()  (pthread_kern.k_pthread_self == 0? \
 *                          NO_PTHREAD : pthread_kern.k_pthread_self)
 */
#define mac_pthread_self() pthread_kern.k_pthread_self
#define state_change       pthread_kern.k_state_change
#define is_in_kernel       pthread_kern.k_is_in_kernel
#define is_updating_timer  pthread_kern.k_is_updating_timer
#define new_signals        pthread_kern.k_new_signals
#define pending_signals    pthread_kern.k_pending_signals
#define all_signals        pthread_kern.k_all_signals
#define no_signals         pthread_kern.k_no_signals
#define cantmask           pthread_kern.k_cantmask
#define process_stack_base pthread_kern.k_process_stack_base
#define queues             pthread_kern.k_queues
#define ready              pthread_kern.k_ready
#define all                pthread_kern.k_all
#define timers             pthread_kern.k_timers
#define suspend_q          pthread_kern.k_suspend_q
#define handlerset         pthread_kern.k_handlerset
#define set_warning        pthread_kern.k_set_warning
#define clear_warning      pthread_kern.k_clear_warning
#define prio_warning       pthread_kern.k_prio_warning

#define dbglist_cond       pthread_kern.k_dbglist_cond
#define dbglist_mutex      pthread_kern.k_dbglist_mutex

#define pthread_tempstack  pthread_kern.k_tempstack
#define k_idle             pthread_kern.k_idle

#define pthread_timeout_q  pthread_kern.k_timeout_q

#define proc_mask	   pthread_kern.k_proc_mask
#define cur_heap           pthread_kern.k_cur_heap
#define timeofday          pthread_kern.k_timeofday

#if defined(IO) && !defined(__FreeBSD__) && !defined(__dos__)
#if defined(USE_POLL)
#define	gnfds		   pthread_kern.k_gnfds
#define gmaxnfds	   pthread_kern.k_gmaxnfds
#define gfds		   pthread_kern.k_gfds
#define gafds		   pthread_kern.k_gafds
#else /* !USE_POLL */
#define gwidth             pthread_kern.k_gwidth    
#define greadfds  	   pthread_kern.k_greadfds  
#define gwritefds 	   pthread_kern.k_gwritefds 
#define gexceptfds	   pthread_kern.k_gexceptfds
#endif /* !USE_POLL */
#endif /* IO && !__FreeBSD__ && !__dos__ */

#define set_errno(e)    (errno = (e))
#define get_errno()     (errno)

#define PT_REENT_SET(p) ((_impure_ptr=(p)->pt_reentp)==_impure_ptr)

/*
 * context switching macros, implemented via setjmp/longjmp plus saving errno
 */
#define SAVE_CONTEXT(t) \
  ( thread_setjmp((t)->context, FALSE) )

#define RESTORE_CONTEXT(t) thread_longjmp((t)->context, TRUE)

/*
 * set/clear Pthread kernel flag
 */

#if defined(DEBUG) 
#define SET_KERNEL_FLAG \
  MACRO_BEGIN \
    if (is_in_kernel) \
      fprintf(stderr, set_warning); \
    else \
      is_in_kernel = TRUE; \
  MACRO_END
#else
#define SET_KERNEL_FLAG is_in_kernel = TRUE
#endif

#define _SHARED_CLEAR_KERNEL_FLAG \
  MACRO_BEGIN \
    is_in_kernel = FALSE; \
    if (state_change) { \
      /*PDEBUG(PDBG_LIBC,"old-errno %d (0x%x)",get_errno(),_impure_ptr);*/ \
      is_in_kernel = TRUE; \
      if ((pthread_signonemptyset(&new_signals) || \
	   mac_pthread_self() != TAILQ_FIRST(&ready)) && \
          !SAVE_CONTEXT(mac_pthread_self())) \
        pthread_sched(); \
      state_change = FALSE; \
      is_in_kernel = FALSE; \
      while (pthread_signonemptyset(&new_signals)) { \
        is_in_kernel = TRUE; \
        pthread_sched_new_signals(mac_pthread_self(), TRUE); \
        if (!SAVE_CONTEXT(mac_pthread_self())) \
          pthread_sched(); \
        state_change = FALSE; \
        is_in_kernel = FALSE; \
      } \
      process_pending(); \
      /*PDEBUG(PDBG_LIBC,"new-errno %d (0x%x)",get_errno(),_impure_ptr);*/ \
    } \
  MACRO_END

#ifdef RR_SWITCH
#define _CLEAR_KERNEL_FLAG \
  MACRO_BEGIN \
    if ((mac_pthread_self()->queue == &ready) && \
	TAILQ_HASTWO(&ready,pt_qelem[PRIMARY_QUEUE]) && \
        (TAILQ_SECOND(&ready, pt_qelem[PRIMARY_QUEUE]) != k_idle) \
	) { \
      pthread_q_deq(&ready,mac_pthread_self(),PRIMARY_QUEUE); \
      pthread_q_enq_tail(&ready); \
    } \
    _SHARED_CLEAR_KERNEL_FLAG; \
  MACRO_END

#elif RAND_SWITCH
#define _CLEAR_KERNEL_FLAG \
  MACRO_BEGIN \
    if ((mac_pthread_self()->queue == &ready) && TAILQ_HASTWO(ready,pt_qelem[PRIMARY_QUEUE]) \
        && ((int)random()&01)) { \
      pthread_q_exchange_rand(&ready); \
    } \
    _SHARED_CLEAR_KERNEL_FLAG; \
  MACRO_END

#else
#if defined(DEBUG) 
#define _CLEAR_KERNEL_FLAG \
  MACRO_BEGIN \
    if (!is_in_kernel) \
      fprintf(stderr, clear_warning); \
    _SHARED_CLEAR_KERNEL_FLAG; \
    if (TAILQ_FIRST(ready)->attr.prio < ready.tail->attr.prio) \
      fprintf(stderr, prio_warning); \
  MACRO_END

#else /* !DEBUG  */
#define _CLEAR_KERNEL_FLAG _SHARED_CLEAR_KERNEL_FLAG
#endif /* DEBUG && !IO */
#endif

#define _SIG_CLEAR_KERNEL_FLAG(b) \
  MACRO_BEGIN \
    if(!SAVE_CONTEXT(mac_pthread_self())) \
      pthread_handle_pending_signals_wrapper(); \
    state_change = FALSE; \
    is_in_kernel = FALSE; \
    while (pthread_signonemptyset(&new_signals)) { \
      is_in_kernel = TRUE; \
      pthread_sched_new_signals(mac_pthread_self(), TRUE); \
      if (!SAVE_CONTEXT(mac_pthread_self())) \
        pthread_sched(); \
      state_change = FALSE; \
      is_in_kernel = FALSE; \
    } \
  MACRO_END

#define SIM_SYSCALL(cond)

void clear_kernel_flag(void);
void sig_clear_kernel_flag(int v);
//#define CLEAR_KERNEL_FLAG clear_kernel_flag();
//#define SIG_CLEAR_KERNEL_FLAG(b) sig_clear_kernel_flag(b)

#define CLEAR_KERNEL_FLAG _CLEAR_KERNEL_FLAG
#define SIG_CLEAR_KERNEL_FLAG(b) _SIG_CLEAR_KERNEL_FLAG(b)


#ifndef	MACRO_BEGIN

#define	MACRO_BEGIN	do {

#ifndef	lint
#define	MACRO_END	} while (0)
#else /* lint */
extern int _NEVER_;
#define	MACRO_END	} while (_NEVER_)
#endif /* lint */

#endif /* !MACRO_BEGIN */

pthread_t pthread_q_deq_head(pthread_queue_t q, int index);
void pthread_q_timeout_deq(pthread_queue_t q, pthread_t p);
int pthread_q_timeout_enq(pthread_queue_t q, pthread_t p);

#endif /* !_pthread_pthread_internals_h */
