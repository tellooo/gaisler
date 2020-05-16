#ifndef	_MACHTIME_H_
#define	_MACHTIME_H_

#if 0
#if defined(__rtems__) || defined(__VISIUM__) || defined(__leon__)
#define _CLOCKS_PER_SEC_ 1000000
#elif defined(__aarch64__) || defined(__arm__) || defined(__thumb__)
#define _CLOCKS_PER_SEC_ 100
#endif
#endif

/* LEON/BCC */
#define _CLOCKS_PER_SEC_ 1000000

#ifdef __SPU__
#include <sys/_timespec.h>
int nanosleep (const struct timespec *, struct timespec *);
#endif

#endif	/* _MACHTIME_H_ */
