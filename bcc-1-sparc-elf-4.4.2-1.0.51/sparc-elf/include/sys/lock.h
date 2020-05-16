#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

#ifndef __leonbare__

/* dummy lock routines for single-threaded aps */

typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;

#define __LOCK_INIT(class,lock) static int lock = 0;
#define __LOCK_INIT_RECURSIVE(class,lock) static int lock = 0;
#define __lock_init(lock) (0)
#define __lock_init_recursive(lock) (0)
#define __lock_close(lock) (0)
#define __lock_close_recursive(lock) (0)
#define __lock_acquire(lock) (0)
#define __lock_acquire_recursive(lock) (0)
#define __lock_try_acquire(lock) (0)
#define __lock_try_acquire_recursive(lock) (0)
#define __lock_release(lock) (0)
#define __lock_release_recursive(lock) (0)

#else /* __leonbare__ */

#ifndef NULL
#define NULL    0
#endif

#include <sys/fsu_pthread_mutex.h>

# ifdef __threadx__

#ifndef _INCLUDE_LEON_ELFMACRO_h
#define _INCLUDE_LEON_ELFMACRO_h

#ifndef weak_alias  
/* Define ALIASNAME as a weak alias for NAME. */
#  define weak_alias(name, aliasname) _weak_alias (name, aliasname)
#  define _weak_alias(name, aliasname) \
      extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));
#endif

#ifndef strong_alias
/* Define ALIASNAME as a strong alias for NAME.  */
# define strong_alias(name, aliasname) _strong_alias(name, aliasname)
# define _strong_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)));
#endif

#ifndef __ASSEMBLER__
typedef int (*initcall_t)(void);
extern initcall_t __leonbare_initcall_start;
extern initcall_t __leonbare_initcall_end;

#endif

#if __GNUC_MINOR__ >= 3
# define __attribute_used__	__attribute__((__used__))
#else
# define __attribute_used__	__attribute__((__unused__))
#endif

#define __define_initcall(level,fn) \
	static initcall_t __initcall_##fn __attribute_used__ \
	__attribute__((__section__(".initcall" level ".init"))) = fn

#define libc_initcall(fn)		__define_initcall("1",fn)

#endif /* !_INCLUDE_LEON_STACK_h */

typedef pthread_mutex_t _LOCK_T;              /* using pthread_mutex_t as placeholder. Initilized to threadx mutex by __lock_init */
typedef pthread_mutex_t _LOCK_RECURSIVE_T;

