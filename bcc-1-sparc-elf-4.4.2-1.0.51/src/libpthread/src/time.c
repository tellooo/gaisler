#include "internals.h"
#include "setjmp.h"
#include <fsu_pthread/asm.h>
#include <fsu_pthread/debug.h>
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/irq.h>
#include <asm-leon/jiffies.h>

/* struct timespec xtime __attribute__ ((aligned (16))); */
/* u64 jiffies_64 = INITIAL_JIFFIES; */
/* unsigned long wall_jiffies = INITIAL_JIFFIES; */
/* unsigned long tick_nsec = TICK_NSEC; */
/* unsigned long tick_usec = TICK_NSEC/1000; */

/* void do_timer(struct pt_regs *regs) { */
/*   unsigned long ticks; */
/*   jiffies_64++; */
/*   ticks = jiffies - wall_jiffies; */
/*   if (ticks) { */
/*     wall_jiffies += ticks; */
/*     do { */
/*       ticks--; */
/*       xtime.tv_nsec += tick_nsec; */
/*       if (xtime.tv_nsec >= 1000000000) { */
/*         xtime.tv_nsec -= 1000000000; */
/*         xtime.tv_sec++; */
/*       } */
/*     } while (ticks); */
/*   } */
/* } */

/* /\* get usec (timeval)  resolution, */
/*  * could use nsec (timespec) because pthread use it  (todo) *\/ */
/* void do_gettimeofday(struct timeval *tv) { */
  
/*   unsigned long flags; */
/*   unsigned long seq; */
/*   unsigned long usec, sec; */
  
/*   do { */
/*     unsigned long lost; */
/*     seq = jiffies; */
/*     usec = ((LEON3_GpTimer_Regs ->e[0].rld&0x7fffff) -  */
/*             (LEON3_GpTimer_Regs ->e[0].val&0x7fffff)); */
/*     lost = jiffies - wall_jiffies; */
    
/*     if (unlikely(lost)) { */
/*       usec += lost * tick_usec; */
/*     } */
    
/*     sec = xtime.tv_sec; */
/*     usec += (xtime.tv_nsec / 1000); */
/*   } while (seq != jiffies); */

/*   while (usec >= 1000000) { */
/*     usec -= 1000000; */
/*     sec++; */
/*   } */
  
/*   tv->tv_sec = sec; */
/*   tv->tv_usec = usec; */
/*   PDEBUG(PDBG_TIMER,"Gettime: %x,%x",tv->tv_sec,tv->tv_usec); */
/* } */

/* int gettimeofday(struct timeval *tv, struct timezone *tz) { */
/*   do_gettimeofday(tv); */
/*   return 0; */
/* } */


