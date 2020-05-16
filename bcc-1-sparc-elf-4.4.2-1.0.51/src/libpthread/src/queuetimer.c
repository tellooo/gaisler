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

/*------------------------------------------------------------*/
/*
 * pthread_q_timed_enq - enqueing a pthread into a list sorted by time
 */
int pthread_q_timeout_enq(pthread_queue_t q, pthread_t p)
{
  pthread_t next;
  PTRACEIN;
  PASSERT(TAILQ_REMOVED(p, pt_qelem[K_QUEUES_TIMEOUT]));
  
  if (!p || p->state & T_RETURNED) {
    PDEBUG(PDBG_TIMEOUT|PDBG_ERROR, "ERROR: q_timed_enq on returned thread attempted\n");
  }
  
  TAILQ_FOREACH(next, q, pt_qelem[K_QUEUES_TIMEOUT]) {
    if (!GT_NTIME(p->tp, next->tp)) 
      break;
  }
  if (next) {
    TAILQ_INSERT_BEFORE(next, p, pt_qelem[K_QUEUES_TIMEOUT]);
  } else {
    TAILQ_INSERT_TAIL(q, p, pt_qelem[K_QUEUES_TIMEOUT]);
  }
  
  PTRACE_QUEUE(PDBG_QUEUE,q,K_QUEUES_TIMEOUT);
  return 0;
}

/*------------------------------------------------------------*/
/*
 * pthread_q_timed_deq - dequeue thread from (timer) queue
 */
/*------------------------------------------------------------*/

void pthread_q_timeout_deq(pthread_queue_t q, pthread_t p)
{
  PTRACEIN;
  TAILQ_REMOVE(q,p,pt_qelem[K_QUEUES_TIMEOUT]);
  
  PTRACE_QUEUE(PDBG_QUEUE,q,K_QUEUES_TIMEOUT);
}
