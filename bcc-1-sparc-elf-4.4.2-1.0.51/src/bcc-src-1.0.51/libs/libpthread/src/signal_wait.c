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
 * sigwait - suspend thread until signal becomes pending
 * Return: signal number if o.k., otherwise -1
 * Notice: cannot mask SIGKILL, SIGSTOP, SIGCANCEL
 */
int sigwait(set, sigp)
     int *sigp;
     SIGWAIT_CONST sigset_t *set;
{
  int status;
  PTRACEIN;

  status = sigtimedwait( set, NULL, NULL );

  if ( status != -1 ) {
    if ( sigp )
      *sigp = status;
    return 0;
  }

  return errno;
  
}

int sigwaitinfo(
  const sigset_t         *set,
  siginfo_t              *info
  ) 
{
  PTRACEIN;
  return sigtimedwait( set, info, NULL );
}

/*------------------------------------------------------------*/
/*
 * sigtimedwait - suspend thread until signal becomes pending or break after timeout
 * Return: signal number if o.k., otherwise -1
 * Notice: cannot mask SIGKILL, SIGSTOP, SIGCANCEL
 */

int sigtimedwait(
  const sigset_t         *set,
  siginfo_t              *info,
  const struct timespec  *timeout
)
{
  register int sig;
  sigset_t new, more;
  register pthread_t p = mac_pthread_self();
  PTRACEIN;

  pthread_sigcpyset2set(&new, set);
  pthread_sigdelset2set(&new, &cantmask);
  pthread_sigcpyset2set(&more, &new);
  SET_KERNEL_FLAG;

  /*
   * Are the signals in set blocked by the current thread?
   */
  if (!timeout) {
    if (!pthread_siggeset2set(&p->mask, &more)) {
      PDEBUG(PDBG_SIGNAL,"# blocked (%x)", p->mask);
      set_errno(EINVAL);
      CLEAR_KERNEL_FLAG;
      return(-1);
    }
  }

  /*
   * Any of the signals in set pending on thread?
   */
  if (pthread_sigismemberset2set(&p->pending, &more))
    for (sig = 1; sig < NNSIG; sig++)
      if (sigismember(&p->pending, sig) && sigismember(&more, sig)) {
	PDEBUG(PDBG_SIGNAL,"# thread signal pending");
        sigdelset(&p->pending, sig);
        CLEAR_KERNEL_FLAG;
	if (info) {
	  info->si_signo = sig;
	  info->si_code = SI_USER;
	  info->si_value.sival_int = 0;
	}
        return(sig);
      }
    
  /*
   * Any of the signals in set pending on process?
   */
  if (pthread_sigismemberset2set(&pending_signals, &more))
    for (sig = 1; sig < NNSIG; sig++)
      if (sigismember(&pending_signals, sig) && sigismember(&more, sig)) {
	PDEBUG(PDBG_SIGNAL,"# proc signal pending ");
        sigdelset(&pending_signals, sig);
        CLEAR_KERNEL_FLAG;
	if (info) {
	  info->si_signo = sig;
	  info->si_code = SI_USER;
	  info->si_value.sival_int = 0;
	}
	return(sig);
      }
    
  /*
   * suspend thread and wait for any of the signals
   */
  pthread_sigaddset2set(&p->sigwaitset, &new);
  pthread_sigdelset2set(&p->mask, &new);
  p->state &= ~T_RUNNING;
  p->state |= T_SIGWAIT | T_BLOCKED | T_INTR_POINT;
  if (sigismember(&p->pending, SIGCANCEL) && !sigismember(&p->mask, SIGCANCEL))
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  else {
    if (timeout) {
      if (pthread_timed_sigwait(p, timeout, REL_TIME, 0, 0) != 0) {
	CLEAR_KERNEL_FLAG;
	return -1;
      }
      p->state |= T_SYNCTIMER;
    } 
    pthread_q_deq_head(&ready, PRIMARY_QUEUE);
    SIM_SYSCALL(TRUE);
    CLEAR_KERNEL_FLAG;
  }

  PDEBUG(PDBG_SIGNAL,"return sig:%d",p->sig);
  
  if (get_errno() == EINTR || get_errno() == EAGAIN)
    return(-1);

/*   /\* */
/*    * determine the received signal */
/*    *\/ */
/*   for (sig = 1; sig < NNSIG; sig++) */
/*     if (sigismember(&new, sig) && !sigismember(&p->sigwaitset, sig)) */
/*       break; */

  /*
   * Clear signal mask
   */
  SET_KERNEL_FLAG;
  sigemptyset(&p->sigwaitset);
  CLEAR_KERNEL_FLAG;

  /*
   * If no signal woke us up directly, a user handler (hence interrupt) must
   * have been activated via a different signal.
   */
  if (p->sig < 0) {
    set_errno(EINVAL);
    return(-1);
  }

  if (info) {
    *info = p->sig_info[p->sig];
  }
  return(p->sig);
}
