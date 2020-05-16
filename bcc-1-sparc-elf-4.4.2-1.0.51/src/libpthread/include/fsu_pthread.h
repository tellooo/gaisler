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
  Gaisler Research, Konrad Eisele<eiselekd@web.de>

  @(#)pthread.h	3.14 11/8/00

*/

#ifndef _pthread_pthread_h
#define _pthread_pthread_h

/*
 * Pthreads interface definition
 */


#include <fsu_pthread/config.h>
#include <fsu_pthread/signal.h>
#include <fsu_pthread/unistd.h>
#include <stdio.h>
#include <fsu_pthread/limits.h>
#include <sys/time.h>            /* needed for struct timeval */
#include <fsu_pthread/errno.h>
#include <fsu_pthread/asm.h>
#include <setjmp.h>
#include <reent.h>

#include <sys/fsu_pthread_queue.h>
#include <asm-leon/contextswitch.h>

#ifdef IO_NP
#include <sys/types.h>
#endif /* IO_NP */

#if defined(MALLOC_NP) || defined(STAND_ALONE)
#ifdef malloc
#undef malloc
#endif
#ifdef calloc
#undef calloc
#endif
#ifdef free
#undef free
#endif
#ifdef cfree
#undef cfree
#endif
#endif /* MALLOC_NP */

#include <fsu_pthread/setjmp.h>

/* contentionscope attribute values */
#define PTHREAD_SCOPE_SYSTEM    0
#define PTHREAD_SCOPE_PROCESS   1

/* creation modes */
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

/* inheritsched attribute values */
#define PTHREAD_INHERIT_SCHED   0
#define PTHREAD_EXPLICIT_SCHED  1

/* Allow variable stack sizes */
#ifndef _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_ATTR_STACKSIZE
#endif

#if defined (__GNUC__) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)

#ifndef _C_PROTOTYPE
#define _C_PROTOTYPE(arglist) arglist
#endif /* !_C_PROTOTYPE */

typedef void *any_t;

#else /* !(defined (__GNUC__) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)) */

#ifndef _C_PROTOTYPE
#define _C_PROTOTYPE(arglist) ()
#endif /* !_C_PROTOTYPE */

typedef char *any_t;

#ifndef const
#define const
#endif

#ifndef volatile
#define volatile
#endif

#endif /* defined(__GNUC__) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus) */

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#if !defined(__sys_stdtypes_h) && !defined(_SIZE_T) && !defined(__FreeBSD__) && !defined(_M_UNIX) && !defined(__linux__) && !defined(__dos__)
typedef unsigned int size_t;
#endif

/*
 * Mutex objects.
 */

#include <sys/fsu_pthread_mutex.h>

/*
 * Once Objects.
 */

typedef struct {
        short init;
        short exec;
        pthread_mutex_t mutex;
} pthread_once_t;

#define PTHREAD_ONCE_INIT(p) {FALSE,FALSE,PTHREAD_MUTEX_INITIALIZER(p.mutex)};

/*
 * Condition variables.
 */
typedef struct pthread_cond {
        struct pthread_queue queue;
        int flags;
        int waiters;
        pthread_mutex_t *mutex;
        TAILQ_ENTRY(pthread_cond) dbglist; 
        char *dbgname;
} pthread_cond_t;

typedef struct {
        int flags;
} pthread_condattr_t;

#define PTHREAD_COND_INITIALIZER(c) { 		\
  TAILQ_HEAD_INITIALIZER(c.queue), /* queue */	\
  1,        /* flags */				\
  0,        /* waiters */			\
  0,        /* mutex */				\
  { 0, 0 },                                     \
  0                                             \
}

#define PTHREAD_COND_DBGSTART(c, n)             \
  pthread_cond_dbgstart_np(&c);                 \
  c.dbgname = n;
#define PTHREAD_COND_DBGSTOP(c, n)              \
  pthread_cond_dbgstrop_np(&c);
#define PTHREAD_MUTEX_DBGSTART(m, n)            \
  pthread_mutex_dbgstart_np(&m);                \
  m.dbgname = n;
#define PTHREAD_MUTEX_DBGSTOP(m, n)             \
  pthread_mutex_dbgstop_np(&m);

/*******************************/
/*      Condition Functions    */
/*Condition Attribute Functions*/
/*******************************/

