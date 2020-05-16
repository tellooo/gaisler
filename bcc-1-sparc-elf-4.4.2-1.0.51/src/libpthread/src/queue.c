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

#define CHECK_STATE_CHANGE(q,t) \
  (q == &ready &&  TAILQ_FIRST(q) == t)

/*------------------------------------------------------------*/
/*
 * pthread_q_all_enq - enqueing a pthread into an unsorted queue (linked list)
 */
void pthread_q_all_enq(q, t)
     pthread_queue_t q;
     pthread_t t;
{
  PTRACEIN;
#ifdef DEBUG
  if (!t || t->state & T_RETURNED)
    dbgleon_printf("ERROR: q_all_enq on returned thread attempted\n");
#endif
  TAILQ_INSERT_HEAD(&all,t,pt_qelem[K_QUEUES_ALL]);
}

/*------------------------------------------------------------*/

void pthread_q_enq_before(q, t, next, idx)
     pthread_queue_t q;
     pthread_t t;
     pthread_t next;
     int idx;
{
  PTRACEIN;
  PDEBUG(PDBG_QUEUE,"q: 0x%x t: 0x%x next: 0x%x idx: %d ",q, t, next, idx);
  
  if (next) {
    TAILQ_INSERT_BEFORE(next,t,pt_qelem[idx]);
  } else {
    TAILQ_INSERT_TAIL(q, t, pt_qelem[idx]);
  }

  if (CHECK_STATE_CHANGE(q,t)) {
    state_change = TRUE;
  }
  
#ifdef RAND_SWITCH
  if (q == &ready)
    pthread_n_ready++;
#endif
  
  PTRACE_READYQUEUE;  
}

void pthread_q_enq_after(q, t, next, idx)
     pthread_queue_t q;
     pthread_t t;
     pthread_t next;
     int idx;
{
  PTRACEIN;
  PDEBUG(PDBG_QUEUE,"q: 0x%x t: 0x%x next: 0x%x idx: %d ",q, t, next, idx);
  
  if (next) {
    TAILQ_INSERT_AFTER(q,next,t,pt_qelem[idx]);
  } else {
    TAILQ_INSERT_HEAD(q, t, pt_qelem[idx]);
  }

  if (CHECK_STATE_CHANGE(q,t)) {
    state_change = TRUE;
  }
  
#ifdef RAND_SWITCH
  if (q == &ready)
    pthread_n_ready++;
#endif
  
  PTRACE_READYQUEUE;
}

/*------------------------------------------------------------*/

/*
 * pthread_q_primary_enq - enqueing a pthread into a queue sorted by priority
 * (ordering: last in priority level)
 */
void pthread_q_primary_enq(q, t)
     pthread_queue_t q;
     pthread_t t;
{
  pthread_t next = 0;
  PTRACEIN;
  
#ifdef DEBUG
  if (!t || t->state & T_RETURNED)
    dbgleon_printf("ERROR: q_primary_enq on returned thread attempted\n");
#endif

  TAILQ_FOREACH(next, q, pt_qelem[K_QUEUES_PRIMARY]) {
    if (next->attr.param.sched_priority < t->attr.param.sched_priority) {
      break;
    }
  }
  pthread_q_enq_before(q, t, next, K_QUEUES_PRIMARY);
  t->queue = q;
}

/*
 * pthread_q_primary_enq_first - enqueing a pthread into a queue
 * sorted by priority
 * (ordering: first in priority level)
 */
void pthread_q_primary_enq_first(q, t)
     pthread_queue_t q;
     pthread_t t;
{
  pthread_t next = 0;
  PTRACEIN;
  
#ifdef DEBUG
  if (!t || t->state & T_RETURNED)
    dbgleon_printf("ERROR: q_primary_enq on returned thread attempted\n");
#endif

  TAILQ_FOREACH(next, q, pt_qelem[K_QUEUES_PRIMARY]) {
    if (next->attr.param.sched_priority <= t->attr.param.sched_priority) {
      break;
    }
  }
  pthread_q_enq_after(q, t, next, K_QUEUES_PRIMARY);
  t->queue = q;

}


/*------------------------------------------------------------*/
/*
 * pthread_q_enq_head - enqueue head of queue
 */
void pthread_q_enq_head(q, t, index)
     pthread_queue_t q;
     pthread_t t;
     int index;
{
  PTRACEIN;
  
#ifdef DEBUG
  if (!t || t->state & T_RETURNED)
    dbgleon_printf("ERROR: q_enq_head on returned thread attempted\n");
#endif

  pthread_q_enq_after(q, t, 0, index);
  if (!(t->queue))
    t->queue = q;
}

#if defined(RR_SWITCH) || defined(RAND_SWITCH)
/*------------------------------------------------------------*/
/*
 * pthread_q_enq_tail - enqueing a pthread into a queue at the tail (RR)
 */
