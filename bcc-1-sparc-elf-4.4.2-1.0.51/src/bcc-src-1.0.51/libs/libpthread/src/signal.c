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

  @(#)signal.c	3.14 11/8/00

*/

/* 
 * Functions for the handling of signals and timers.
 */

/*
 * The DEBUG flag causes a message to be printed out during signal handling.
 * The IO flag handles I/O requests asynchronously such that a signal is
 * delivered to the process upon completion of the operation.
 * If both flags are set at the same time, the signal handler would issue
 * an I/O request for each invocation which in turns causes another signal
 * to be delivered to yet another instance of the signal handler.
 * To avoid this, messages are only printed if DEBUG is defined but not IO.
 */

#define PTHREAD_KERNEL
#include "signal_internals.h"
#include "internals.h"
#include "setjmp.h"
#include "offsets.h"
#ifdef TDI_SUPPORT
#include "tdi-aux.h"
#endif

#if !defined (_M_UNIX) && !defined (__dos__)
//#include <sys/syscall.h>
#endif

#include <fsu_pthread/malloc.h>
#include <fsu_pthread/debug.h>

#include <asm-leon/kernel.h>
#include <asm-leon/sigcontext.h>

#ifdef NOERR_CHECK
#undef NOERR_CHECK
#include "mutex.h"
#define NOERR_CHECK
#else /* !NOERR_CHECK */
#include "mutex.h"
#endif /* NOERR_CHECK */

#ifndef FC_CODE
#define FC_CODE(c) (c & 0xff)
#endif
#ifndef  __FDS_BITS
#define __FDS_BITS(set) ((set)->fds_bits)
#endif

#ifdef STAND_ALONE
extern char heap_start;
#endif /* STAND_ALONE */

#ifdef RAND_SWITCH 
extern int pthread_n_ready;
#endif

#if defined(STACK_CHECK) && defined(SIGNAL_STACK)
extern int pthread_page_size;
extern KERNEL_STACK pthread_tempstack;
#endif

#ifdef STAND_ALONE
//pthread_timer_q_t pthread_timer;              /* timer queue                 */
#else
pthread_timer_q pthread_timer;                /* timer queue                 */
static struct itimerval it;                   /* timer structure             */
#endif

struct sigaction pthread_user_handler[NNSIG]; /* user signal handlers        */
volatile int new_code[NNSIG];                 /* UNIX signal code (new sigs) */
pthread_cond_t *new_cond[NNSIG];              /* cond for user handlers      */
struct context_t *new_scp[NNSIG];             /* info for user handlers      */
static int pending_code[NNSIG];               /* UNIX signal code (pending)  */
static sigset_t synchronous;                  /* set of synchronous signals  */
static sigset_t sig_handling;                 /* set of signals being handled*/


/*------------------------------------------------------------*/
/*
 * default_action - take default action on process
 * Notice: SIGSTOP and SIGKILL can never be received (unmaskable)
 * but they are included anyway.
 */
static void default_action(sig)
     int sig;
{
  PTRACEIN;
  switch (sig) {
  case SIGURG:
  case SIGIO:
  case SIGCONT:
  case SIGCHLD:
  case SIGWINCH:
    break; /* ignore or continue */

  case SIGSTOP:
  case SIGTSTP:
  case SIGTTIN:
  case SIGTTOU:
  PDEBUG(PDBG_SIGNAL,"default action exit");
    exit(0);
    break;
  default:
    PDEBUG(PDBG_SIGNAL,"default action exit");
    exit(0);
  }
}

/*------------------------------------------------------------*/
/*
 * handle_thread_signal - try to handle one signal on a thread and
 * return TRUE if it was handled, otherwise return FALSE
 */
static int handle_thread_signal(p, sig, code)
pthread_t p;
int sig;
int code;
{
  register struct context_t *scp;
#ifdef C_CONTEXT_SWITCH
  extern void pthread_fake_call_wrapper_wrapper();
#endif /* C_CONTEXT_SWITCH */
  
  PTRACEIN;
  PDEBUG(PDBG_SIGNAL,"thread:0x%x sig:%d code:0x%x",p, sig, code);

  /*
   * handle timer signals
   */
  if (sig == TIMER_SIG) {
  }
  
  /*
   * handle signals for sigwait
   */
  if (p->state & T_SIGWAIT && sigismember(&p->sigwaitset, sig)) {
    pthread_q_wakeup_thread(NO_QUEUE, p, NO_QUEUE_INDEX);
    p->state &= ~T_SIGWAIT;
    pthread_sigaddset2set(&p->mask, &p->sigwaitset);
    pthread_sigdelset2set(&p->mask, &cantmask);
    sigdelset(&p->sigwaitset, sig);
  }
  
  /*
   * handler set to ignore
   */
  if (pthread_user_handler[sig].sa_handler == SIG_IGN && sig != SIGCANCEL) {
    PDEBUG(PDBG_SIGNAL,"ignore %d ",sig);
    return(TRUE);
  }
  
  /*
   * handler set to default action
   */
  if (pthread_user_handler[sig].sa_handler == SIG_DFL && sig != SIGCANCEL) {
    PDEBUG(PDBG_SIGNAL,"default_action[%d] ",sig);
    default_action(sig);
    return(TRUE);
  }

  /*
   * handle signals for sigsuspend and user handlers
   */
  if (sigismember(&handlerset, sig)) {
    if (p->state & T_BLOCKED) {
      if (p->pt_reentp) {
	p->pt_reentp->_errno = EINTR;
      }
    }
      
    if (!(p->state & T_RUNNING)) {
      if (p->state & T_SYNCTIMER) {
	PDEBUG(PDBG_TIMEOUT,": timeout on sigtimedwait\n");
	pthread_cancel_timed_sigwait(p, TRUE, SYNC_TIME, TRUE);
      } else {
        pthread_q_wakeup_thread(p->queue, p, PRIMARY_QUEUE);
        if (p->state & (T_SIGWAIT | T_SIGSUSPEND)) {
          p->state &= ~(T_SIGWAIT | T_SIGSUSPEND);
          sigemptyset(&p->sigwaitset);
        }
      }
    }
      
    p->sig_info[sig].si_signo = sig;
#if (defined(STACK_CHECK) && defined(SIGNAL_STACK)) 
    if ((sig == SIGSEGV || sig == SIGBUS) && FC_CODE(code) != code) {
      p->sig_info[sig].si_value.sigval_ptr = (caddr_t) code;
      p->sig_info[sig].si_code = FC_PROT;
    }
    else
#endif /* STACK_CHECK && SIGNAL_STACK */
      p->sig_info[sig].si_code = code;


    PDEBUG(PDBG_SIGNAL,"signal %d",sig);

#ifdef TRASH
    /*
     * return avoids a loop in thread signal processing, no other solution
     */
    if (p->context[THREAD_JB_PC] == (int) pthread_fake_call_wrapper_wrapper)
     {
       /*
	* We must mark the signal as pending!!!
        */
       return TRUE;
     }
#endif
    if (pthread_not_called_from_sighandler(p->context[THREAD_JB_PC])){
      PDEBUG(PDBG_SIGNAL,"DIRECTED_AT_THREAD");
      p->nscp = (struct context_t *) DIRECTED_AT_THREAD;
    }
    
    p->sig = sig;
    p->osp = p->context[THREAD_JB_SP];
    p->opc = p->context[THREAD_JB_PC];
    PDEBUG(PDBG_SIGNAL,"old context sp: 0x%x pc: 0x%x (%x,%x)",p->osp, p->opc,&p->osp, &p->opc);
    p->context[THREAD_JB_PC] = (int) pthread_fake_call_wrapper_wrapper;
    p->opc += RETURN_OFFSET;
    p->context[THREAD_JB_PC] -= RETURN_OFFSET;

    
    PDEBUG(PDBG_SIGNAL,"initialized",p, sig, code);
    return(TRUE);
  }

  /*
   * handle cancel signal
   */
  if (sig == SIGCANCEL) {
    if (p->state & T_SYNCTIMER)
      pthread_cancel_timed_sigwait(p, FALSE, ALL_TIME, TRUE);
    else if (p->state & (T_SIGWAIT | T_SIGSUSPEND)) {
      p->state &= ~(T_SIGWAIT | T_SIGSUSPEND);
      sigemptyset(&p->sigwaitset);
    }

    if (p->queue && !(p->state & T_RUNNING))
      pthread_q_deq(p->queue, p, PRIMARY_QUEUE);
    
    TAILQ_REMOVE(&all,p,pt_qelem[K_QUEUES_ALL]);
    
    /*
     * no more signals for this thread, not even cancellation signal
     */
    pthread_sigcpyset2set(&p->mask, &all_signals);
    sigaddset(&p->mask, SIGCANCEL);
    p->nscp = (struct context_t *) DIRECTED_AT_THREAD;
    p->sig = (int) PTHREAD_CANCELED;
    p->context[THREAD_JB_PC] = (int) pthread_fake_call_wrapper_wrapper;
    p->context[THREAD_JB_PC] -= RETURN_OFFSET;
    if (!(p->state & T_RUNNING))
      pthread_q_wakeup_thread(NO_QUEUE, p, NO_QUEUE_INDEX);
    
    return(TRUE);
  }

  return (FALSE);
}

static int aio_handle();

/*------------------------------------------------------------*/
/*
 * handle_one_signal - handle one signal on the process level
 * assumes SET_KERNEL_FLAG
 */
static void handle_one_signal(sig, code)
int sig;
int code;
{
  register pthread_t p = mac_pthread_self();
  struct itimerval it;
  struct timespec now;
  extern pthread_t pthread_q_all_find_receiver();
  
  PTRACEIN;
  PDEBUG(PDBG_SIGNAL,"sig:%d code:0x%x",sig, code);
  

  /*
   * Determine who needs to get the signal (in the following order):
   * (1) signal directed at specific thread: take this thread
   * (2) signal at process level:
   * (2a) synchronous signal: direct at current thread
   * (2b) SIGALRM, timer queue not empty, timer expired: take head off timer q
   * (2c) SIGIO, asynchronous I/O requested: determine receiver and make ready
   * (2c) handler defined: take first thread in all queue with signal unmasked
   * (3) no handler defined: pend signal on process till thread unmasks signal
   *      if signal already pending, it's lost
   */
  if (p != NO_PTHREAD &&
      (p->nscp == DIRECTED_AT_THREAD ||
       pthread_not_called_from_sighandler(p->context[THREAD_JB_PC])) &&
      (p = (pthread_t) code))
    code = SI_USER;
  else if (p != NO_PTHREAD && sigismember(&synchronous, sig))
    /* p = p */;
  else if (sig == SIGALRM) {
    /* eiselekd todo: fix later */ 
    /*if ((p = pthread_timer) && 
        !clock_gettime(CLOCK_REALTIME, &now) && GTEQ_NTIME(now, p->tp))
      pthread_cancel_timed_sigwait(p, TRUE, ANY_TIME, p->queue != &ready);
    */
    return;
  }
  /* SIGIO is being used to invoke pthread_select_isr which takes care of 
   * suspending and waking up threads waiting on I/O.
   * The parameter 2 is used instead of UART_ISR
   */
     
  else if (sig == SIGIO) {
    //pthread_select_isr(SIGIO);
    return;
  }
  else if (!(p = pthread_q_all_find_receiver(&all, sig))) {
    if (!sigismember(&pending_signals, sig)) {
      sigaddset(&pending_signals, sig);
      pending_code[sig] = code;
    }
    return;
  }
  
  if (p->state & T_RETURNED)
    return;
  
  /*
   * Pend signal on thread if it's masked out OR
   * if the signal is SIGCANCEL, the interrupt state CONTROLLED, and
   * we are not at an interruption point.
   */
  if (sigismember(&p->mask, sig) ||
      sig == SIGCANCEL &&
      p->state & T_CONTROLLED && !(p->state & T_INTR_POINT)) {
    sigaddset(&p->pending, sig);
    p->sig_info[sig].si_code = code;
    return;
  }

  if (handle_thread_signal(p, sig, code))
    return;
  
  default_action(sig);
}
  
/*------------------------------------------------------------*/
/*
 * pthread_handle_many_process_signals - determine pending signal(s).
 * if no thread ready, suspend process;
 * returns the head of the ready queue.
 * assumes SET_KERNEL_FLAG
 */
pthread_t pthread_handle_many_process_signals()
{
  register int sig;
  PTRACEIN;

  do {
    while (pthread_signonemptyset(&new_signals)) {
      /*
       * start critical section
       */
      SIGPROCMASK(SIG_BLOCK, &all_signals, (struct sigset_t *) NULL);
      
      pthread_sigcpyset2set(&sig_handling, &new_signals);
      pthread_sigdelset2set(&new_signals, &sig_handling);
      
      SIGPROCMASK(SIG_UNBLOCK, &all_signals, (struct sigset_t *) NULL);
      /*
       * end of critical section
       */

      for (sig = 1; sig < NNSIG; sig++)
        if (sigismember(&sig_handling, sig))
          handle_one_signal(sig, new_code[sig]);
    }

    /*
     * No thread, no action: suspend waiting for signal at process level
     */
    if (TAILQ_EMPTY(&ready)) {
      SIGPROCMASK(SIG_BLOCK, &all_signals, (struct sigset_t *) NULL);
      if (!pthread_signonemptyset(&new_signals)) {
        while(!pthread_signonemptyset(&new_signals))
          SIGSUSPEND(&no_signals);
      }
      SIGPROCMASK(SIG_UNBLOCK, &all_signals, (struct sigset_t *) NULL);
    }

  } while (TAILQ_EMPTY(&ready));

  return(TAILQ_FIRST(&ready));
}

/*------------------------------------------------------------*/
/*
 * pthread_handle_one_process_signal - handle latest signal caught by 
 * universal handler while not in kernel
 * returns the head of the ready queue.
 * assumes SET_KERNEL_FLAG
 */
void pthread_handle_one_process_signal(sig, code)
int sig;
int code;
{
  PTRACEIN;
  
  handle_one_signal(sig, code);

  if (pthread_signonemptyset(&new_signals) ||
      TAILQ_EMPTY(&ready))
    pthread_handle_many_process_signals();
}

/*------------------------------------------------------------*/
/*
 * pthread_handle_pending_signals - handle unmasked pending signals of 
 * current thread assumes SET_KERNEL_FLAG
 */
void pthread_handle_pending_signals()
{
  pthread_t p = mac_pthread_self();
  int sig;
  PTRACEIN;

  /*
   * handle signals pending on threads if they are unmasked and
   * SIGCANCEL only on an interruption point.
   */
  if (!pthread_siggeset2set(&p->mask, &p->pending))
    for (sig = 1; sig < NNSIG; sig++)
      if (sigismember(&p->pending, sig) && !sigismember(&p->mask, sig) && 
          (sig != SIGCANCEL || p->state & T_INTR_POINT)) {
        sigdelset(&p->pending, sig);
        
        handle_thread_signal(p, sig, p->sig_info[sig].si_code);
      }

  /*
   * handle signals pending on process
   */
  if (!pthread_siggeset2set(&p->mask, &pending_signals))
    for (sig = 1; sig < NNSIG; sig++)
      if (sigismember(&pending_signals, sig) && !sigismember(&p->mask, sig)) {
        sigdelset(&pending_signals, sig);
        
        handle_thread_signal(p, sig, pending_code[sig]);
      }
}


/*------------------------------------------------------------*/
/*
 * pthread_init_signals - initialize signal package
 */
void pthread_init_signals()
{
  int sig;
  struct sigaction vec;
#if defined(STACK_CHECK) && defined(SIGNAL_STACK)
  SIGSTACK_T ss;

  ss.ss_sp = (char *) SA((int) pthread_tempstack_top - STACK_OFFSET);
  CLR_SS_ONSTACK;
#ifndef STAND_ALONE
  if (SIGSTACK(&ss, (SIGSTACK_T *) NULL))
#ifdef DEBUG
    dbgleon_printf(
            "Pthreads: Could not specify signal stack, errno %d\n", errno)
#endif /* DEBUG */
    ;
#endif /* !STAND_ALONE */
#endif
  PTRACEIN;

  /*
   * initialize kernel structure
   */
  is_in_kernel = is_updating_timer = FALSE;

  sigemptyset(&synchronous);
  sigaddset(&synchronous, SIGILL);
  sigaddset(&synchronous, SIGABRT);
#if !defined (__linux__) && !defined(__dos__)
  sigaddset(&synchronous, SIGEMT);
#endif /* !__linux__ && !__dos__ */
  sigaddset(&synchronous, SIGFPE);
  sigaddset(&synchronous, SIGBUS);
  sigaddset(&synchronous, SIGSEGV);
  
  sigemptyset((sigset_t *) &new_signals);
  sigemptyset(&pending_signals);
  sigemptyset(&handlerset);
  sigemptyset(&sig_handling);

  sigemptyset(&cantmask);
  sigaddset(&cantmask, SIGKILL);
  sigaddset(&cantmask, SIGSTOP);
  sigaddset(&cantmask, SIGCANCEL);

  sigfillset(&all_signals);
  sigdelset(&all_signals, SIGKILL);
  sigdelset(&all_signals, SIGSTOP);

  pthread_queue_init(&ready);
  pthread_queue_init(&all);
  pthread_queue_init(&suspend_q);
  pthread_queue_init(&pthread_timeout_q);

  (&ready)->tqh_name = "ready-queue";
  (&all)->tqh_name = "all-queue";
  (&suspend_q)->tqh_name = "suspend-queue";
  (&pthread_timeout_q)->tqh_name = "timeout-queue";
  
  pthread_queue_init(&dbglist_cond);
  pthread_queue_init(&dbglist_mutex);

  set_warning = "CAUTION: entering kernel again\n";
  clear_warning = "CAUTION: leaving kernel again\n";
  prio_warning = "CAUTION: prio violation when leaving kernel\n";
#ifdef RAND_SWITCH
  srandom(1);
  pthread_n_ready = 0;
#endif

#ifdef STAND_ALONE
  sigemptyset(&proc_mask);
  cur_heap = (int)lreg_sp; //(&heap_start);
#else /* !STAND_ALONE */
  /*
   * no signal requests
   */
  for (sig = 0; sig < NNSIG; sig++) {
    pthread_user_handler[sig].sa_handler = SIG_DFL;
    sigemptyset(&pthread_user_handler[sig].sa_mask);
    pthread_user_handler[sig].sa_flags = 0;
    new_code[sig] = 0;
    new_cond[sig] = NULL;
    new_scp[sig] = NULL;
  }

  /*
   * install universal signal handler for all signals
   * except for those which cannot be masked
   */
  vec.sa_handler = sighandler;
  pthread_sigcpyset2set(&vec.sa_mask, &all_signals);
#if (defined(_M_UNIX) && !defined(SCO5)) || defined(__dos__)
  vec.sa_flags = SA_SIGINFO;
#else
  vec.sa_flags = SA_SIGINFO | SA_RESTART;
#endif
#ifdef __linux__
  vec.sa_restorer = (void (*)(void)) NULL;
#endif
    
  for (sig = 1; sig < NSIG; sig++)
    if (sig != SIGPROF && !sigismember(&cantmask, sig)) {
#if defined(STACK_CHECK) && defined(SIGNAL_STACK)
      if (sig == SIGBUS || sig == SIGILL || sig == SIGSEGV)
        vec.sa_flags |= SA_ONSTACK;
#endif
      if (SIGACTION(sig, &vec, (struct sigaction *) NULL))
#ifdef DEBUG
        dbgleon_printf( "Pthreads (signal): \
          Could not install handler for signal %d\n", sig)
#endif
          ;
#if defined(STACK_CHECK) && defined(SIGNAL_STACK)
      if (sig == SIGBUS || sig == SIGILL || sig == SIGSEGV)
        vec.sa_flags &= ~SA_ONSTACK;
#endif
    }
#endif /* STAND_ALONE */

}


#ifdef GNAT
int segv = FALSE;
#endif /* GNAT */

/*------------------------------------------------------------*/
/*
 * sigaction - install interrupt handler for a thread on a signal
 * return 0 if o.k., -1 otherwise
 * Notice: cannot mask SIGKILL, SIGSTOP, SIGCANCEL
 */
int sigaction(sig, act, oact)
int sig;
SIGACTION_CONST struct sigaction *act;
struct sigaction *oact;
{
  register pthread_t p = mac_pthread_self();
  struct sigaction vec;
  PTRACEIN;

  if (sig == SIGPROF)
    return(SIGACTION(sig, act, oact));

  if ((sigismember(&cantmask, sig) && act) || sig < 1 || sig >= NSIG) {
    set_errno(EINVAL);
    return(-1);
  }

  if (!act) {
    if (oact)
      *oact = pthread_user_handler[sig];
    return(0);
  }

  if (pthread_sigismemberset2set(&act->sa_mask, &cantmask)) {
    set_errno(EINVAL);
    return(-1);
  }

  SET_KERNEL_FLAG;
  if (oact)
    *oact = pthread_user_handler[sig];

  PDEBUG(PDBG_SIGNAL,"install action[%d]=0x%x",sig,act);
  
  pthread_user_handler[sig] = *act;

  /*
   * queue up mac_pthread_self() in the signal queue indicated
   */
  if (!sigismember(&handlerset, sig))
    sigaddset(&handlerset, sig);

  /*
   * dequeue pending signals on process and threads if to be ignored
   * or perform default action on process if default action chosen
   */
  if (act->sa_handler == SIG_IGN || act->sa_handler == SIG_DFL) {
    if (sigismember(&pending_signals, sig)) {
      sigdelset(&pending_signals, sig);
      if (act->sa_handler == SIG_DFL)
        default_action(sig);
    }

    TAILQ_FOREACH(p, &all, pt_qelem[K_QUEUES_ALL]) {
      if (sigismember(&p->pending, sig)) {
        sigdelset(&p->pending, sig);
        if (act->sa_handler == SIG_DFL)
          default_action(sig);
      }
    }
  }

#ifdef __dos__
  signal (sig, act->sa_handler);
#else
  SIM_SYSCALL(TRUE);
#endif
  CLEAR_KERNEL_FLAG;

  return(0);
}

#ifdef _M_UNIX			/* Added by monty for SCO 3.2V4.2 */
pthread_sighandler_t sigset(sig,handler)
     int sig;
     pthread_sighandler_t handler;
{
    return signal(sig, handler);
}
#endif

/* #ifndef __dos__ */
/* /\*------------------------------------------------------------*\/ */
/* /\* */
/*  * signal - install signal handler */
/*  *\/ */
/* pthread_sighandler_t signal(sig, handler) */
/*      int sig; */
/*      pthread_sighandler_t handler; */
/* { */
/*   struct sigaction act; */

/*   act.sa_handler = handler; */
/* #if defined(SOLARIS) || defined(__dos__) || defined(__USE_POSIX) */
/*   sigemptyset(&act.sa_mask); */
/* #else /\* !SOLARIS || !__dos__ *\/ */
/*   act.sa_mask = 0; */
/* #endif /\* !SOLARIS *\/ */
/* #if defined(__linux__) || defined(__FreeBSD__) || defined(_M_UNIX) */
/*   act.sa_flags = SA_ONESHOT | SA_NOMASK; */
/* #endif */
/* #ifdef __dos__ */
/*   act.sa_flags = 0; */
/* #endif */
/* #ifdef __linux__  */
/*   act.sa_restorer = NULL; */
/* #endif /\* __linux__ || __FreeBSD__ || _M_UNIX *\/ */
/*   if (!sigaction(sig, &act, (struct sigaction *) NULL)) */
/*     return(handler); */
/*   else */
/*     return((pthread_sighandler_t)-get_errno()); */
/* } */
/* #endif */


#if !defined(__linux__) && !defined(__dos__)

#ifdef TIMEVAL_TO_TIMESPEC
#define env_use environment[0]._jb
#define env_decl environment
#else /* !TIMEVAL_TO_TIMESPEC */
#define env_use  environment
#define env_decl environment
#endif /* !TIMEVAL_TO_TIMESPEC */

/*------------------------------------------------------------*/
/*
 * pthread_setsigcontext_np - modify the signal context to return to a setjmp()
 * call location, i.e. simulate a longjmp() but leave the signal handler
 * properly.
 */
void pthread_setsigcontext_np(scp, env_decl, val)
     struct context_t *scp;
     jmp_buf env_decl;
     int val;
{
  scp->sc_pc = env_use[THREAD_JB_PC];
#if !defined (__FreeBSD__) && !defined (_M_UNIX) && !defined(__linux__)
  scp->sc_npc = env_use[THREAD_JB_PC] + 4;
#endif
#if defined(ASM_SETJMP) || !defined(C_CONTEXT_SWITCH)
  scp->sc_pc += RETURN_OFFSET;
#if !defined (__FreeBSD__) && !defined (_M_UNIX) && !defined(__linux__)
  scp->sc_npc += RETURN_OFFSET;
#endif
#endif /* defined(ASM_SETJMP) || !defined(C_CONTEXT_SWITCH) */
  scp->sc_sp = env_use[THREAD_JB_SP];
  if (env_use[THREAD_JB_SVMASK])
    pthread_sigcpyset2set(&scp->sc_mask, &env_use[THREAD_JB_MASK]); /* copy sigmasks */
#if !defined (__FreeBSD__) && !defined (_M_UNIX) && !defined(__linux__)
  scp->sc_o0 = val; 
#endif
}
#endif /* !__linux__ */

/*------------------------------------------------------------*/
/*
 * SIGPROCMASK - change or examine signal mask of process
 */
int SIGPROCMASK(how, set, oset)
int how;
const sigset_t *set;
sigset_t *oset;
{
#ifdef SVR4
  return(syscall(SYS_sigprocmask, how, set, oset));
#else /* !SVR4 */
  sigset_t old;
  PTRACEIN;

#ifndef STAND_ALONE
#if defined (__FreeBSD__) || defined (_M_UNIX) || defined(__linux__) || defined (__dos__)
  SYS_SIGPROCMASK (how, NULL, &proc_mask);
#else
  proc_mask = sigblock(0);
#endif
#endif /* !STAND_ALONE */

  if (oset)
    pthread_sigcpyset2set(oset, &proc_mask);

  if (!set)
    return(0);

  switch (how) {
  case SIG_BLOCK:
    pthread_sigaddset2set(&proc_mask, set);
    break;
  case SIG_UNBLOCK:
    pthread_sigdelset2set(&proc_mask, set);
    break;
  case SIG_SETMASK:
    pthread_sigcpyset2set(&proc_mask, set);
    break;
  default:
    set_errno(EINVAL);
    return(-1);
  }

  pthread_sigdelset2set(&proc_mask, &cantmask);
#ifndef STAND_ALONE
#if defined (__FreeBSD__) || defined (_M_UNIX) || defined(__linux__) || defined (__dos__)
  SYS_SIGPROCMASK (SIG_SETMASK, &proc_mask, NULL);
#else
  sigsetmask(proc_mask);
#endif
#endif /* !STAND_ALONE */
  
  return(0);
#endif /* !SVR4 */
}

/*------------------------------------------------------------*/
/*
 * SIGACTION - change or examine signal handlers of process
 */
int SIGACTION(sig, act, oact)
int sig;
const struct sigaction *act;
struct sigaction *oact;
{
#ifdef SVR4
#if defined(SOLARIS) && defined(IO)
  if (sig == SIGIO)
    sigaction(sig, act, oact);
  else
#endif /* SOLARIS && IO */
    return(syscall(SYS_sigaction, sig, act, oact));
#else /* !SVR4 */
#ifndef STAND_ALONE
#if defined(__FreeBSD__) || defined (_M_UNIX) || defined(__linux__) || defined (__dos__)
#ifdef TRASH
#ifdef __USE_POSIX
  return(__sigaction(sig, act, oact));
#else
  return(SYS_SIGACTION (sig, act, oact));
#endif /* __USE_POSIX */
#else
  return(SYS_SIGACTION (sig, act, oact));
#endif
#else
  return(sigvec(sig, act, oact)); /* cheating: structs distinct / same layout */
#endif
#else /* !STAND_ALONE */
  set_errno(EINVAL);
  return(-1);
#endif /* !STAND_ALONE */
#endif /* !SVR4 */
}

/*------------------------------------------------------------*/
/*
 * SIGSUSPEND - suspend process waiting for signals
 */
int SIGSUSPEND(set)
sigset_t *set;
{
#ifdef SVR4
  return(syscall(SYS_sigsuspend, set));
#else /* !SVR4 */
#ifndef STAND_ALONE
#if defined (__FreeBSD__) || defined (_M_UNIX) || defined(__linux__) || defined (__dos__)
  return (SYS_SIGSUSPEND (set));
#else
  return(sigpause(*set));
#endif
#else /* !STAND_ALONE */
  /* busy wait */;
#endif /* !STAND_ALONE */
#endif /* !SVR4 */
}


