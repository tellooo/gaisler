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
 * sigsuspend - suspend thread,
 * set replaces the masked signals for the thread temporarily,
 * suspends thread, and resumes execution when a user handler is invoked
 * Return: -1 and EINTR if interrupted or EINVAL if wrong parameters
 * Notice: cannot mask SIGKILL, SIGSTOP
 */
int sigsuspend(set)
SIGSUSPEND_CONST sigset_t *set;
{
  register int sig;
  sigset_t old, pending;
  register pthread_t p = mac_pthread_self();
  PTRACEIN;

  if (!set) {
    set_errno(EINVAL);
    return(-1);
  }

  SET_KERNEL_FLAG;
  pthread_sigcpyset2set(&old, &p->mask);
  pthread_sigcpyset2set(&p->mask, set);
  pthread_sigdelset2set(&p->mask, &cantmask);
  pthread_sigaddset2set(&p->sigwaitset, &p->mask);

  p->state &= ~T_RUNNING;
  p->state |= T_SIGSUSPEND | T_BLOCKED | T_INTR_POINT;
  if (sigismember(&p->pending, SIGCANCEL) && !sigismember(&p->mask, SIGCANCEL))
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  else {
    pthread_q_deq_head(&ready, PRIMARY_QUEUE);
    SIM_SYSCALL(TRUE);
    CLEAR_KERNEL_FLAG;
  }

  /*
   * restore the initial signal mask
   */
  SET_KERNEL_FLAG;
  pthread_sigcpyset2set(&p->mask, &old);

  pthread_sigcpyset2set(&pending, &p->pending);
  pthread_sigaddset2set(&pending, &pending_signals);
  pthread_sigcpyset2set(&old, set);
  pthread_sigdelset2set(&old, &cantmask);
  pthread_sigdelset2set(&old, &p->mask);
  if (pthread_sigismemberset2set(&pending, &old))
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  else
    CLEAR_KERNEL_FLAG;

  return(-1);
}

/*------------------------------------------------------------*/
/*
 * pause - suspend thread until any signal is caught,
 * same as sigsuspend except that the signal mask doesn't change
 */
int pause()
{
  register int sig;
  register pthread_t p = mac_pthread_self();
  PTRACEIN;

  SET_KERNEL_FLAG;
  pthread_sigcpyset2set(&p->sigwaitset, &p->mask);
  
  p->state &= ~T_RUNNING;
  p->state |= T_SIGSUSPEND | T_BLOCKED | T_INTR_POINT;
  if (sigismember(&p->pending, SIGCANCEL) && !sigismember(&p->mask, SIGCANCEL))
    SIG_CLEAR_KERNEL_FLAG(TRUE);
  else {
    pthread_q_deq_head(&ready, PRIMARY_QUEUE);
    SIM_SYSCALL(TRUE);
    CLEAR_KERNEL_FLAG;
  }
  
  return(-1);
}

//#if 0
#ifdef STAND_ALONE

/*------------------------------------------------------------*/
/*
 * pthread_timed_sigwait - suspend running thread until specified time
 * Return -1 if error, 0 otherwise
 * assumes SET_KERNEL_FLAG
 */
int pthread_timed_sigwait(p, timeout, mode, func, arg)
     pthread_t p;
     struct timespec *timeout;
     int mode;
     pthread_func_t func;
     any_t arg;
{
  struct timespec now;
  PTRACEIN;
    
  if (!timeout || timeout->ts_sec < 0 || 
      timeout->ts_nsec < 0 || timeout->ts_nsec >= 1000000000)
    return(EINVAL);
  
  clock_gettime(CLOCK_REALTIME, &now);
  
  if (mode == ABS_TIME) {
    if (GTEQ_NTIME(now, *timeout)) {
      set_errno(EAGAIN);
      return(-1);
    }
  }
  else if (mode == REL_TIME) {
    if (LE0_NTIME(*timeout/*set_errno*/)) {
      set_errno(EAGAIN);
      return(-1);
    }
    PLUS_NTIME(*timeout, now, *timeout);
  }
  else {
    set_errno(EINVAL);
    return(-1);
  }
  PDEBUG(PDBG_TIMEOUT,"Timeout for [%ds,%dns] now:[%ds,%dns]",timeout->ts_sec,timeout->ts_nsec,now.ts_sec,now.ts_nsec);
  
  /* queue up current thread on the timer queue */
  p->timer_func = func;
  p->timer_arg = arg;
  p->tp.ts_sec = timeout->ts_sec;
  p->tp.ts_nsec = timeout->ts_nsec;
  return pthread_q_timeout_enq(&pthread_timeout_q, p);
}