#ifdef __cplusplus
extern "C" {
#endif
 
extern int pthread_cond_destroy     _C_PROTOTYPE((pthread_cond_t *__cond));
extern int pthread_cond_init        _C_PROTOTYPE((pthread_cond_t *__cond,
                                                  pthread_condattr_t *__attr));
extern int pthread_condattr_init    _C_PROTOTYPE((pthread_condattr_t *__attr));
extern int pthread_condattr_destroy _C_PROTOTYPE((pthread_condattr_t *__attr));
extern int pthread_cond_wait        _C_PROTOTYPE((pthread_cond_t *__cond,
                                                  pthread_mutex_t *__mutex));
extern int pthread_cond_timedwait   _C_PROTOTYPE((pthread_cond_t *__cond,
                                                  pthread_mutex_t *__mutex,
                                                  struct timespec *__timeout));
extern int pthread_cond_signal      _C_PROTOTYPE((pthread_cond_t *__cond));
extern int pthread_cond_broadcast   _C_PROTOTYPE((pthread_cond_t *__cond));
extern int pthread_condattr_getpshared
                                    _C_PROTOTYPE((pthread_condattr_t *__attr,
                                                  int *__pshared));
extern int pthread_condattr_setpshared
                                    _C_PROTOTYPE((pthread_condattr_t *__attr,
                                                  int __pshared));

#ifdef __cplusplus
} /* extern "C" */
#endif

/*
 * Threads.
 */

typedef any_t (*pthread_func_t) _C_PROTOTYPE((any_t __arg));
typedef void (*pthread_sighandler_t) _C_PROTOTYPE((int));

struct sched_param {
  int sched_priority;
};

typedef struct {
  int flags;
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
  int stacksize;
#endif
#ifdef _POSIX_THREAD_ATTR_CUSTOM
  char *stack;
  char *name;
#endif
  
  int contentionscope;
  int inheritsched;
  int detachstate;
  int sched;
  struct sched_param param;
  struct timespec starttime, deadline, period;
} pthread_attr_t;

typedef struct {
  int state;
  pthread_queue_t queue;
  pthread_cond_t *cond;
} pthread_suspend_t;  

typedef struct timer_ent {
        struct timeval tp;                     /* wake-up time                */
        struct pthread *thread;                /* thread                      */
        int mode;                              /* mode of timer (ABS/REL/RR)  */
        TAILQ_ENTRY(timer_ent) timerlist;
        TAILQ_ENTRY(timer_ent) threadlist;
} *timer_ent_t;

/*
 * Queue indices
 */
#define PRIMARY_QUEUE 0
/* this slot may be used by the TIMER_QUEUE with index 2 */
#define NUM_QUEUES    3

#define K_QUEUES_TIMEOUT 2
#define K_QUEUES_ALL     1
#define K_QUEUES_PRIMARY 0

typedef int pthread_key_t;

typedef struct pthread_cleanup *pthread_cleanup_t;
typedef TAILQ_HEAD(pthread_timer_q, timer_ent) *pthread_timer_q_t;

