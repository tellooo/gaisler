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

#include "internals.h"
#include "setjmp.h"
#include "offsets.h"
#include <fsu_pthread/malloc.h>
#include <fsu_pthread/debug.h>
#include <asm-leon/kernel.h>
#include <asm-leon/sigcontext.h>
#include <asm-leon/clock.h>

/*------------------------------------------------------------*/
/*
 * pthread_absnanosleep - suspend until abs. time interval specified by "rqtp"
 */
int pthread_absnanosleep(rqtp)
const struct timespec *rqtp;
{
  register pthread_t p = mac_pthread_self();
  struct timespec rmtp;
  struct timespec now;
  PTRACEIN;

  rmtp.ts_sec = rqtp->ts_sec;
  rmtp.ts_nsec = rqtp->ts_nsec;

  do {
    SET_KERNEL_FLAG;
    
    if (pthread_timed_sigwait(p, &rmtp, ABS_TIME, NULL, NULL) == -1) {
      CLEAR_KERNEL_FLAG;
      return(-1);
    }

    /*
     * clear error number before suspending
     */
    set_errno(0);

    p->state &= ~T_RUNNING;
    p->state |= T_BLOCKED | T_SYNCTIMER | T_INTR_POINT;
    if (sigismember(&p->pending, SIGCANCEL) &&
        !sigismember(&p->mask, SIGCANCEL))
      SIG_CLEAR_KERNEL_FLAG(TRUE);
    else {
      pthread_q_deq_head(&ready, PRIMARY_QUEUE);
      SIM_SYSCALL(TRUE);
      CLEAR_KERNEL_FLAG;
    }

    PDEBUG(PDBG_TIMEOUT,"sleep returned");

    /* Check if condition was signaled or time-out was exceeded. */
    if (get_errno() == EINTR) {

      PDEBUG(PDBG_TIMEOUT,"sleep interrupted");
      clock_gettime(CLOCK_REALTIME, &now);
      if (GT_NTIME(p->tp, now)) 
	MINUS_NTIME(rmtp, p->tp, now); 
      
    }
    else
      NTIMERCLEAR(rmtp);
  } while (!LE0_NTIME(rmtp));

  return(0);
}

/*------------------------------------------------------------*/
/*
 * nanosleep - suspend until time interval specified by "rqtp" has passed
 * or a user handler is invoked or the thread is canceled
 */
int nanosleep(rqtp, rmtp)
const struct timespec *rqtp;
struct timespec *rmtp;
{
  register pthread_t p = mac_pthread_self();
  struct timespec now;
  PTRACEIN;

  if (!rqtp || rqtp->ts_nsec >= NSEC_PER_SEC ) {
    set_errno(EINVAL);
    return(-1);
  }
  
  if (rmtp)
    rmtp->ts_sec = rmtp->ts_nsec = 0;

  if (rqtp->ts_sec < 0 || (rqtp->ts_sec == 0 && rqtp->ts_nsec == 0)) {
    return 0;
  }

  SET_KERNEL_FLAG;

  if (pthread_timed_sigwait(p, rqtp, REL_TIME, NULL, NULL) != 0) {
    CLEAR_KERNEL_FLAG;
    return(-1);
  }

  /* clear error number before suspending */
  set_errno(0);

  p->state &= ~T_RUNNING;
  p->state |= T_BLOCKED | T_SYNCTIMER | T_INTR_POINT;
  if (sigismember(&p->pending, SIGCANCEL) &&
      !sigismember(&p->mask, SIGCANCEL))
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  else {
    pthread_q_deq_head(&ready, PRIMARY_QUEUE);
    SIM_SYSCALL(TRUE);
    CLEAR_KERNEL_FLAG;
  }
  
  clock_gettime(CLOCK_REALTIME, &now);

  PDEBUG(PDBG_TIMEOUT,"nanosleep returned [now:%ds,%dns] [togo:%ds,%dns]",now.ts_sec,now.ts_nsec,p->tp.ts_sec,p->tp.ts_nsec);
  PDEBUG(PDBG_TIMEOUT,"errno %d",get_errno());
  if (get_errno() == EAGAIN) {
    if (GT_NTIME(p->tp, now)) {
      if (rmtp) {
	MINUS_NTIME(*rmtp, p->tp, now);
	PDEBUG(PDBG_TIMEOUT,"time left [%ds,%dns]",rmtp->ts_sec,rmtp->ts_nsec);
      }
    }
    return -1;
  }
  return(0);
}

/*------------------------------------------------------------*/
/*
 * sleep - suspend thread for time interval (in seconds)
 */
unsigned int sleep(seconds)
unsigned int seconds;
{
  struct timespec rqtp, rmtp;
  PTRACEIN;

  if (rqtp.ts_sec = seconds) {
    rqtp.ts_nsec = 0;
    nanosleep(&rqtp, &rmtp);
    if (get_errno() == EAGAIN) 
      return(rmtp.ts_sec + (rmtp.ts_nsec ? 1 : 0)); /* pessimistic round-up */
  }
  return(0);
}


/*------------------------------------------------------------*/
/*
 * alarm - Deliver SIGALARM after "seconds"
 */
unsigned int alarm(seconds)
     unsigned int seconds;
{
  PTRACEIN;

  set_errno(ENOSYS);
  return(-1);
}


