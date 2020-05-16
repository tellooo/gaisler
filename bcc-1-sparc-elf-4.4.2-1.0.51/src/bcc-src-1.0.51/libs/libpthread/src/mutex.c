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

  @(#)mutex.c	3.14 11/8/00

*/

/* 
 * Functions for the support of mutual exclusion - mutexes and their 
 * attributes.  
 */

#define _LIBCLOCK_NOT_WEAK
#include "internals.h"
#include "mutex.h"
#ifdef TDI_SUPPORT
#include "tdi-aux.h"
#endif
#include <fsu_pthread/debug.h>
#include <asm-leon/elfmacro.h>
#include <asm-leon/liblocks.h>

/*------------------------------------------------------------*/
/*
 * pthread_mutex_lock - Checks are made to see if the mutex is 
 * currently in use or not.  If the mutex is not in use, then its 
 * locked. Otherwise the currently executing thread is put in the 
 * wait queue and a new thread is selected. (Fast mutex_lock without
 * error checks can be found in pthread_sched.S.)
 */
int __pthread_mutex_lock(mutex)
     pthread_mutex_t *mutex;
{
  register pthread_t p = mac_pthread_self();
  
  PTRACEIN;
  
  if (mutex == NO_MUTEX)
    return(EINVAL);

  switch (mutex->flags) {
  case 1<<PTHREAD_MUTEX_TIMED_NP:
    if (mutex->owner == p)
      return(EDEADLK);
    break;
  case 1<<PTHREAD_MUTEX_RECURSIVE_NP:
    if (mutex->owner == p) {
      mutex->count++;
      return 0;
    }
    break;
  default:
    return(EINVAL);
  }
  
#ifdef _POSIX_THREADS_PRIO_PROTECT
  if (mutex->protocol == PTHREAD_PRIO_PROTECT && p->attr.param.sched_priority > mutex->prioceiling)
    return(EINVAL);
#endif /* _POSIX_THREADS_PRIO_PROTECT */

  SET_KERNEL_FLAG;
  SIM_SYSCALL(mutex->lock);
  mac_mutex_lock(mutex, p);
  CLEAR_KERNEL_FLAG;
  return(0);
}
strong_alias (__pthread_mutex_lock, pthread_mutex_lock)
strong_alias (__pthread_mutex_lock, __st_pthread_mutex_lock)

/*------------------------------------------------------------*/
/*
 * pthread_mutex_trylock - Just try to lock the mutex. If 
 * lock succeeds return. Otherwise return right away without 
 * getting the lock. (Fast mutex_trylock without
 * error checks can be found in pthread_sched.S.)
 */
int __pthread_mutex_trylock(mutex)
     pthread_mutex_t *mutex;
{

pthread_t p = mac_pthread_self();
  PTRACEIN;

  if (mutex == NO_MUTEX)
    return(EINVAL);

  switch (mutex->flags) {
  case 1<<PTHREAD_MUTEX_TIMED_NP:
    if (mutex->owner == p)
      return(EDEADLK);
    break;
  case 1<<PTHREAD_MUTEX_RECURSIVE_NP:
    if (mutex->owner == p) {
      mutex->count++;
      return 0;
    }
    break;
  default:
    return(EINVAL);
  }
  
#ifdef _POSIX_THREADS_PRIO_PROTECT
  if (mutex->protocol == PTHREAD_PRIO_PROTECT &&
      p->attr.param.sched_priority > mutex->prioceiling)
    return(EINVAL);
#endif /* _POSIX_THREADS_PRIO_PROTECT */

  SET_KERNEL_FLAG;
  if (mutex->lock) {
    CLEAR_KERNEL_FLAG;
    return(EBUSY);
  }
 
  mutex->lock = TRUE;
  mutex->owner = p;
  mutex->count = 0;
#ifdef NOERR_CHECK 
  mac_change_lock_prio(mutex, p); 
#else 
#ifdef _POSIX_THREADS_PRIO_PROTECT 
 if (mutex->protocol == PTHREAD_PRIO_PROTECT) 
   mac_change_lock_prio(mutex, p);
#endif                  
#endif
 
  CLEAR_KERNEL_FLAG;
  return(0);
}
strong_alias (__pthread_mutex_trylock, pthread_mutex_trylock)
strong_alias (__pthread_mutex_trylock, __st_pthread_mutex_trylock)

/*------------------------------------------------------------*/
/*
 * pthread_mutex_unlock - Called by the owner of mutex to release
 * the mutex. (Fast mutex_unlock without
 * error checks can be found in pthread_sched.S.)
 */
int __pthread_mutex_unlock(mutex)
     pthread_mutex_t *mutex;
{
pthread_t p = mac_pthread_self();
  PTRACEIN;

  if (mutex == NO_MUTEX)
    return(EINVAL);

  if (mutex->owner != mac_pthread_self())
    return(EPERM );

  switch (mutex->flags) {
  case 1<<PTHREAD_MUTEX_TIMED_NP:
    break;
  case 1<<PTHREAD_MUTEX_RECURSIVE_NP:
    if (mutex->count > 0) {
      mutex->count--;
      return 0;
    }
    break;
  default:
    return(EINVAL);
  }

  SET_KERNEL_FLAG;
  SIM_SYSCALL(mutex->queue.head != mutex->queue.tail);
  mac_mutex_unlock(mutex, p,  /* NULL */);
  CLEAR_KERNEL_FLAG;
  return(0);
}
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock)
strong_alias (__pthread_mutex_unlock, __st_pthread_mutex_unlock)