typedef struct pthread {
        threadctx_t context __attribute__((aligned(8)));                /* save area for context switch   */
        threadctx_t body __attribute__((aligned(8)));                   /* save area for pthread_body     */
        volatile int ret;                  /* return value (EINTR --> -1)    */
        char *stack_base;                  /* bottom of run-time stack       */
        int state;                         /* thread state, -> pthread_asm.h */

        TAILQ_ENTRY(pthread) pt_qelem[NUM_QUEUES];
        struct pthread_timer_q pt_active_timers;
        struct pthread_timer_q pt_free_timers;
        
        char *pt_name;
  
        int num_timers;                    /* number of timers for thread    */
        struct timeval interval;           /* time left for SCHED_RR         */
        struct p_siginfo sig_info[NNSIG];  /* info for user handlers         */
        struct _reent pt_reent;        /* reentrant structure for newlib */
        struct _reent *pt_reentp;        /* pointer to eather pt_reent or global reent */
  
        int sig;                           /* latest received signal         */
        int code;                          /* latest received signal code    */
        int osp, opc, obp;                 /* save area for old context sw   */
        struct context_t *nscp;            /* UNIX signal context (new sig)  */
        struct context_t *scp;             /* UNIX signal context (current)  */
        struct pthread_queue joinq;        /* queue to await termination     */
        pthread_cond_t *cond;              /* cond var. if in cond_wait      */
        pthread_queue_t queue;/* primary queue thread is contained in        */
        sigset_t mask;        /* set of signals thread has masked out        */
        sigset_t pending;     /* set of signals pending on thread            */
        sigset_t sigwaitset;  /* set of signals thread is waiting for        */
        pthread_func_t func;  /* actual function to call upon activation     */
        any_t arg;            /* argument list to above function             */
        any_t result;         /* return value of above function              */
        any_t key[_POSIX_DATAKEYS_MAX];    /* thread specific data           */
        pthread_cleanup_t cleanup_top;     /* stack of cleanup handlers      */
        pthread_attr_t attr;               /* attributes                     */
        int base_prio;                     /* Base priority of thread        */
        int max_ceiling_prio; /* Max of ceiling prio among locked mutexes    */
        int new_prio;         /* New Priority                                */
	pthread_suspend_t suspend;         /* save area for thread suspend   */
#if defined(IO_NP) && !defined(__FreeBSD__) && !defined(__dos__)
#if !defined(__linux__) && !defined(_M_UNIX)
        volatile struct aio_result_t resultp;/*result of asynchronous I/O ops*/
#endif /* !__linux__ && !_M_UNIX */
#ifdef USE_POLL_NP
        int wait_on_select;                /* more information in fds        */
        int nfds;                          /* pertinent file desc. set width */
        int how_many;                      /* how many amoung  0 .. width -1 */
        struct pollfd* fds;                /* poll events */
#else /* !USE_POLL_NP */
        int wait_on_select;                /* more information in fds        */
        int width;                         /* pertinent file desc. set width */
        int how_many;                      /* how many amoung  0 .. width -1 */
        fd_set readfds;                    /* read file descriptor set       */
        fd_set writefds;                   /* write file descriptor set      */
        fd_set exceptfds;                  /* except. file descriptor set    */
#endif /* !USE_POLL_NP */
#endif /* IO_NP && !__FreeBSD__ && !__dos__ */
#ifdef STAND_ALONE_NP
        pthread_func_t timer_func;        /* function to be called on timeout*/
        any_t timer_arg;                  /* arg of above function           */
        struct timespec tp;               /* wake-up time                    */
        int dummy;                        /* filler for stack alignment      */
#else /* !STAND_ALONE_NP */
        struct timeval tp;                /* wake-up time                    */
#endif /* !STAND_ALONE_NP */
} *pthread_t __attribute__((aligned(8)));

#ifdef __FreeBSD__
#define IO_SIZE_T size_t
#elif defined (__dos__)
#define IO_SIZE_T size_t
#define SIGSUSPEND_CONST const
#define SIGACTION_CONST const
#define SIGPROCMASK_CONST const
#define SIGWAIT_CONST const
#define LONGJMP_CONST
#define SIGLONGJMP_CONST
#elif defined (__linux__)
#define IO_SIZE_T size_t
#elif defined (_M_UNIX)
#define IO_SIZE_T size_t
#else
#if defined(__GNUC__) && defined(_PARAMS)
#define IO_SIZE_T __SIZE_TYPE__
/*
 * for gcc 2.5.8 or before use:
 * #define IO_SIZE_T unsigned int
 */
#define SIGSUSPEND_CONST
#define SIGACTION_CONST const
#define SIGPROCMASK_CONST const
#define SIGWAIT_CONST const
#define LONGJMP_CONST
#define SIGLONGJMP_CONST
#else
/*
 * for older gcc, remove "const" for "SIGPROCMASK_CONST"
 */
#define IO_SIZE_T size_t
#define SIGSUSPEND_CONST const
#define SIGACTION_CONST const
#define SIGPROCMASK_CONST const
#define SIGWAIT_CONST const
#define LONGJMP_CONST
#define SIGLONGJMP_CONST
#endif
#endif


/******************************/
/*      Thread Functions      */
/* Thread Attribute Functions */
/******************************/

