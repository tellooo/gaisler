#ifndef _INCLUDE_LEONSPINLOCK_h
#define _INCLUDE_LEONSPINLOCK_h

#include <asm-leon/leon.h>

typedef struct {
	unsigned char lock;
} raw_spinlock_t;

#define __RAW_SPIN_LOCK_UNLOCKED	{ 0 }

typedef struct {
	volatile unsigned int lock;
} raw_rwlock_t;

#define __RAW_RW_LOCK_UNLOCKED		{ 0 }

static inline void __raw_spin_lock(raw_spinlock_t *lock)
{
	__asm__ __volatile__(
	"\n1:\n\t"
	"ldstuba	[%0]1, %%g2\n\t" /* ASI_LEON23_DCACHE_MISS */
	"orcc	%%g2, 0x0, %%g0\n\t"
	"bne,a	2f\n\t"
	" ldub	[%0], %%g2\n\t"
	".subsection	2\n"
	"2:\n\t"
	"orcc	%%g2, 0x0, %%g0\n\t"
	"bne,a	2b\n\t"
	" ldub	[%0], %%g2\n\t"
	"b,a	1b\n\t"
	".previous\n"
	: /* no outputs */
	: "r" (lock)
	: "g2", "memory", "cc");
}

static inline int __raw_spin_trylock(raw_spinlock_t *lock)
{
	unsigned int result;
	__asm__ __volatile__("ldstuba [%1]1, %0" /* ASI_LEON23_DCACHE_MISS */
			     : "=r" (result)
			     : "r" (lock)
			     : "memory");
	return (result == 0);
}

static inline void __raw_spin_unlock(raw_spinlock_t *lock)
{
	__asm__ __volatile__(
	B2BSTORE_FIX_STR
	B2BSTORE_FIX_STR
	"stb %%g0, [%0]\n\t"
	B2BSTORE_FIX_STR
	B2BSTORE_FIX_STR
	:
	: "r" (lock)
	: "memory");
}



#endif /* _INCLUDE_LEONSPINLOCK_h */
/* end of include file */