/*------------------------------------------------------------*/
/*
 * pthread_cancel_timed_sigwait - dequeue thread waiting on timer only
 * "signaled" indicates if the thread "p" received a SIGALRM
 * Notice: set error in both thread structure and global UNIX errno
 *         since this may be called from pthread_handle_many_process_signals
 * assumes SET_KERNEL_FLAG
 */
int pthread_cancel_timed_sigwait(first_p, signaled, mode, activate)
     pthread_t first_p;
     int signaled, mode, activate;
{
  pthread_t p = first_p;
  struct timespec now;
  PTRACEIN;
  
  if (TAILQ_EMPTY(&pthread_timeout_q)) {
    is_updating_timer = FALSE;
    return(0);
  }

  clock_gettime(CLOCK_REALTIME, &now);
  
  do {
#ifdef TRASH
    if (!p->timer_func) {
#endif
      if (p->state & T_CONDTIMER) {

	PDEBUG_HEADER(PDBG_TIMEOUT);
	PDEBUG_PTNAME(PDBG_TIMEOUT,p);
	PDEBUG_SIMPLECODE(PDBG_TIMEOUT,": cond timeout [now: %ds,%dns]\n",now.ts_sec,now.ts_nsec);
	
        p->state &= ~(T_CONDTIMER | T_SYNCTIMER);
        pthread_q_deq(p->queue, p, PRIMARY_QUEUE);
        pthread_q_timeout_wakeup_thread(&pthread_timeout_q, p,
#ifndef DEF_RR
                                      (p == first_p ? activate : TRUE));
#else
                                      ((p == first_p && (tmr->mode & mode)) ? activate : TRUE));
#endif
      } else {

	PDEBUG_HEADER(PDBG_TIMEOUT);
	PDEBUG_PTNAME(PDBG_TIMEOUT,p);
	PDEBUG_SIMPLECODE(PDBG_TIMEOUT," synctimer timeout [now: %ds,%dns]\n",now.ts_sec,now.ts_nsec);

	p->state &= ~T_SYNCTIMER;
        pthread_q_timeout_wakeup_thread(&pthread_timeout_q, p,
#ifndef DEF_RR
                                      (p == first_p ? activate : TRUE));
#else
                                      ((p == first_p && (tmr->mode & mode)) ? activate : TRUE));
#endif
      }
      if (p != first_p || signaled) {
	PDEBUG(PDBG_TIMEOUT,"register interrupted (EAGAIN(%d) for reent:0x%x)",EAGAIN,p->pt_reentp);
	if (p->pt_reentp) {
	  p->pt_reentp->_errno = EAGAIN;
	}
      }
#ifdef TRASH
    }
    else
      (*p->timer_func)(p->timer_arg);
#endif

     p = TAILQ_FIRST(&pthread_timeout_q);

     clock_gettime(CLOCK_REALTIME, &now);

  } while (p && GTEQ_NTIME(now, p->tp));
  
  is_updating_timer = FALSE;

  /*
   * timer signal received while cancelling => ignore it
   */
  if (!signaled && (sigismember(&new_signals, SIGALRM)))
    sigdelset(&new_signals, SIGALRM);
  
  return(0);
}

#else /* !STAND_ALONE */

/*------------------------------------------------------------*/
/*
 * pthread_timed_sigwait - suspend running thread until specified time
 * Return -1 if error, 0 otherwise
 * assumes SET_KERNEL_FLAG
 */