#ifdef __cplusplus
extern "C" {
#endif
 
extern pthread_t pthread_self           _C_PROTOTYPE ((void));
extern void pthread_init                _C_PROTOTYPE ((void));
extern void pthread_init_attr           _C_PROTOTYPE ((pthread_attr_t *__attr));
extern int pthread_create               _C_PROTOTYPE((pthread_t *thread,
                                                      pthread_attr_t *__attr,
                                                      pthread_func_t __func,
                                                      any_t __arg));
extern int pthread_equal                _C_PROTOTYPE((pthread_t __t1,
                                                      pthread_t __t2));
extern int pthread_detach               _C_PROTOTYPE((pthread_t thread));
extern int pthread_join                 _C_PROTOTYPE((pthread_t thread,
                                                      any_t *__status));
extern int sched_yield                  _C_PROTOTYPE((void));
extern void pthread_exit                _C_PROTOTYPE((any_t __status));
extern int pthread_attr_init            _C_PROTOTYPE((pthread_attr_t *__attr));
extern int pthread_attr_destroy         _C_PROTOTYPE((pthread_attr_t *__attr));
extern int pthread_getschedparam        _C_PROTOTYPE((pthread_t thread,
						      int *__policy,
                                                      struct sched_param *__param));
extern int pthread_setschedparam        _C_PROTOTYPE((pthread_t thread,
						      int __policy,
                                                      struct sched_param *__param));
extern int pthread_attr_setstacksize    _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      size_t __stacksize));
extern int pthread_attr_getstacksize    _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      size_t *__stacksize));
extern int pthread_attr_setscope        _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      int __contentionscope));
extern int pthread_attr_setinheritsched _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      int __inheritsched));
extern int pthread_attr_setschedpolicy  _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      int __policy));
extern int pthread_attr_setschedparam   _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      struct sched_param *__param));
extern int pthread_attr_getscope        _C_PROTOTYPE((pthread_attr_t *__attr,
						      int *__contentionscope));
extern int pthread_attr_getinheritsched _C_PROTOTYPE((pthread_attr_t *__attr,
						      int *__inheritsched));
extern int pthread_attr_getschedpolicy  _C_PROTOTYPE((pthread_attr_t *__attr,
						      int *__policy));
extern int pthread_attr_getschedparam   _C_PROTOTYPE((pthread_attr_t *__attr,
						      struct sched_param *__param));

extern int pthread_attr_getstarttime_np _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      struct timespec *__tp));
extern int pthread_attr_setstarttime_np _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      struct timespec *__tp));
extern int pthread_attr_getdeadline_np  _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      struct timespec *__tp));
extern int pthread_attr_setdeadline_np  _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      struct timespec *__tp,
                                                      pthread_func_t func));
extern int pthread_attr_getperiod_np    _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      struct timespec *__tp));
extern int pthread_attr_setperiod_np    _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      struct timespec *__tp,
                                                      pthread_func_t func));

extern int pthread_key_create           _C_PROTOTYPE((pthread_key_t *__key,
                                                      void (*__func)
                                                      (any_t __value)));
extern int pthread_key_delete           _C_PROTOTYPE((pthread_key_t __key));
extern int pthread_setspecific          _C_PROTOTYPE((pthread_key_t __key,
                                                      any_t __value));
extern any_t pthread_getspecific        _C_PROTOTYPE((pthread_key_t __key));
extern void pthread_cleanup_push        _C_PROTOTYPE((void (*__func)
                                                      (any_t __value),
                                                      any_t __arg));
extern void pthread_cleanup_pop         _C_PROTOTYPE((int __execute));
extern int sched_get_priority_max       _C_PROTOTYPE((int __policy));
extern int sched_get_priority_min       _C_PROTOTYPE((int __policy));
extern int pthread_attr_setdetachstate  _C_PROTOTYPE((pthread_attr_t *__attr,
                                                      int __detachstate));
extern int pthread_attr_getdetachstate  _C_PROTOTYPE((pthread_attr_t *__attr,
						      int *__detachstate));
extern int pthread_once                 _C_PROTOTYPE((pthread_once_t *__once_c,
                                                      void (*__func) (void)));

/*
 * implementation-defined extensions
 */
extern void pthread_setsigcontext_np    _C_PROTOTYPE((struct context_t *__scp,
                                                      jmp_buf __env,
                                                      int __val));
extern int pthread_lock_stack_np        _C_PROTOTYPE((pthread_t __p));

extern int pthread_suspend_np           _C_PROTOTYPE((pthread_t __t));
extern int pthread_resume_np            _C_PROTOTYPE((pthread_t __t));

extern void pthread_mutex_dbgstart_np   _C_PROTOTYPE((pthread_mutex_t *__mutex));
extern void pthread_mutex_dbgstop_np   _C_PROTOTYPE((pthread_mutex_t *__mutex));
extern void pthread_cond_dbgstart_np    _C_PROTOTYPE((pthread_cond_t *__mutex));
extern void pthread_cond_dbgstop_np    _C_PROTOTYPE((pthread_cond_t *__mutex));
  
/******************************/
/*      Signal Functions      */
/******************************/

extern int sigwait             _C_PROTOTYPE((SIGWAIT_CONST sigset_t *__set,
					     int *__sig));
