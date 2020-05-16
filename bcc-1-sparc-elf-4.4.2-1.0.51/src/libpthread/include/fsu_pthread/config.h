/* Copyright (C) 1992, 1993, 1994, 1995, 1996 the Florida State University
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
  Gaisler Research, Konrad Eisele<eiselekd@web.de>

  %@(#)config_header.c	3.14%11/8/00%
*/

/*
 * configuration header file to identify compile options
 */

#ifndef __leonbare__
#define __leonbare__
#endif

#ifndef SRP_NP
#define SRP_NP
#endif
#ifndef SRP
#define SRP
#endif


/* #ifndef DEF_RR_NP */
/* #define DEF_RR_NP */
/* #endif */
/* #ifndef DEF_RR */
/* #define DEF_RR         */
/* #endif */

#ifndef RR_SWITCH_NP
#define RR_SWITCH_NP
#endif
#ifndef RR_SWITCH
#define RR_SWITCH
#endif

#ifndef STAND_ALONE_NP
#define STAND_ALONE_NP
#endif
#ifndef STAND_ALONE
#define STAND_ALONE        
#endif

#ifndef CLOCK_REALTIME_NP
#define CLOCK_REALTIME_NP 1
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 1
#endif

#ifndef REAL_TIME_NP
#define REAL_TIME_NP 1
#endif
#ifndef REAL_TIME
#define REAL_TIME 1
#endif

#ifndef ASM_SETJMP_NP
#define ASM_SETJMP_NP
#endif
#ifndef ASM_SETJMP
#define ASM_SETJMP
#endif

#ifndef C_CONTEXT_SWITCH_NP
#define C_CONTEXT_SWITCH_NP
#endif
#ifndef C_CONTEXT_SWITCH
#define C_CONTEXT_SWITCH
#endif

/* /\*#ifndef IO_NP */
/*   #define IO_NP */
/*   #endif*\/ */

/* #ifndef C_INTERFACE_NP */
/* #define C_INTERFACE_NP */
/* #endif */

/* /\*#ifndef SRP_NP */
/*   #define SRP_NP */
/*   #endif*\/ */

/* #ifndef AUTO_INIT_NP */
/* #define AUTO_INIT_NP */
/* #endif */

/* #ifndef _POSIX */
/* #define _POSIX */
/* #endif */

/* #ifndef CLEANUP_HEAP_NP */
/* #define CLEANUP_HEAP_NP */
/* #endif */

/* #ifndef C_CONTEXT_SWITCH_NP */
/* #define C_CONTEXT_SWITCH_NP */
/* #endif */

/* #ifndef RELEASE_NP */
/* #define RELEASE_NP 2426 */
/* #endif */

#ifndef _M_UNIX
#if defined(M_UNIX) || defined(__M_UNIX)
#define _M_UNIX
#endif
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(_M_UNIX)
#define SIGACTION_CONST const
#define SIGSUSPEND_CONST const
#define SIGPROCMASK_CONST const
#define SIGWAIT_CONST const
#define LONGJMP_CONST 
#define SIGLONGJMP_CONST 
#define READV_CONST const
#define WRITEV_CONST const
#endif /* defined(__linux__) || defined(__FreeBSD__) || defined(_M_UNIX) */
