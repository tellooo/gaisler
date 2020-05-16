#ifndef _ASMSPARC_CLOCK_PARAM_H
#define _ASMSPARC_CLOCK_PARAM_H

#include <asm-leon/param.h>

#ifndef __ASSEMBLER__
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv , const struct timezone *tz);
void do_gettimeofday(struct timeval *tv);
#endif

#define USEC_PER_SEC (1000000L)
#define NSEC_PER_SEC (1000000000L)
#define NSEC_PER_USEC (1000L)

extern unsigned long tick_nsec;
extern unsigned long tick_usec;

#endif