void pthread_q_enq_tail(q)
     pthread_queue_t q;
{
  pthread_t prev = NO_PTHREAD;
  pthread_t t = mac_pthread_self();
  pthread_t last = TAILQ_LAST(q,pthread_queue);
  PTRACEIN;
  PTRACE_READYQUEUE;
 
#ifdef DEBUG
  if (!t || t->state & T_RETURNED)
    dbgleon_printf("ERROR: q_primary_enq on returned thread attempted\n");
#endif

  if (TAILQ_HASTWO(q,pt_qelem[PRIMARY_QUEUE]) && (last == k_idle)) {
    TAILQ_INSERT_BEFORE(last,t,pt_qelem[PRIMARY_QUEUE]);
  } else {
    TAILQ_INSERT_TAIL(q, t, pt_qelem[PRIMARY_QUEUE]);
  }
  
  if (CHECK_STATE_CHANGE(q,t)) {
    state_change = TRUE;
  }

  if (!(t->queue))
    t->queue = q;
  
#ifdef RAND_SWITCH 
  if (q == &ready)
    pthread_n_ready++; 
#endif

  PTRACE_READYQUEUE;
}
#endif

/*------------------------------------------------------------*/
/*
 * pthread_q_deq - dequeue a thread from a queue
 */
void pthread_q_deq(q, t, index)
     pthread_queue_t q;
     pthread_t t;
     int index;
{
  pthread_t next = 0;
  timer_ent_t tmr;
  struct timeval now;
  PTRACEIN;

  TAILQ_FOREACH(next, q, pt_qelem[index]) {
    if (next == t) {
      break;
    }
  }
  
  if (next == NO_PTHREAD) {
    PDEBUG(PDBG_ERROR,"### !!! pthread_q_deq(thread %x, index %d) failed ###\n", t, index);
    return;
  }

  if (CHECK_STATE_CHANGE(q,t)) {
    state_change = TRUE;
  }
  
  TAILQ_REMOVE(q,t,pt_qelem[index]);
  
  if (index == PRIMARY_QUEUE) {
    t->queue = NULL;
#ifdef DEF_RR
    if (t == mac_pthread_self() && t->attr.sched == SCHED_RR &&
	t->state & T_ASYNCTIMER) {
      for (tmr = pthread_timer.head; tmr; tmr = tmr->next[TIMER_QUEUE])
	if (tmr->thread == t && tmr->mode == RR_TIME)
	  break;
      if (tmr && !gettimeofday(&now, (struct timezone *) NULL) &&
	  GT_TIME(tmr->tp, now))
	MINUS_TIME(t->interval, tmr->tp, now);
      else
	t->interval.tv_usec = t->interval.tv_sec = 0;

      pthread_cancel_timed_sigwait(t, FALSE, RR_TIME, FALSE);
    }
#endif
  }

#ifdef RAND_SWITCH 
  if (q == &ready)
    pthread_n_ready--; 
#endif

  PTRACE_READYQUEUE;
}


/*------------------------------------------------------------*/
/*
 * pthread_q_deq_head - dequeue head of queue and return head
 */
pthread_t pthread_q_deq_head(q, index)
     pthread_queue_t q;
     int index;
{
  pthread_t t;
  PTRACEIN;

  if ((t = TAILQ_FIRST(q)) != NO_PTHREAD) {
    if (q == &ready)
      state_change = TRUE;

    TAILQ_REMOVE(q, t, pt_qelem[index]);
    
    if (index == PRIMARY_QUEUE) {
      t->queue = NULL;
#ifdef DEF_RR
      if (t == mac_pthread_self() && t->attr.sched == SCHED_RR &&
	  t->state & T_ASYNCTIMER)
        pthread_cancel_timed_sigwait(t, FALSE, RR_TIME, FALSE);
#endif
    }
  }

#ifdef RAND_SWITCH  
  if (q == &ready)
    pthread_n_ready--;  
#endif

  PTRACE_READYQUEUE;
  PDEBUG(PDBG_QUEUE,"return 0x%x",t);
  return(t);
}

#ifdef RAND_SWITCH
/*------------------------------------------------------------*/
/*
 * pthread_q_exchange_rand - Remove the current thread from the queue and
 *                   put that at the end of the queue and put the
 * thread #index in the queue at the head.
 */
void pthread_q_exchange_rand(q)
     pthread_queue_t q;
{
  pthread_t t = mac_pthread_self();
  int i, index = (int)random() % pthread_n_ready;
  PTRACEIN;

#ifdef DEBUG                        
  if (!t || t->state & T_RETURNED)
    dbgleon_printf("ERROR: q_primary_enq on returned thread attempted\n");
#endif

  pthread_q_deq(q, t, PRIMARY_QUEUE);
  pthread_q_enq_tail(q);
  t = q->head;
  for (i = 0; i < index; i++)
    t = t->next[PRIMARY_QUEUE];

  pthread_q_deq(q, t, PRIMARY_QUEUE);
  pthread_q_enq_head(q, t, PRIMARY_QUEUE);
}
#endif