/*------------------------------------------------------------*/
/*
 * pthread_mutex_init - Initialize the mutex and at the same time 
 * ensure that it's usaable.  Set the attribute values from attr 
 * specified. No check is made to see if the attributes are right 
 * as yet.
 */
int __pthread_mutex_init(mutex, attr)
     pthread_mutex_t *mutex;
     pthread_mutexattr_t *attr;
{
  PTRACEIN;
  if (mutex == NO_MUTEX)
    return(EINVAL);
#ifdef _POSIX_THREADS_PRIO_PROTECT
#ifndef NOERR_CHECK
  if (attr && ((!attr->flags) ||
	       (!((attr->protocol == PTHREAD_PRIO_NONE) ||
		  (attr->protocol == PTHREAD_PRIO_INHERIT) ||
		  (attr->protocol == PTHREAD_PRIO_PROTECT))) ||
	       (attr->prioceiling < 0))) {
    PDEBUG(PDBG_ASSERT,"EINVAL: flags: %d protocol: %d ceil: %d\n", attr->flags, attr->protocol, attr->prioceiling);
    return(EINVAL);
  }
#endif
#endif

  if (!attr)
    attr = &pthread_mutexattr_default;

  mutex->owner = NO_PTHREAD;
  mutex->flags = attr->flags;
  pthread_queue_init(&mutex->queue);
#ifdef _POSIX_THREADS_PRIO_PROTECT
  mutex->prioceiling = attr->prioceiling;
  mutex->protocol = attr->protocol;
#endif
  mutex->lock = FALSE;
  return(0);
}  
strong_alias (__pthread_mutex_init, pthread_mutex_init)
strong_alias (__pthread_mutex_init, __st_pthread_mutex_init)

/*------------------------------------------------------------*/
/*
 * pthread_mutex_destroy - Destroys the mutex.
 */
int __pthread_mutex_destroy(mutex) 
     pthread_mutex_t *mutex;
{
  PTRACEIN;
  if (mutex == NO_MUTEX)
    return(EINVAL);

  /*
   * free mutex only if not locked and not associated with any cond var
   */
#ifdef NOERR_CHECK
  if (pthread_test_and_set(&mutex->lock))
    return(EBUSY);
#else /* !NOERR_CHECK */
  SET_KERNEL_FLAG;
  if (mutex->lock) {
    CLEAR_KERNEL_FLAG;
    return(EBUSY);
  }
  CLEAR_KERNEL_FLAG;
#endif /* !NOERR_CHECK */

  mutex->flags = 0;

  return(0);
}
strong_alias (__pthread_mutex_destroy, pthread_mutex_destroy)
strong_alias (__pthread_mutex_destroy, __st_pthread_mutex_destroy)

