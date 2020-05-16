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


/* multi vector trapping version of trap handler installs */
int mvtlolevelirqinstall(int irqnr,void (*handler)()) {
    unsigned long h = (unsigned long)handler;
    unsigned long *addr = (unsigned long *)((locore_readtbr() & ~0xFFF) + 0x100 + (16 * irqnr));

    if (irqnr == 0 || irqnr >= 15) {
	return 0;
    }
    
    addr[0] = ((h >> 10) & 0x3fffff) | 0x29000000; /* 29000000: sethi	%hi(handler), %l4       */
    addr[1] = ((h) &          0x3ff) | 0x81c52000; /* 81c52000: jmpl	%l4 + %lo(handler), %g0 */
    addr[2] = 0x01000000;                          /* 01000000: nop */
    addr[3] = 0x01000000;                          /* 01000000: nop */
    return 1;
}

int lolevelirqinstall(int irqnr,void (*handler)()) {
    return mvtlolevelirqinstall(irqnr,handler);
}