int pthread_timed_sigwait(p, timeout, mode, func, arg)
     pthread_t p;
     struct timespec *timeout;
     int mode;
     pthread_func_t func;
     any_t arg;
{
  struct itimerval it;
  struct timeval now, in;
  register timer_ent_t phead;
  PTRACEIN;

  if (p->num_timers >= TIMER_MAX) {
    set_errno(EAGAIN);
    return(-1);
  }

  /* convert posix time (timespec-nano-nsec) to unix time (timeval-micro-usec) */
  if (mode != RR_TIME) {
    if (!timeout || timeout->ts_nsec < 0 || timeout->ts_nsec >= 1000000000) {
      set_errno(EINVAL);
      return(-1);
    }
    else
      P2U_TIME(in, (*timeout));
  }
  
  if (gettimeofday(&now, (struct timezone *) NULL)) {
    set_errno(EINVAL);
    return(-1);
  }
  
  it.it_interval.tv_sec = it.it_interval.tv_usec = 0;

  if (mode == ABS_TIME) {
    if (GTEQ_TIME(now, in)) {
      /* time has already passed */
      set_errno(EAGAIN);
      return(-1);
    }
    MINUS_TIME(it.it_value, in, now);
  } else if (mode == REL_TIME) {
    if (LE0_TIME(in)) {
      /* time has already passed */
      set_errno(EAGAIN);
      return(-1);
    }
    it.it_value.tv_sec = in.tv_sec;
    it.it_value.tv_usec = in.tv_usec;
    PLUS_TIME(in, in, now);
  }
#ifdef DEF_RR
  else if (mode == RR_TIME) {
    p->state |= T_ASYNCTIMER;
    if ((p->interval.tv_sec == 0) && (p->interval.tv_usec == 0)) {
      in.tv_sec = it.it_value.tv_sec = 0;
      in.tv_usec = it.it_value.tv_usec = TIME_SLICE;
    }
    else {
      in.tv_sec = it.it_value.tv_sec = p->interval.tv_sec; 
      in.tv_usec = it.it_value.tv_usec = p->interval.tv_usec;
    }
    PLUS_TIME(in, in, now);
  }
#endif
  else {
    set_errno(EINVAL);
    return(-1);
  }

  /*
   * if no timer set, set timer to current request; otherwise
   * overwrite timer if current request needs to be served next
   */
  if (!(phead = TAILQ_FIRST(&pthread_timer)) || GT_TIME(phead->tp, in)) {
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &it, (struct itimerval *) NULL)) {
#ifdef DEBUG
      dbgleon_printf( "ERROR: setitimer in timed_sigwait\n");
#endif
      set_errno(EINVAL);
      return(-1);
    }
    PDEBUG(PDBG_TIMER,"setitimer %d.%d sec.usec\n",
            it.it_value.tv_sec, it.it_value.tv_usec);
  }
  else {
    PDEBUG(PDBG_ERROR|PDBG_TIMER,"timer not set up, pthread_timer.head=%x\n", phead);
  }
    
  /*
   * queue up current thread on the timer queue
   */
  return pthread_q_timeout_enq(&pthread_timer,
                      in,
                      func == (pthread_func_t) pthread_exit ? DL_TIME : mode,
                      p);
}

/*------------------------------------------------------------*/
/*
 * pthread_cancel_timed_sigwait - dequeue thread waiting for alarm only
 * "signaled" indicates if the thread "p" received a SIGALRM
 * Notice: set error in both thread structure and global UNIX errno
 *         since this may be called from pthread_handle_many_process_signals
 * assumes SET_KERNEL_FLAG
 */