/*------------------------------------------------------------*/
/*
 * pthread_mutex_setprioceiling - locks the mutex, changes the
 * mutex's priority ceiling and releases the mutex.
 */
int pthread_mutex_setprioceiling(mutex, prioceiling, old_prioceiling)
pthread_mutex_t *mutex;
int prioceiling;
int *old_prioceiling;
{
  PTRACEIN;

  if (mutex == NO_MUTEX || !mutex->flags || !old_prioceiling)
    return(EINVAL);
#ifdef _POSIX_THREADS_PRIO_PROTECT
  if (prioceiling >= MIN_PRIORITY && prioceiling <= MAX_PRIORITY) {
    if (!pthread_mutex_trylock(mutex)) {
      if (old_prioceiling) {
	*old_prioceiling = mutex->prioceiling;
      }
      mutex->prioceiling = prioceiling;
      pthread_mutex_unlock(mutex);
      return(ENOSYS);
    }
    else
      return(EPERM);
  }
  else
    return(EINVAL);
#else
  return(ENOTSUP);
#endif
}

/*------------------------------------------------------------*/
/*
 * pthread_mutex_getprioceiling - Returns the current priority
 * ceiling of the mutex in "prioceiling".
 */
int pthread_mutex_getprioceiling(mutex, prioceiling)
pthread_mutex_t *mutex;
int *prioceiling;
{
  PTRACEIN;

  if (mutex == NO_MUTEX || !mutex->flags || !prioceiling)
    return(EINVAL);
  
#ifdef _POSIX_THREADS_PRIO_PROTECT
  if (mutex->prioceiling >= MIN_PRIORITY &&
      mutex->prioceiling <= MAX_PRIORITY) {
    *prioceiling = mutex->prioceiling;
    return(0);
  }
  else
    return(EINVAL);
#else
  return(ENOTSUP);
#endif
}

/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_init - Initializes the mutex attribute object
 * with default values.
 */
int __pthread_mutexattr_init(attr)
pthread_mutexattr_t *attr;
{
  PTRACEIN;
  if (!attr)
    return(EINVAL);

  attr->flags = TRUE;
#ifdef _POSIX_THREADS_PRIO_PROTECT
  attr->prioceiling = DEFAULT_PRIORITY;
  attr->protocol = PTHREAD_PRIO_NONE;
#endif
  return(0);
}
strong_alias (__pthread_mutexattr_init, pthread_mutexattr_init)
strong_alias (__pthread_mutexattr_init, __st_pthread_mutexattr_init)

/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_destroy - Destroys the mutex attribute object.
 */
int __pthread_mutexattr_destroy(attr)
pthread_mutexattr_t *attr;
{
  PTRACEIN;

  if (!attr || !attr->flags)
    return(EINVAL);
  attr->flags = FALSE;
#ifdef _POSIX_THREADS_PRIO_PROTECT
  attr->prioceiling = DEFAULT_PRIORITY;
  attr->protocol = PTHREAD_PRIO_NONE;
#endif  
  return(0);
}
strong_alias (__pthread_mutexattr_destroy, pthread_mutexattr_destroy)
strong_alias (__pthread_mutexattr_destroy, __st_pthread_mutexattr_destroy)

/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_setprotocol - Sets the protocol (value can be
 * PTHREAD_PRIO_NONE, PTHREAD_PRIO_INHERIT or PTHREAD_PRIO_PROTECT)
 * for the mutex attr.
 */
int pthread_mutexattr_setprotocol(attr,protocol)
pthread_mutexattr_t *attr;
pthread_protocol_t protocol;
{
  PTRACEIN;
  
  if (!attr || !attr->flags)
    return(EINVAL);
#ifndef _POSIX_THREADS_PRIO_PROTECT
  return(ENOSYS);
#else
  if (protocol < PTHREAD_PRIO_NONE || protocol > PTHREAD_PRIO_PROTECT)
    return(EINVAL);
  if (protocol == PTHREAD_PRIO_INHERIT)
    return(ENOTSUP);
  attr->protocol = protocol;
  return(0);
#endif
}

