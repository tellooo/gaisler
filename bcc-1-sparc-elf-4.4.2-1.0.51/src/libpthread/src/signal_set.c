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
#include "setjmp.h"
#include "offsets.h"

/*------------------------------------------------------------*/
/*
 * pthread_sigcpyset2set - add a signal set to another one
 */
void pthread_sigcpyset2set(dst, src)
int *dst, *src;
{
  int i;

  for (i = 0; i < PTHREAD_SIGSET2SET_SIZE_NP; i += sizeof(int))
    *(dst++) = *(src++);
}

/*------------------------------------------------------------*/
/*
 * pthread_sigaddset2set - add a signal set to another one
 */
void pthread_sigaddset2set(dst, src)
int *dst, *src;
{
  int i;

  for (i = 0; i < PTHREAD_SIGSET2SET_SIZE_NP; i += sizeof(int))
    *(dst++) |= *(src++);
}

/*------------------------------------------------------------*/
/*
 * pthread_sigdelset2set - delete a signal set from another one
 */
void pthread_sigdelset2set(dst, src)
int *dst, *src;
{
  int i;

  for (i = 0; i < PTHREAD_SIGSET2SET_SIZE_NP; i += sizeof(int))
    *(dst++) &= ~(*(src++));
}

/*------------------------------------------------------------*/
/*
 * pthread_sigismemberset2set - check if two sets overlap
 */
int pthread_sigismemberset2set(dst, src)
int *dst, *src;
{
  int i;

  for (i = 0; i < PTHREAD_SIGSET2SET_SIZE_NP; i += sizeof(int))
    if (*(dst++) & *(src++))
      return(TRUE);

  return(FALSE);
}

/*------------------------------------------------------------*/
/*
 * pthread_signonemptyset - check if a set has any bits set
 */
int pthread_signonemptyset(set)
int *set;
{
  int i;

  for (i = 0; i < PTHREAD_SIGSET2SET_SIZE_NP; i += sizeof(int))
    if (*(set++))
      return(TRUE);

  return(FALSE);
}

/*------------------------------------------------------------*/
/*
 * pthread_siggeset2set - check if dst set is >= src set, i.e.
 * dst has at least the bits set which src has set
 */
int pthread_siggeset2set(dst, src)
int *dst, *src;
{
  int i;

  for (i = 0; i < PTHREAD_SIGSET2SET_SIZE_NP; i += sizeof(int))
    if ((*(dst++) & *(src)) != *(src))
      return(FALSE);
    else
      src++;

  return(TRUE);
}

/*------------------------------------------------------------*/
/*
 * pthread_sigeqset2set - check if dst set is == src set
 */
int pthread_sigeqset2set(dst, src)
int *dst, *src;
{
  int i;

  for (i = 0; i < PTHREAD_SIGSET2SET_SIZE_NP; i += sizeof(int))
    if (*(dst++) != *(src++))
      return(FALSE);

  return(TRUE);
}

