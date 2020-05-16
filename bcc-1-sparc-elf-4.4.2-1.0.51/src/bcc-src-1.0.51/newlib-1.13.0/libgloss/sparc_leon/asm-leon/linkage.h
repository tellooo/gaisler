#ifndef H_LEONBARE_LINKAGE_H
#define H_LEONBARE_LINKAGE_H

#ifndef _ASM	
# define __inline__	__inline__	__attribute__((always_inline))

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#define barrier()       __memory_barrier()

#define gccalign8 __attribute__((aligned(8)))

#else /* !_ASM */

#define	MCOUNT_SIZE	0	/* no instructions inserted */
#define	MCOUNT(x)

/*
 * ENTRY provides the standard procedure entry code and an easy way to
 * insert the calls to mcount for profiling. ENTRY_NP is identical, but
 * never calls mcount.
 */
#define	ENTRY(x) \
	.section	".text"; \
	.align	4; \
	.global	x; \
	.type	x, #function; \
x:	MCOUNT(x)

#define	ENTRY_SIZE	MCOUNT_SIZE

#endif /* _ASM */

#endif
