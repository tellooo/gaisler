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

  @(#)p_aux.S	3.14 11/8/00

*/

/*
 * Auxiliary assembly routines
 *
 * Portability notes:
 * The assembly code in this file is only needed for the following conditional
 * compilation:
 * NOERR_CHECK:                 pthreead_test_and_set
 * SOLARIS && IO:               pthread_get_retaddr
 * STACK_CHECK && SIGNAL_STACK: service routines such as set/get PC, SP, FP etc.
 * always needed:               start_float entry (initialize Pthreads)
 * SOLARIS:			pthread_dummy
 */

#include <fsu_pthread/config.h>
#include <../src/config_internals.h>

#define LOCORE
#define _ASM

#ifdef SOLARIS
#include <sys/asm_linkage.h>
#define NAME(x) x
#else
#if defined(__FreeBSD__) || defined(_M_UNIX) || defined(__linux__) || defined (__dos__)
#else
//#include <sparc/asm_linkage.h>
#include <asm-leon/linkage.h>
#ifdef NAME	
#undef NAME
#endif /* NAME */
#define	IDENTITY(x) x
#define NAME(x) x
#endif
#endif

#include <asm-leon/linkage.h>

#include <fsu_pthread/unistd.h>

#ifdef STAND_ALONE
#ifdef sun4e
#include <sun4e/trap.h>
#elif defined(sun4c)
#include <sun4c/trap.h>
#elif defined(sun4m)
#include <sun4m/trap.h>
#endif
#else /* !STAND_ALONE */
#ifdef SOLARIS
#include <sys/trap.h>
#else /* !SORLARIS */
#if defined(__FreeBSD__) || defined(_M_UNIX) || defined(__linux__) || defined (__dos__)
#else
#include <sparc/trap.h>
#endif
#endif /* !SOLARIS */
#endif /* !STAND_ALONE */

#ifdef NOERR_CHECK
!	int pthread_test_and_set(flag)
!	int *flag;
!
!	executes a test & set instruction on the passed flag and
!	returns the previous value of the flag. (leaf routine)
	ENTRY(pthread_test_and_set)
	retl
	ldstub	[%o0],%o0		! Delay: Flag in o0 (no save/restore)
#endif /* NOERR_CHECK */

#if defined(SOLARIS) && defined(IO)
!	char *pthread_get_retaddr()
!
!	returns the return address (leaf procedure)
	ENTRY(pthread_get_retaddr)
	retl
	mov	%i7, %o0		! Delay: i7 in o0 (no save/restore)
#endif /* SOLARIS && IO */

#if (defined(STACK_CHECK) && defined(SIGNAL_STACK)) || defined(STAND_ALONE)
!	char *pthread_get_sp()
!
!	returns the current sp (leaf procedure)
	ENTRY(pthread_get_sp)
	retl
	mov	%sp, %o0		! Delay: sp in o0 (no save/restore)

!       void pthread_set_sp(new_sp)
!	int *new_sp;
!
!       sets the sp (leaf procedure)
        ENTRY(pthread_set_sp)
        retl
        mov      %o0, %sp             ! Delay: o0 in sp (no save/restore)

!       char *pthread_get_fp()
!
!       returns the current fp (leaf procedure)
        ENTRY(pthread_get_fp)
        retl
        mov      %fp, %o0             ! Delay: fp in o0 (no save/restore)
 
!       void pthread_set_fp(new_fp) 
!	int *new_fp;
!
!       sets the fp (leaf procedure) 
        ENTRY(pthread_set_fp)
        retl 
        mov      %o0, %fp             ! Delay: o0 in fp (no save/restore) 

!	void pthread_ST_FLUSH_WINDOWS()
!
!	Flushes the windows
	ENTRY(pthread_ST_FLUSH_WINDOWS)
	retl
	ta	ST_FLUSH_WINDOWS	! Delay:Flush Windows(no save/restore)
#endif /* (STACK_CHECK && SIGNAL_STACK) || STAND_ALONE */


#ifdef SOLARIS
!	void pthread_dummy()
!
!	The sole purpose of this label is to ensure that the corresponding
!	 object file is always linked. The entry is references
!	(but never used) in pthread.c to ensure just that.
	ENTRY(pthread_dummy)
#endif /* SOLARIS */
