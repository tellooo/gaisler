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

  @(#)signal.c	3.14 11/8/00

*/

#include "internals.h"
#include <fsu_pthread/debug.h>
#include <time.h>
#include <asm-leon/clock.h>
#include <asm-leon/time.h>
#include <asm-leon/jiffies.h>

/*------------------------------------------------------------*/
/*
 * clock_gettime - reads the clock
 */
int clock_gettime(int clock_id, struct timespec *tp) {
  struct timeval now;
  PTRACEIN;
  
  if (clock_id != CLOCK_REALTIME || !tp ) {
    set_errno(EINVAL);
    return(-1);
  }
  do_gettimeofday(&now);
  
  U2P_TIME((*tp), now);
  return(0);
}

int clock_settime(int  clock_id, struct timespec *tp ) {
  struct tm         split_time;
  
  switch ( clock_id ) {
  case CLOCK_REALTIME: 
    {
      struct timeval tv;
      tv.tv_sec = tp->tv_sec;
      tv.tv_usec = tp->tv_nsec / 1000;
      if (settimeofday(&tv , 0)) {
	set_errno(EINVAL);
	return -1;
      }
      break;
    }
    
  default:
    set_errno(EINVAL);
    return -1;
  }
  return 0;
}

int clock_getres( int clock_id,  struct timespec *res ) {

  if (!res) {
    set_errno(EINVAL);
    return -1;
  }
  
  switch ( clock_id ) {
  case CLOCK_REALTIME:
    if ( res ) {
      res->tv_sec = tick_nsec / NSEC_PER_SEC;
      res->tv_nsec = tick_nsec;
    }
    break;
  default:
    set_errno(EINVAL);
    return -1;
  }
  return 0;
}
