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

  @(#)queue.c	3.14 11/8/00

*/

/*
 * Implementation of queues
 */

#include "internals.h"
#include <fsu_pthread/malloc.h>
#include <fsu_pthread/debug.h>

#ifdef RAND_SWITCH
int pthread_n_ready;
#endif


/*------------------------------------------------------------*/
/*
 * pthread_q_all_find_receiver - find thread in all queue s.t.
 * (a) either a handler is installed and the thread has the signal unmasked,
 * (b) or the signal is in the sigwaitset,
 * and return this thread (or NO_THREAD).
 */
pthread_t pthread_q_all_find_receiver(q, sig)
     pthread_queue_t q;
     int sig;
{
  pthread_t t;
  PTRACEIN;

  TAILQ_FOREACH(t, &all, pt_qelem[K_QUEUES_ALL]) {
    PDEBUG_HEADER(PDBG_QUEUE);
    PDEBUG_PTNAME(PDBG_QUEUE,t);
    PDEBUG_SIMPLECODE(PDBG_QUEUE," mask: 0x%x\n",t->mask);
    if ((t != k_idle) && ( sigismember(&t->sigwaitset, sig) || !sigismember(&t->mask, sig)))
      break;
  }
  
  PDEBUG_HEADER(PDBG_QUEUE);
  PDEBUG_PTNAME(PDBG_QUEUE,t);
  PDEBUG_SIMPLECODE(PDBG_QUEUE,"\n");
  return(t);
}

/*------------------------------------------------------------*/
/*
 * pthread_q_sleep_thread - block current thread: move from anywhere in ready
 * queue to some other queue;
 * Assumes SET_KERNEL_FLAG
 */
void pthread_q_sleep_thread(q, p, index)
pthread_queue_t q;
pthread_t p;
int index;
{
  PTRACEIN;
  pthread_q_deq(&ready, p, PRIMARY_QUEUE);
  if (index == PRIMARY_QUEUE)
    pthread_q_primary_enq(q, p);
  p->state &= ~T_RUNNING;
  p->state |= T_BLOCKED;
}

/*------------------------------------------------------------*/
/*
 * pthread_q_sleep - block current thread: move from head of ready to some 
 * other queue; Assumes SET_KERNEL_FLAG
 */
void pthread_q_sleep(q, index)
pthread_queue_t q;
int index;
{
  pthread_t p;
  PTRACEIN;
  
  p = pthread_q_deq_head(&ready, PRIMARY_QUEUE);
  if (index == PRIMARY_QUEUE)
    pthread_q_primary_enq(q, p);
  p->state &= ~T_RUNNING;
  p->state |= T_BLOCKED;
}

/*------------------------------------------------------------*/
/* 
 * pthread_q_wakeup -  Wakeup head of the queue and return head.
 * If there one exists, deq him and put him on the run queue ready
 * to execute.  Note: Deq takes care of who runs when., so scheduling
 * goes on fine!
 * Assumes SET_KERNEL_FLAG
 */
void pthread_q_wakeup(q, index)
     pthread_queue_t q;
     int index;
{
  pthread_t p;
  PTRACEIN;

  p = pthread_q_deq_head(q, index);
  if (p != NO_PTHREAD && !(p->state & T_RUNNING)) {
    p->state &= ~(T_BLOCKED | T_INTR_POINT);
    p->state |= T_RUNNING;
    pthread_q_primary_enq(&ready, p);
  }
}

/*------------------------------------------------------------*/
/*
 * pthread_q_wakeup_thread -  same as pthread_q_wakeup but for specific thread
 * return pointer to thread if thread found in queue, NO_PTHREAD otherwise
 */
void pthread_q_wakeup_thread(q, p, index)
     pthread_queue_t q;
     pthread_t p;
{
  PTRACEIN;
  
  if (q != NO_QUEUE)
    pthread_q_deq(q, p, index);

  if (p != NO_PTHREAD && !(p->state & T_RUNNING)) {
    p->state &= ~(T_BLOCKED | T_INTR_POINT);
    p->state |= T_RUNNING;
    pthread_q_primary_enq(&ready, p);
  }
}
      
/*------------------------------------------------------------*/
/*
 * pthread_q_timed_wakeup_thread - same as pthread_q_wakeup_thread but 
 * for timer queue
 */
void pthread_q_timeout_wakeup_thread(pthread_queue_t q, pthread_t p, int activate)
{
  PTRACEIN;
  pthread_q_timeout_deq(q, p);
 
  if (activate && p != NO_PTHREAD && !(p->state & T_RUNNING)) {
    p->state &= ~(T_BLOCKED | T_INTR_POINT);
    p->state |= T_RUNNING;
    pthread_q_primary_enq(&ready, p);
  }
}
 
/*------------------------------------------------------------*/
/*
 * pthread_q_wakeup_all - Wake up all the threads waiting on a condition   
 * change.  See pthread_q_wakeup.
 * Assumes SET_KERNEL_FLAG
 */
void pthread_q_wakeup_all(q, index)
     pthread_queue_t q;
{
  pthread_t p;
  PTRACEIN;
  
  while ((p = pthread_q_deq_head(q, index)) != NO_PTHREAD)
    if (!(p->state & T_RUNNING)) {
      p->state &= ~(T_BLOCKED | T_INTR_POINT);
      p->state |= T_RUNNING;
      pthread_q_primary_enq(&ready, p);
    }
}

/*------------------------------------------------------------*/
#ifdef _POSIX_THREADS_PRIO_PROTECT
/*
 * pthread_mutex_q_adjust - Removes p from primary queue and reinserts
 * it in the primary queue in the first place among threads of same
 * priority. Unlocking a mutex should not take it to the end.
 * Assumes that unlocking can only lower the priority.
 * Assumes SET_KERNEL_FLAG
 */
void pthread_mutex_q_adjust(p)
     pthread_t p;
{
  pthread_queue_t q = p->queue;
  pthread_t next = TAILQ_NEXT(p,pt_qelem[PRIMARY_QUEUE]);
  PTRACEIN;
 
  /*
   * If queue has only one thread or new priority is still higher than
   * next priority, no reordering is necessary.
   */
  if (!next || p->attr.param.sched_priority >= next->attr.param.sched_priority)
    return;
 
  pthread_q_deq(q, p, PRIMARY_QUEUE);
  pthread_q_primary_enq_first(q, p);
}
/*------------------------------------------------------------*/
#endif
