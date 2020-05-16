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

#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>
#define NULL 0

TAILQ_HEAD(pending_queue,pendingaction) pending = TAILQ_HEAD_INITIALIZER(pending); 

void add_pending(struct pendingaction *a) {
  unsigned long old = leonbare_disable_traps();
  TAILQ_INSERT_TAIL(&pending,a,next);
  leonbare_enable_traps(old);
}

struct pendingaction *get_pending() {
  struct pendingaction *a = 0;
  unsigned long old = leonbare_disable_traps();
  if (a = TAILQ_FIRST(&pending)) {
    TAILQ_REMOVE(&pending,a,next);
  }
  leonbare_enable_traps(old);
  return a;
}

void process_pending() {
  struct pendingaction *a;
  while(a = get_pending()) {
    if (a->handler) {
      a->handler(a->arg);
    }
  }
}


