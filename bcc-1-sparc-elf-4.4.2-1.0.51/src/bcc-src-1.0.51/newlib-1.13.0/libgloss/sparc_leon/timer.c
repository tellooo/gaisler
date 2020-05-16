/*

    LEON2/3 LIBIO low-level routines 
    Written by Jiri Gaisler.
    Copyright (C) 2004  Gaisler Research AB

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/timer.h>
#include <asm-leon/leoncompat.h>

// '''''''''''''''''''''''''''''''''''''''''''''''''''''

TAILQ_HEAD(timer_queue,timerevent) timers = TAILQ_HEAD_INITIALIZER(timers); 

int addtimer(struct timerevent *e) {
  struct timerevent *next;
  unsigned long old = leonbare_disable_traps();
  TAILQ_FOREACH(next, &timers, n) {
    if (!GT_TIMESPEC(e->expire, next->expire)) 
      break;
  }
  if (next) { 
    TAILQ_INSERT_BEFORE(next, e, n);
  } else {
    TAILQ_INSERT_TAIL(&timers, e, n);
  }
  leonbare_enable_traps(old);
}

extern unsigned long noalarm;
void settimer() {
  struct timeval tv,te;
  struct timerevent *e = TAILQ_FIRST(&timers), *n;
  while (e) {
    n = TAILQ_NEXT(e, n);
    te.tv_sec = e->expire.tv_sec;
    te.tv_usec = e->expire.tv_nsec / NSEC_PER_USEC;
    do_gettimeofday(&tv);
    if (GT_TIMEVAL(te,tv)) {
      MINUS_TIMEVAL(te,te,tv);
      if (!tv.tv_sec ||  te.tv_usec <= tick_usec) {
	if (!noalarm) {
	  //---------------------
	  switch (LEONCOMPAT_VERSION) {
	  case 3:
	  default:
	    LEON3_GpTimer_Regs->e[1].val = 0;
	    LEON3_GpTimer_Regs->e[1].rld = te.tv_usec-1;
	    LEON3_GpTimer_Regs->e[1].ctrl = 0;
	    LEON3_GpTimer_Regs->e[1].ctrl = 
	      LEON3_GPTIMER_EN |
	      LEON3_GPTIMER_LD | LEON3_GPTIMER_IRQEN;
	    break;
	  }
	}
	//---------------------
      }
    } else {
      unsigned long old = leonbare_disable_traps();
      TAILQ_REMOVE(&timers,e,n);
      e->handler(e->arg);
      leonbare_enable_traps(old);
    }
    e = n;
  }
}

int Timer_getTimer1(unsigned int **count,unsigned int **reload,unsigned int **ctrl) 
{
  //---------------------
  switch (LEONCOMPAT_VERSION) {
  case 3:
  default:
    amba_init();
    *count =(unsigned int *) &(LEON3_GpTimer_Regs->e[0].val);
    *reload = (unsigned int *)&(LEON3_GpTimer_Regs->e[0].rld);
    *ctrl = (unsigned int *)&(LEON3_GpTimer_Regs->e[0].ctrl);
    break;
  }
  //---------------------
  return 1;
}

int Timer_getTimer2(unsigned int **count,unsigned int **reload,unsigned int **ctrl)
{
  //---------------------
  switch (LEONCOMPAT_VERSION) {
  case 3:
  default:
    amba_init();
    if (!noalarm) {
      *count = (unsigned int *)&(LEON3_GpTimer_Regs->e[1].val);
      *reload = (unsigned int *)&(LEON3_GpTimer_Regs->e[1].rld);
      *ctrl = (unsigned int *)&(LEON3_GpTimer_Regs->e[1].ctrl);
      break;
    }
    return 0;
  }
  //---------------------
  return 1;
}

