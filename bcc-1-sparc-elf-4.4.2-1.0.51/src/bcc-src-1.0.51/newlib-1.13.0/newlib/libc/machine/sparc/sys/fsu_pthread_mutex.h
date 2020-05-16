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

#ifndef _pthread_pthread_mutex_h
#define _pthread_pthread_mutex_h

#include <sys/fsu_pthread_queue.h>

/*
 * Mutex objects.
 */

typedef int pthread_protocol_t;

typedef TAILQ_HEAD(pthread_queue,pthread)  * pthread_queue_t;

#define pthread_mutex_t_defined
typedef struct pthread_mutex {
        struct pthread_queue queue;
        char lock;
        struct pthread *owner;
        int flags;
        int count;
        int prioceiling;
        pthread_protocol_t protocol;
        int prev_max_ceiling_prio;
        TAILQ_ENTRY(pthread_mutex) dbglist;
        char *dbgname;
        int _fitothers[16];
} pthread_mutex_t;

typedef struct {
        int flags;
        int prioceiling;
        pthread_protocol_t protocol;
} pthread_mutexattr_t;

/*
 * If we have Priority Ceilings, then we have Priority Inheritance
 */
#define PTHREAD_PRIO_NONE    0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2

#define PTHREAD_MUTEX_TIMED_NP     0
#define PTHREAD_MUTEX_RECURSIVE_NP 1

#define PTHREAD_MUTEX_INITIALIZER(m) {		  \
  TAILQ_HEAD_INITIALIZER(m.queue), /* queue */    \
  0,                 /* lock */			  \
  0,                 /* owner */		  \
  1<<PTHREAD_MUTEX_TIMED_NP,       /* flags */    \
  0,                 /* count */		  \
  0,                 /* prioceiling */		  \
  PTHREAD_PRIO_NONE, /* protocol */		  \
  0,                 /* prev_max_ceiling_prio */  \
  { 0, 0 },                                       \
  0                                               \
}

#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP(m) { \
  TAILQ_HEAD_INITIALIZER(m.queue), /* queue */    \
  0,                 /* lock */			  \
  0,                 /* owner */		  \
  1<<PTHREAD_MUTEX_RECURSIVE_NP,   /* flags */    \
  0,                 /* count */		  \
  0,                 /* prioceiling */		  \
  PTHREAD_PRIO_NONE, /* protocol */		  \
  0,                 /* prev_max_ceiling_prio */  \
  { 0, 0 },                                       \
  0                                               \
}

/******************************/
/* Mutex Attributes Functions */
/*       Mutex Functions      */
/******************************/

#ifdef __cplusplus
extern "C" {
#endif

extern int pthread_mutex_lock     (pthread_mutex_t *__mutex);
extern int pthread_mutex_trylock  (pthread_mutex_t *__mutex);
extern int pthread_mutex_unlock   (pthread_mutex_t *__mutex);
extern int pthread_mutex_init     (pthread_mutex_t *__mutex,
				   pthread_mutexattr_t *__attr);
extern int pthread_mutex_destroy  (pthread_mutex_t *__mutex);
extern int pthread_mutex_setprioceiling
                                  (pthread_mutex_t *__mutex,
                                   int __prioceiling,
				   int *__old_prioceiling);
extern int pthread_mutex_getprioceiling
                                  (pthread_mutex_t *__mutex,
				   int *__prioceiling);
extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr);
extern int pthread_mutexattr_destroy
                                  (pthread_mutexattr_t *__attr);
extern int pthread_mutexattr_setprotocol
                                  (pthread_mutexattr_t *__attr,
                                   pthread_protocol_t __protocol);
extern int pthread_mutexattr_getprotocol
                                  (pthread_mutexattr_t *__attr,
				   int *__protocol);

extern int pthread_mutexattr_setprioceiling
                                  (pthread_mutexattr_t *__attr,
                                   int __prioceiling);
extern int pthread_mutexattr_getprioceiling
                                  (pthread_mutexattr_t *__attr,
				   int *__prioceiling);
extern int pthread_mutexattr_getpshared
                                  (pthread_mutexattr_t *__attr,
                                   int *__pshared);
extern int pthread_mutexattr_setpshared
                                  (pthread_mutexattr_t *__attr,
                                   int __pshared);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* !_pthread_pthread_mutex_h */
