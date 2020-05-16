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

#include "signal_internals.h"
#include "internals.h"
#include "setjmp.h"
#include "offsets.h"
#include <fsu_pthread/malloc.h>
#include <fsu_pthread/debug.h>
#include <asm-leon/kernel.h>
#include <asm-leon/sigcontext.h>

/*------------------------------------------------------------*/
/*
 * raise - send a signal to the current process;
 * NOT directed to any particular thread,
 * any thread waiting for the signal may pick it up.
 */
#ifdef _M_UNIX
#ifdef raise
#undef raise
#endif
#endif

int raise(sig)
int sig;
{
  PTRACEIN;
  SET_KERNEL_FLAG;
  
  if (!SAVE_CONTEXT(mac_pthread_self())) {
    pthread_signal_sched(sig, (int) NO_PTHREAD);
  }

  CLEAR_KERNEL_FLAG;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_kill - send signal to thread
 */
int pthread_kill(thread, sig)
pthread_t thread;
int sig;
{
  PTRACEIN;
  if (thread == NO_PTHREAD || thread->state & T_RETURNED ||
      sigismember(&cantmask, sig) || !sigismember(&all_signals, sig) || sig >= NNSIG)
    return(EINVAL);

  /*
   * queue up signal associated with thread
   */
  SET_KERNEL_FLAG;
  SIM_SYSCALL(TRUE);
  if (!SAVE_CONTEXT(mac_pthread_self())) {
    pthread_signal_sched(sig, (int) thread);
  }
  
  CLEAR_KERNEL_FLAG;

  return(0);
}

int pthread_kill_value(thread, sig, value)
pthread_t thread;
int sig;
const union p_sigval *value;
{
  PTRACEIN;
  if (!thread || sig >= NNSIG) {
    return(EINVAL);
  }
  if (value) {
    thread->sig_info[sig].si_value = *value;
  }
  return pthread_kill(thread, sig);  
}

int sigqueue(pid_t id, int sig, const union p_sigval value) {
  pthread_t t;
  PTRACEIN;
  if (id != getpid()){ return (EINVAL); }
  TAILQ_FOREACH(t, &all, pt_qelem[K_QUEUES_ALL]) {
    if ((t != k_idle) && ( sigismember(&t->sigwaitset, sig) || !sigismember(&t->mask, sig))) {
      pthread_kill_value(t, sig, &value);
    }
  }
  return 0;
};

int kill(pid_t id, int  sig){
  pthread_t t;
  PTRACEIN;
  if (id != getpid()){ return (EINVAL); }
  TAILQ_FOREACH(t, &all, pt_qelem[K_QUEUES_ALL]) {
    if ((t != k_idle) && ( sigismember(&t->sigwaitset, sig) || !sigismember(&t->mask, sig))) {
      pthread_kill_value(t, sig, 0);
    }
  }
  return 0;
}


/*------------------------------------------------------------*/
/*
 * pthread_cancel - cancel thread
 * Open question: is this an interruption point if a thread cancels itself?
 * As of now, it isn't!
 */
int pthread_cancel(thread)
pthread_t thread;
{
  PTRACEIN;
  if (thread == NO_PTHREAD || thread->state & T_RETURNED)
    return(EINVAL);

  /*
   * queue up signal associated with thread
   */
  SET_KERNEL_FLAG;
  if (!SAVE_CONTEXT(mac_pthread_self())) {
    pthread_signal_sched(SIGCANCEL, (int) thread);
  }
  
  CLEAR_KERNEL_FLAG;
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_setcancelstate - set interruptablility state for thread cancellation
 */
int pthread_setcancelstate(state, oldstate)
     int state, *oldstate;
{
  int old;
  sigset_t new;
  PTRACEIN;

  if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE)
    return(EINVAL);
  
  SIM_SYSCALL(TRUE);
  old = (sigismember(&mac_pthread_self()->mask, SIGCANCEL) ?
         PTHREAD_CANCEL_DISABLE : PTHREAD_CANCEL_ENABLE);
  sigemptyset(&new);
  sigaddset(&new, SIGCANCEL);
  if (sigprocmask(state, &new, (sigset_t *) NULL) == 0) {
    if (oldstate)
      *oldstate = old;
    return(0);
  }
  else
    return(get_errno());
}

/*------------------------------------------------------------*/
/*
 * pthread_setcanceltype - set interruptablility type for thread cancellation
 */
int pthread_setcanceltype(type, oldtype)
     int type, *oldtype;
{
  register pthread_t p = mac_pthread_self();
  int old;
  PTRACEIN;
  
  old = (p->state & T_CONTROLLED ?
         PTHREAD_CANCEL_DEFERRED : PTHREAD_CANCEL_ASYNCHRONOUS);
  switch (type) {
  case PTHREAD_CANCEL_DEFERRED:
    SET_KERNEL_FLAG;
    p->state |= T_CONTROLLED;
    SIM_SYSCALL(TRUE);
    CLEAR_KERNEL_FLAG;
    if (oldtype)
      *oldtype = old;
    return(0);
  case PTHREAD_CANCEL_ASYNCHRONOUS:
    SET_KERNEL_FLAG;
    p->state &= ~T_CONTROLLED;
    if (sigismember(&p->pending, SIGCANCEL) &&
        !sigismember(&p->mask, SIGCANCEL)) {
      p->state |= T_INTR_POINT;
      SIG_CLEAR_KERNEL_FLAG(TRUE);
    }
    else {
      SIM_SYSCALL(TRUE);
      CLEAR_KERNEL_FLAG;
    }
    if (oldtype)
      *oldtype = old;
    return(0);
  default:
    return(EINVAL);
  }
}

/*------------------------------------------------------------*/
/*
 * pthread_testcancel - act upon pending cancellation (creates interruption point)
 */
void pthread_testcancel()
{
  register pthread_t p = mac_pthread_self();

  SET_KERNEL_FLAG;
  if (sigismember(&p->pending, SIGCANCEL) &&
      !sigismember(&p->mask, SIGCANCEL)) {
    p->state |= T_INTR_POINT;
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  }
  else {
    SIM_SYSCALL(TRUE);
    CLEAR_KERNEL_FLAG;
  }
}
