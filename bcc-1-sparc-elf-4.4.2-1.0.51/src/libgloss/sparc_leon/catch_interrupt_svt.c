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

/* single vector trapping version of trap handler installs */
struct svt_trap_entry {
  int start,end,func;
};
extern struct svt_trap_entry trap_table[28];
extern struct svt_trap_entry svt_trap_table_ext[17];
extern struct svt_trap_entry svt_trap_table_ext_end;

static struct svt_trap_entry *gettrap_pos(int nr) {
    struct svt_trap_entry *p = trap_table;
    while((p->start) || (p->end) || (p->func)) {
	if (p->start <= nr && p->end >= nr) {
	    break;
	}
	p++;
    }
    return p;
}

int svtloleveltrapinstall(int trap,void (*handler)()){
    struct svt_trap_entry *p = gettrap_pos(trap);
    if (p >= &svt_trap_table_ext_end) {
	return 0;
    }
    p->start = trap;
    p->end = trap;
    p->func = handler;
    return 1;
}

int svtlolevelirqinstall(int irqnr,void (*handler)()){
    if (irqnr == 0 || irqnr >= 15) {
	return 0;
    }
    return svtloleveltrapinstall(irqnr+0x10,handler);
}