#define __LOCK_INIT(class,lock,name) class _LOCK_T lock;	\
int __initcallfn_##lock() {					\
    __st_pthread_mutex_init (&(lock), NULL, name);		\
    return 0;							\
}								\
libc_initcall(__initcallfn_##lock);
	      
#define __LOCK_INIT_RECURSIVE(class,lock,name) __LOCK_INIT(class,lock,name) /*static int lock = 0;*/
#define __lock_init(lock)                                           (__st_pthread_mutex_init (&(lock), NULL))
#define __lock_init_recursive(lock) __lock_init(lock)
#define __lock_close(lock)                                          (__st_pthread_mutex_destroy (&(lock)))
#define __lock_close_recursive(lock) __lock_close(lock)
#define __lock_acquire(lock)                                        (__st_pthread_mutex_lock (&(lock)))
#define __lock_acquire_recursive(lock) __lock_acquire(lock)
#define __lock_try_acquire(lock)                                    (__st_pthread_mutex_trylock (&(lock)))
#define __lock_try_acquire_recursive(lock) __lock_try_acquire(lock)
#define __lock_release(lock)                                        (__st_pthread_mutex_unlock (&(lock)))
#define __lock_release_recursive(lock) __lock_release(lock)

# else /*__threadx__*/

/* Mutex type.  */
typedef pthread_mutex_t __libc_lock_t;
typedef pthread_mutex_t __libc_lock_recursive_t;

/* Define a lock variable NAME with storage class CLASS.  The lock must be
   initialized with __libc_lock_init before it can be used (or define it
   with __libc_lock_define_initialized, below).  Use `extern' for CLASS to
   declare a lock defined in another module.  In public structure
   definitions you must use a pointer to the lock structure (i.e., NAME
   begins with a `*'), because its storage size will not be known outside
   of libc.  */
#define __libc_lock_define(CLASS,NAME) \
  CLASS __libc_lock_t NAME;
#define __libc_lock_define_recursive(CLASS,NAME) \
  CLASS __libc_lock_recursive_t NAME;

/* Define an initialized lock variable NAME with storage class CLASS.

   For the C library we take a deeper look at the initializer.  For
   this implementation all fields are initialized to zero.  Therefore
   we don't initialize the variable which allows putting it into the
   BSS section.  (Except on PA-RISC and other odd architectures, where
   initialized locks must be set to one due to the lack of normal
   atomic operations.) */

#if __LT_SPINLOCK_INIT == 0
#  define __libc_lock_define_initialized(CLASS,NAME) \
  CLASS __libc_lock_t NAME;
#else
#  define __libc_lock_define_initialized(CLASS,NAME) \
  CLASS __libc_lock_t NAME = PTHREAD_MUTEX_INITIALIZER(NAME);
#endif

/* Define an initialized recursive lock variable NAME with storage
   class CLASS.  */
#define __libc_lock_define_initialized_recursive(CLASS,NAME) \
  CLASS __libc_lock_recursive_t NAME = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP(NAME);


/* Initialize the named lock variable, leaving it in a consistent, unlocked
   state.  */
#define __libc_lock_init(NAME) \
  (__st_pthread_mutex_init (&(NAME), NULL));

/* Same as last but this time we initialize a recursive mutex.  */
#define __libc_lock_init_recursive(NAME) \
  do {									      \
	pthread_mutexattr_t __attr;					      \
	__st_pthread_mutexattr_init (&__attr);				      \
	__st_pthread_mutexattr_settype (&__attr, PTHREAD_MUTEX_RECURSIVE_NP); \
	__st_pthread_mutex_init (&(NAME), &__attr);			      \
	__st_pthread_mutexattr_destroy (&__attr);				      \
  } while (0);

/* Finalize the named lock variable, which must be locked.  It cannot be
   used again until __libc_lock_init is called again on it.  This must be
   called on a lock variable before the containing storage is reused.  */
#define __libc_lock_fini(NAME)              (__st_pthread_mutex_destroy (&(NAME)))
/* Finalize recursive named lock.  */
#define __libc_lock_fini_recursive(NAME)     __libc_lock_fini (NAME)
/* Lock the named lock variable.  */
#define __libc_lock_lock(NAME)              (__st_pthread_mutex_lock (&(NAME)))
/* Lock the recursive named lock variable.  */
#define __libc_lock_lock_recursive(NAME)     __libc_lock_lock (NAME)
/* Try to lock the named lock variable.  */
#define __libc_lock_trylock(NAME)           (__st_pthread_mutex_trylock (&(NAME)))
/* Try to lock the recursive named lock variable.  */
#define __libc_lock_trylock_recursive(NAME)  __libc_lock_trylock (NAME)
/* Unlock the named lock variable.  */
#define __libc_lock_unlock(NAME)            (__st_pthread_mutex_unlock (&(NAME)))
/* Unlock the recursive named lock variable.  */
#define __libc_lock_unlock_recursive(NAME)   __libc_lock_unlock (NAME)

/* #ifndef weak_extern */
/* #define weak_extern(symbol) _weak_extern (symbol) */
/* #define _weak_extern(symbol) asm (".weak " #symbol); */
/* #endif */

/* #ifndef _LIBCLOCK_NOT_WEAK */
/* weak_extern(__st_pthread_mutex_init); */
/* weak_extern(__st_pthread_mutex_destroy);      */
/* weak_extern(__st_pthread_mutex_trylock);      */
/* weak_extern(__st_pthread_mutex_lock);         */
/* weak_extern(__st_pthread_mutex_unlock);       */
/* weak_extern(__st_pthread_mutexattr_init);     */
/* weak_extern(__st_pthread_mutexattr_destroy);  */
/* weak_extern(__st_pthread_mutexattr_settype); */
/* #endif */

/* extern int __st_pthread_mutex_init        (pthread_mutex_t *__mutex, pthread_mutexattr_t *__mutex_attr); */
/* extern int __st_pthread_mutex_destroy     (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutex_trylock     (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutex_lock        (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutex_unlock      (pthread_mutex_t *__mutex); */
/* extern int __st_pthread_mutexattr_init    (pthread_mutexattr_t *__attr); */
/* extern int __st_pthread_mutexattr_destroy (pthread_mutexattr_t *__attr); */
/* extern int __st_pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind); */

/* /\* Functions that are used by this file and are internal to the GNU C library.  *\/ */
/* extern int __pthread_mutex_init        (pthread_mutex_t *__mutex, pthread_mutexattr_t *__mutex_attr); */
/* extern int __pthread_mutex_destroy     (pthread_mutex_t *__mutex); */
/* extern int __pthread_mutex_trylock     (pthread_mutex_t *__mutex); */
/* extern int __pthread_mutex_lock        (pthread_mutex_t *__mutex); */
/* extern int __pthread_mutex_unlock      (pthread_mutex_t *__mutex); */
/* extern int __pthread_mutexattr_init    (pthread_mutexattr_t *__attr); */
/* extern int __pthread_mutexattr_destroy (pthread_mutexattr_t *__attr); */
/* extern int __pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind); */

/* /\* Make the pthread functions weak so that we can elide them from */
/*    single-threaded processes.  *\/ */

/* #ifndef weak_extern */
/* #define weak_extern(symbol) _weak_extern (symbol) */
/* #define _weak_extern(symbol) asm (".weak " #symbol); */
/* #endif */

/* weak_extern (__pthread_mutex_init) */
/* weak_extern (__pthread_mutex_destroy) */
/* weak_extern (__pthread_mutex_lock) */
/* weak_extern (__pthread_mutex_trylock) */
/* weak_extern (__pthread_mutex_unlock) */
/* weak_extern (__pthread_mutexattr_init) */
/* weak_extern (__pthread_mutexattr_destroy) */
/* weak_extern (__pthread_mutexattr_settype) */
/* weak_extern (__pthread_once) */
/* weak_extern (__pthread_initialize) */

typedef __libc_lock_t _LOCK_T;
typedef __libc_lock_recursive_t _LOCK_RECURSIVE_T;

#define __LOCK_INIT(class,lock,name)              __libc_lock_define_initialized(class, lock)
#define __LOCK_INIT_RECURSIVE(class, lock,name)   __libc_lock_define_initialized_recursive(class, lock)

#define __lock_init(__lock)                  __libc_lock_init(__lock)
#define __lock_init_recursive(__lock)        __libc_lock_init_recursive(__lock)
#define __lock_acquire(__lock)               __libc_lock_lock(__lock)
#define __lock_acquire_recursive(__lock)     __libc_lock_lock_recursive(__lock)
#define __lock_release(__lock)               __libc_lock_unlock(__lock)
#define __lock_release_recursive(__lock)     __libc_lock_unlock_recursive(__lock)
#define __lock_try_acquire(__lock)           __libc_lock_trylock(__lock)
#define __lock_try_acquire_recursive(__lock) __libc_lock_trylock_recursive(__lock)
#define __lock_close(__lock)                 __libc_lock_fini(__lock)
#define __lock_close_recursive(__lock)       __libc_lock_fini_recursive(__lock)


#endif /* __threadx__ */
#endif /* __leonbare__ */

#endif /* __SYS_LOCK_H__ */
