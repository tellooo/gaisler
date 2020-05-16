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
  Gaisler Research, Konrad Eisele<eiselekd@web.de>

  @(#)unistd.h	3.14 11/8/00

*/

#ifndef _pthread_unistd_h
#define _pthread_unistd_h


#ifndef _POSIX_THREADS
#define	_POSIX_THREADS
#endif /* !_POSIX_THREADS */

#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
#define	_POSIX_THREAD_PRIORITY_SCHEDULING
#endif /* !_POSIX_THREAD_PRIORITY_SCHEDULING */

#ifndef _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_ATTR_STACKSIZE
#endif /* !_POSIX_THREAD_ATTR_STACKSIZE */

#ifndef _POSIX_THREAD_ATTR_CUSTOM
#define _POSIX_THREAD_ATTR_CUSTOM
#endif /* !_POSIX_THREAD_ATTR_CUSTOM */

#ifdef SRP
# ifndef _POSIX_THREADS_PRIO_PROTECT
#  define _POSIX_THREADS_PRIO_PROTECT
# endif /* !_POSIX_THREADS_PRIO_PROTECT */
#else /* !SRP */
# undef _POSIX_THREADS_PRIO_PROTECT
#endif /* !SRP */

#undef _POSIX_THREADS_PROCESS_SHARED
#undef _POSIX_THREADS_PRIO_INHERIT
#undef _POSIX_REENTRANT_FUNCTIONS

#endif /* !_pthread_unistd_h */