int pthread_cancel_timed_sigwait(first_p, signaled, mode, activate)
     pthread_t first_p;
     int signaled, mode, activate;
{
  pthread_t p = first_p;
  timer_ent_t tmr;
  timer_ent_t old_tmr_head = TAILQ_FIRST(&pthread_timer);
  struct itimerval it;
  struct timeval now;
  int time_read = FALSE;
#if defined(IO) && defined(_M_UNIX)
  int recalculate = 0;
  static int aio_calculate();
#endif
  PTRACEIN;

  /*
   * find the first instance of this thread in timer queue
   */
#ifdef DEF_RR
  for (tmr = pthread_timer.head; tmr; tmr = tmr->next[TIMER_QUEUE])
    if (tmr->thread == p && (tmr->mode & mode))
      break;
#else
  tmr = p;
#endif

  if (!tmr) {
#ifdef TIMER_DEBUG
    dbgleon_printf( "pthread_cancel_timed_sigwait: exit0\n");
#endif
    return(0);
  }

  /*
   * for each occurrence, remove the timer entry
   */
  do {
#ifdef DEF_RR
    if (tmr->mode == RR_TIME) {
      p->state &= ~T_ASYNCTIMER;
      pthread_q_timer_deq(&pthread_timer, tmr);
    }
#ifdef REAL_TIME
    else if (tmr->mode == DL_TIME)
      pthread_q_timer_deq(&pthread_timer, tmr);
#endif /* REAL_TIME */
    else
#endif /* DEF_RR */
    if (p->state & T_CONDTIMER) {
      p->state &= ~(T_CONDTIMER | T_SYNCTIMER);
      pthread_q_deq(p->queue, p, PRIMARY_QUEUE);
#ifndef DEF_RR
      pthread_q_timed_wakeup_thread(&pthread_timer, p, 
                                      (p == first_p ? activate : TRUE));
#else
      pthread_q_timer_deq(&pthread_timer, tmr);
      pthread_q_timed_wakeup_thread(NULL, p, 
                      ((p == first_p && (tmr->mode & mode)) ? activate : TRUE));
#endif
      if (p != first_p || signaled) {
        p->terrno = EAGAIN;
#ifdef __dos__
        p->terrno = 0; /* To pass c95040b and c9a008a */
#endif

        if (p == mac_pthread_self())
          set_errno(EAGAIN);
      }
    }  
    else if (p->state & T_SYNCTIMER) {
      p->state &= ~T_SYNCTIMER;
#ifndef DEF_RR
      pthread_q_timed_wakeup_thread(&pthread_timer, p,
                     ((p == first_p) ? activate : TRUE));
#else
      pthread_q_timer_deq(&pthread_timer, tmr);
      pthread_q_timed_wakeup_thread(NULL, p,
                     ((p == first_p && (tmr->mode & mode)) ? activate : TRUE));
#endif
#if defined(IO) && defined(_M_UNIX)
      if (sigismember(&p->sigwaitset, AIO_SIG)) {
       p->wait_on_select = 0;
       p->how_many = 0;
       p->terrno = 0;
       if (p==mac_pthread_self())
        set_errno(0); 
       recalculate++;
       sigdelset(&p->sigwaitset, AIO_SIG);
      }
#endif
     }
#if defined(TIMER_DEBUG)
    else {
#if defined(DEF_RR)
       dbgleon_printf( "pthread_cancel_timed_sigwait: thread=%d invalid timer=%d return(0)\n", p, tmr->mode);
#else
       dbgleon_printf( "pthread_cancel_timed_sigwait: thread=%d invalid timer=%d return(0)\n", p, mode);
#endif
       fflush(stderr);
       _exit(1);
     }
#endif

#ifdef DEF_RR
    /*
     * find next instance of this thread in timer queue
     */
    if (mode == ALL_TIME) {
      tmr = pthread_timer.head;
      while (tmr && tmr->thread != p)
        tmr = tmr->next[TIMER_QUEUE];
    }
    else
      tmr = NO_TIMER;

    /*
     * check if head of timer queue can be woken up, i.e. now > tmr->tp
     */
    if (tmr == NO_TIMER && !time_read)
#else
    if (!time_read)
#endif
      if (gettimeofday(&now, (struct timezone *) NULL)) {
#ifdef TIMER_DEBUG
        dbgleon_printf( "pthread_cancel_timed_sigwait: exit1\n");
#endif
        set_errno(EINVAL);
	return(-1);
      }
      else
        time_read = TRUE;

    if (time_read) {
      tmr = pthread_timer.head;
#ifdef DEF_RR
      if (tmr)
        p = tmr->thread;
#else
      p = tmr;
#endif
    }
      
  } while (tmr && (!time_read || GTEQ_TIME(now, tmr->tp)));

  /*
   * We need recalculate the threads waiting for synchronous I/O
   */
#if defined(IO) && defined(_M_UNIX)
  if (recalculate)
    aio_calculate();
#endif

  /*
   * if head of timer queue hasn't changed, no action
   */
  if (tmr == old_tmr_head && !signaled) {
#ifdef TIMER_DEBUG
    dbgleon_printf( "pthread_cancel_timed_sigwait: exit2\n");
#endif
    return(0);
  }

  /*
   * delete SIGALRM from new_signals if it came in meanwhile
   */
  sigdelset(&new_signals, SIGALRM);

  /*
   * overwrite timer if current request needs to be served next or invalidate
   */
  if (tmr != NO_TIMER)
    MINUS_TIME(it.it_value, tmr->tp, now);
  else
    it.it_value.tv_sec = it.it_value.tv_usec = 0;

  it.it_interval.tv_sec = it.it_interval.tv_usec = 0;
  if (setitimer(ITIMER_REAL, &it, (struct itimerval *) NULL)) {
#ifdef DEBUG
    dbgleon_printf( "ERROR: setitimer in pthread_cancel_timed_sigwait\n");
#endif
    set_errno(EINVAL);
    return(-1);
  }

#ifdef TIMER_DEBUG
  last_alarm.tv_sec = it.it_value.tv_sec;
  last_alarm.tv_usec = it.it_value.tv_usec;
#endif

#ifdef TIMER_DEBUG
  dbgleon_printf( "pthread_cancel_timed_sigwait: setitimer %d.%d sec.usec\n",
          it.it_value.tv_sec, it.it_value.tv_usec);
#endif
  
  return(0);
}

#endif /* STAND_ALONE */