extern int sigtimedwait        _C_PROTOTYPE((SIGWAIT_CONST sigset_t *__set,
					     siginfo_t              *info,
					     const struct timespec  *timeout));
extern int sigprocmask         _C_PROTOTYPE((int __how,
                                             SIGPROCMASK_CONST sigset_t *__set,
                                             sigset_t *__oset));
extern int pthread_sigmask     _C_PROTOTYPE((int __how,
                                             SIGPROCMASK_CONST sigset_t *__set,
                                             sigset_t *__oset));
extern int sigpending          _C_PROTOTYPE((sigset_t *__set));
extern int sigsuspend          _C_PROTOTYPE((SIGSUSPEND_CONST sigset_t *__set));
extern int pause               _C_PROTOTYPE((void));
#ifdef __dos__
/* we use a macro to rename these so we can still call the system
   functions, since we don't have system-call hooks under DJGPP. */
#define raise(sig)                   pthread_dummy_raise(sig)
#define sigprocmask(sig, set1, set2) pthread_dummy_sigprocmask(sig, set1, set2)
#define sigsuspend(set)              pthread_dummy_sigsuspend(set)
#endif
#if defined(M_UNIX)
#undef raise
#endif
extern int raise               _C_PROTOTYPE((int __sig));
extern int pthread_kill        _C_PROTOTYPE((pthread_t thread,
                                             int __sig));
extern int sigqueue            _C_PROTOTYPE((pid_t id,
                                             int __sig,
					     const union p_sigval value));
extern int pthread_cancel      _C_PROTOTYPE((pthread_t thread));
extern int pthread_setcancelstate _C_PROTOTYPE((int __state, int *__oldstate));
extern int pthread_setcanceltype  _C_PROTOTYPE((int __type, int *__oldtype));
extern void pthread_testcancel _C_PROTOTYPE((void));
extern int sigaction           _C_PROTOTYPE((int __sig,
                                             SIGACTION_CONST struct sigaction *__act,
                                             struct sigaction *__oact));
#if defined(__FreeBSD__) || defined(_M_UNIX) || defined(__linux__) || defined(__dos__)
extern pthread_sighandler_t signal
			        _C_PROTOTYPE((int __sig,
                                             pthread_sighandler_t handler));
#else
#if defined(SOLARIS_NP) && defined(__cplusplus)
extern void (*signal(int, void (*)(int)))(int);
#else
extern void (*signal())();
#endif
#endif

/* yet to come...
extern unsigned int alarm      _C_PROTOTYPE((unsigned int __seconds));
*/
extern int nanosleep           _C_PROTOTYPE((const struct timespec *__rqtp,
                                             struct timespec *__rmtp));
extern unsigned int sleep      _C_PROTOTYPE((unsigned int __seconds));
extern int clock_gettime       _C_PROTOTYPE((int __clock_id,
                                             struct timespec *__tp));
extern int clock_settime       _C_PROTOTYPE((int __clock_id,
                                             struct timespec *__tp));
extern int clock_getres       _C_PROTOTYPE((int __clock_id,
                                             struct timespec *__tp));

/******************************/
/*    Low-Level Functions     */
/******************************/

#ifndef __dos__
#ifdef setjmp
#undef setjmp
#endif
#ifdef longjmp
#undef longjmp
#endif
#ifdef sigsetjmp
#undef sigsetjmp
#endif
#ifdef siglongjmp
#undef siglongjmp
#endif
#endif

extern int setjmp         _C_PROTOTYPE((jmp_buf __env));
extern void longjmp       _C_PROTOTYPE((LONGJMP_CONST jmp_buf __env,
                                        int __val));
extern int sigsetjmp      _C_PROTOTYPE((sigjmp_buf __env,
                                        int __savemask));
extern void siglongjmp    _C_PROTOTYPE((SIGLONGJMP_CONST sigjmp_buf __env,
                                        int __val));

/******************************/
/*       leon lowlevel        */
/******************************/
extern void pthreadleon_init_ticks                _C_PROTOTYPE ((void));
  
/******************************/
/*       I/O Functions        */
/******************************/

extern int read  _C_PROTOTYPE((int __fd,
                               void *__buf,
                               IO_SIZE_T __nbytes));
extern int write _C_PROTOTYPE((int __fd,
                               const void *__buf,
                               IO_SIZE_T __nbytes));

#ifdef __cplusplus
} /* extern "C" */
#endif

#define PTHREAD_CANCELED (void *)-1

#endif /* !_pthread_pthread_h */
