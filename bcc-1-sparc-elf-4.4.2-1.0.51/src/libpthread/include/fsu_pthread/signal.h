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

  pthreads-bugs@ada.cs.fsu.edu,
  Gaisler Research, Konrad Eisele<eiselekd@web.de>
  @(#)signal.h	3.14 11/8/00

*/

#ifndef _pthread_signal_h
#define _pthread_signal_h

#ifndef	__signal_h

#ifdef LOCORE
#undef LOCORE
#endif

#include "stdtypes.h"
#include <signal.h>

#ifndef SIGMAX
#define SIGMAX _NSIG
#endif

#ifndef NSIG
#define NSIG SIGMAX
#endif

#ifdef __SIGRTMIN
#define NNSIG     __SIGRTMIN+1
#else
#define NNSIG     NSIG+1
#endif

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0

#if defined(__FreeBSD__)
#include <sys/time.h>
#elif defined(__dos__)
#include <time.h>
#endif

#define ts_sec tv_sec
#define ts_nsec tv_nsec

#ifndef TIMEVAL_TO_TIMESPEC
#if !defined(__linux__) || !defined(_ASMi386_SIGCONTEXT_H)
#if !defined(__SI_MAX_SIZE) && !defined(_STRUCT_TIMESPEC)
struct timespec {
  time_t tv_sec;
  long   tv_nsec;
};
#endif
#endif /* !defined(__linux) || !defined(SIGCLD) */
#endif /* !TIMEVAL_TO_TIMESPEC */
#else /* CLOCK_REALTIME */
#define ts_sec tv_sec
#define ts_nsec tv_nsec
#endif /* CLOCK_REALTIME */

#define PTHREAD_CANCEL_ENABLE       SIG_UNBLOCK
#define PTHREAD_CANCEL_DISABLE      SIG_BLOCK
#define PTHREAD_CANCEL_DEFERRED   0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

#ifdef si_value
#undef si_value
#endif

union p_sigval {
  int sival_int;
  void *sival_ptr;
};

struct p_siginfo {
  int si_signo;
  int si_code;
  union p_sigval si_value;
};

/* Signal Actions, P1003.1b-1993, p. 64 */
/* si_code values, p. 66 */

#define SI_USER    1    /* Sent by a user. kill(), abort(), etc */
#define SI_QUEUE   2    /* Sent by sigqueue() */
#define SI_TIMER   3    /* Sent by expiration of a timer_settime() timer */
#define SI_ASYNCIO 4    /* Indicates completion of asycnhronous IO */
#define SI_MESGQ   5    /* Indicates arrival of a message at an empty queue */


/*
 * This defines the implementation-dependent context structure provided
 * as the third parameter to user handlers installed by sigaction().
 * It should be a copy of the first part of the BSD sigcontext structure.
 * The second half should not be accessed since it is only present if
 * a _sigtramp instance is present right below the user handler on the
 * thread's stack. For SVR4, we will have to build this structure from scratch.
 */

#if defined(__leonbare__)

#define context_t sigcontext
#define sc_sp   sigc_sp // fix:
//#define sc_fp   si_regs.fp // fix:
#define sc_pc   sigc_pc
#define sc_npc  sigc_npc
#define sc_ps   sigc_psr
#define sc_mask sigc_mask
#define sc_o0   sigc_o0

#define p_sigval p_sigval
#define siginfo p_siginfo
#define FC_PROT 1
#ifndef __SI_MAX_SIZE
typedef struct siginfo siginfo_t;
#endif

#undef sigmask
#undef sigemptyset
#undef sigfillset
#undef sigaddset
#undef sigdelset
#undef sigismember

#define sigmask(n)              ((unsigned int)1 << ((n) - 1))
#define sigemptyset(set)        (*(set) = (sigset_t) 0)
#define sigfillset(set)         (*(set) = (sigset_t) -1)
#define sigaddset(set, signo)   (*(set) |= sigmask(signo))
#define sigdelset(set, signo)   (*(set) &= ~sigmask(signo))
#define sigismember(set, signo) (*(set) & sigmask(signo))

#endif /* __leonbare__ */

#ifndef SA_SIGINFO
#define SA_SIGINFO 0
#endif

#ifndef SA_ONESHOT
#define SA_ONESHOT 0
#endif

#ifndef SA_NOMASK
#define SA_NOMASK 0
#endif

#ifndef SA_ONSTACK
#define SA_ONSTACK SV_ONSTACK
#endif /* !SA_ONSTACK */

#ifndef BUS_OBJERR
#define BUS_OBJERR FC_OBJERR
#endif /* !BUS_OBJERR */

#ifndef BUS_CODE
#define BUS_CODE(x) FC_CODE(x)
#endif

#ifdef __SIGRTMIN
#define SIGCANCEL __SIGRTMIN
#else
#define SIGCANCEL NSIG
#endif

#endif /* __signal_h */

#endif /* !_pthread_signal_h */
