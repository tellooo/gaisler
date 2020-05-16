#ifndef _ASMSPARC_TIMER_H
#define _ASMSPARC_TIMER_H

#include <sys/fsu_pthread_queue.h>
#include <sys/time.h>
#include <asm-leon/clock.h>

#ifndef __ASSEMBLER__
typedef int (*timerevent_handler)(void *);
struct timerevent  {
  TAILQ_ENTRY(timerevent) n;
  struct timespec expire;
  timerevent_handler handler;
  void *arg;
  
};
#endif

#define GT_TIMESPEC(t1, t2) \
      (t1.tv_sec > t2.tv_sec || \
       (t1.tv_sec == t2.tv_sec && \
	t1.tv_nsec > t2.tv_nsec))

#define GT_TIMEVAL(t1, t2) \
      (t1.tv_sec > t2.tv_sec || \
       (t1.tv_sec == t2.tv_sec && \
	t1.tv_usec > t2.tv_usec))

/*
 * MINUS_TIME only works if src1 > src2
 */
#define MINUS_TIMEVAL(dst, src1, src2) \
    if ((src2).tv_usec > (src1).tv_usec) { \
      (dst).tv_sec = (src1).tv_sec - (src2).tv_sec - 1; \
      (dst).tv_usec = ((src1).tv_usec - (src2).tv_usec) + USEC_PER_SEC; \
    } \
    else { \
      (dst).tv_sec = (src1).tv_sec - (src2).tv_sec; \
      (dst).tv_usec = (src1).tv_usec - (src2).tv_usec; \
    } 
  
/* Protypes */
#ifndef __ASSEMBLER__
void leonbare_init_ticks();
int addtimer(struct timerevent *e);
#endif

#endif
