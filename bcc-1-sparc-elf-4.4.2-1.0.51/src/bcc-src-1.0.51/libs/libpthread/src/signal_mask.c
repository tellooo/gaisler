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
 * sigprocmask - change or examine signal mask of thread
 * return 0 or -1 if error
 * cannot mask SIGKILL, SIGSTOP, SIGCANCEL
 */
int sigprocmask(how, set, oset)
int how;
SIGPROCMASK_CONST sigset_t *set;
sigset_t *oset;
{
  sigset_t new, old, pending;
  register pthread_t p = mac_pthread_self();
  register already_in_kernel = is_in_kernel;
  PTRACEIN;

  if (!already_in_kernel)
    SET_KERNEL_FLAG;

  pthread_sigcpyset2set(&old, &p->mask);
  if (oset)
    pthread_sigcpyset2set(oset, &old);
  if (!set) {
    if (!already_in_kernel)
      CLEAR_KERNEL_FLAG;
    return(0);
  }

  pthread_sigcpyset2set(&new, set);
  pthread_sigdelset2set(&new, &cantmask);

  switch (how) {
  case SIG_BLOCK:
    pthread_sigaddset2set(&p->mask, &new);
    break;
  case SIG_UNBLOCK:
    pthread_sigdelset2set(&p->mask, &new);
    break;
  case SIG_SETMASK:
    pthread_sigcpyset2set(&p->mask, &new);
    break;
  default:
    set_errno(EINVAL);
    if (!already_in_kernel)
      CLEAR_KERNEL_FLAG;
    return(-1);
  }

  pthread_sigcpyset2set(&pending, &p->pending);
  pthread_sigaddset2set(&pending, &pending_signals);
  pthread_sigdelset2set(&old, &p->mask);
  if (pthread_sigismemberset2set(&pending, &old))
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  else {
    SIM_SYSCALL(TRUE);
    if (!already_in_kernel)
      CLEAR_KERNEL_FLAG;
  }
  return(0);
}

/*------------------------------------------------------------*/
/*
 * pthread_sigmask - change or examine signal mask of thread
 * return 0 or error value if error
 * cannot mask SIGKILL, SIGSTOP, SIGCANCEL
 */
int pthread_sigmask(how, set, oset)
int how;
SIGPROCMASK_CONST sigset_t *set;
sigset_t *oset;
{
  int ret;
  ret = sigprocmask(how, set, oset);
  if (ret != 0)
    ret = get_errno();
  return ret;
}

/*------------------------------------------------------------*/
/*
 * sigpending - inquire about pending signals which are blocked, i.e. applies
 * to only those signals which are explicitly pending on the current thread
 * return 0 if o.k., -1 otherwise
 */
int sigpending(set)
sigset_t *set;
{
  PTRACEIN;
  
  if (!set) {
    set_errno(EINVAL);
    return(-1);
  }

  SIM_SYSCALL(TRUE);
  pthread_sigcpyset2set(set, &mac_pthread_self()->pending);
  pthread_sigaddset2set(set, &pending_signals);
  pthread_sigdelset2set(set, &cantmask);
  return(0);
}
