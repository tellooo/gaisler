#include "internals.h"
#include "setjmp.h"
#include <fsu_pthread/asm.h>
#include <fsu_pthread/debug.h>
#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>
#include <asm-leon/irq.h>
#include <asm-leon/time.h>
#include <malloc.h>
#include <sys/lock.h>

#ifndef __SINGLE_THREAD__
__LOCK_INIT_RECURSIVE(static, __malloc_lock_object);
#endif

void
__malloc_lock (ptr)
     struct _reent *ptr;
{
#ifndef __SINGLE_THREAD__
  __lock_acquire_recursive (__malloc_lock_object);
#endif
}

void
__malloc_unlock (ptr)
     struct _reent *ptr;
{
#ifndef __SINGLE_THREAD__
  __lock_release_recursive (__malloc_lock_object);
#endif
}


