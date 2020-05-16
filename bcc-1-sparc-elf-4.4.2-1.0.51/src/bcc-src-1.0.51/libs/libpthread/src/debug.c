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

  @(#)disp.c	3.14 11/8/00

*/

#include "internals.h"
#include "setjmp.h"
#include <fsu_pthread/debug.h>

void pthread_mutex_dbgstart_np(pthread_mutex_t *m) {
    TAILQ_INSERT_TAIL(&dbglist_mutex, m, dbglist);
}

void pthread_cond_dbgstart_np(pthread_cond_t *c) {
    TAILQ_INSERT_TAIL(&dbglist_cond, c, dbglist);
}

void pthread_mutex_dbgstop_np(pthread_mutex_t *m) {
    TAILQ_REMOVE(&dbglist_mutex, m, dbglist);
}

void pthread_cond_dbgstop_np(pthread_cond_t *c) {
    TAILQ_REMOVE(&dbglist_cond, c, dbglist);
}