/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_getprotocol - Gets the current protcol.
 */
int pthread_mutexattr_getprotocol(attr, protocol)
pthread_mutexattr_t *attr;
int *protocol;
{
  PTRACEIN;
  if (!attr || !attr->flags || !protocol)
    return(EINVAL);
#ifdef _POSIX_THREADS_PRIO_PROTECT
  *protocol = attr->protocol;
  return(0);
#else
  return(ENOSYS);
#endif
}

/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_setprioceiling - Sets the priority ceiling
 * for the mutex attribute object.
 */
int pthread_mutexattr_setprioceiling(attr, prioceiling)
pthread_mutexattr_t *attr;
int prioceiling;
{
  PTRACEIN;
  if (!attr || !attr->flags)
    return(EINVAL);
#ifndef _POSIX_THREADS_PRIO_PROTECT
  return(ENOSYS);
#else
  if (prioceiling > MAX_PRIORITY || prioceiling < MIN_PRIORITY)
    return(EINVAL);
  attr->prioceiling = prioceiling;
  return(0);
#endif
}

/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_getprioceiling - returns the current priority
 * ceiling of the mutex attribute object.
 */
int pthread_mutexattr_getprioceiling(attr, prioceiling)
pthread_mutexattr_t *attr;
int *prioceiling;
{
  PTRACEIN;
  if (!attr || !attr->flags || !prioceiling)
    return(EINVAL);
#ifdef _POSIX_THREADS_PRIO_PROTECT
  *prioceiling = attr->prioceiling;
  return(0);
#else
  return(ENOSYS);
#endif
}

/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_getpshared - Not Implemented. Returns ENOSYS.
 */
 
int pthread_mutexattr_getpshared(attr, pshared)
pthread_mutexattr_t *attr;
int *pshared;
{
  PTRACEIN;
  return(ENOSYS);
}
 
/*------------------------------------------------------------*/
/*
 * pthread_mutexattr_setpshared - Not Implemented. Returns ENOSYS.
 */
 
int pthread_mutexattr_setpshared(attr, pshared)
pthread_mutexattr_t *attr;
int pshared;
{
  PTRACEIN;
  return(ENOSYS);
}


int __pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind)
{
  PTRACEIN;
  switch (kind) {
  case PTHREAD_MUTEX_TIMED_NP:
    PDEBUG(PDBG_MUTEX,"PTHREAD_MUTEX_TIMED_NP");
    break;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    PDEBUG(PDBG_MUTEX,"PTHREAD_MUTEX_RECURSIVE_NP");
    break;
  default:
      return EINVAL;
  }
  
  attr->flags = 1 << kind;
  return 0;
}
weak_alias   (__pthread_mutexattr_settype, pthread_mutexattr_settype)
strong_alias ( __pthread_mutexattr_settype, __pthread_mutexattr_setkind_np)
weak_alias   (__pthread_mutexattr_setkind_np, pthread_mutexattr_setkind_np)
strong_alias ( __pthread_mutexattr_settype, __st_pthread_mutexattr_setkind_np)

void pthread_leonbare_lockinit() 
{
  __lbst_pthread_mutex_init = __pthread_mutex_init;
  __lbst_pthread_mutex_destroy = __pthread_mutex_destroy;
  __lbst_pthread_mutex_trylock = __pthread_mutex_trylock;
  __lbst_pthread_mutex_lock = __pthread_mutex_lock;
  __lbst_pthread_mutex_unlock = __pthread_mutex_unlock;
  __lbst_pthread_mutexattr_init = __pthread_mutexattr_init;
  __lbst_pthread_mutexattr_destroy = __pthread_mutexattr_destroy;
  __lbst_pthread_mutexattr_settype = __pthread_mutexattr_settype;
}

