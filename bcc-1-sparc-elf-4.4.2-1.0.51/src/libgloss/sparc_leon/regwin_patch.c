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

#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/asmmacro.h>

extern volatile unsigned int __window_overflow_rettseq[3];
extern volatile unsigned int __window_overflow_slow1[2];

static unsigned int installed = 0;
static unsigned int save__window_overflow_rettseq[3];

int install_winoverflow_hook(void (*func)(void)) {
    if (installed) {
	return 0;
    }
    if (!installed) {
	/*
	  a7 50 00 00 	rd  %wim, %l3
	  ae 10 00 01 	mov  %g1, %l7
	  83 34 e0 01 	srl  %l3, 1, %g1
	  
	  81 c4 40 00 	jmp  %l1
	  81 cc 80 00 	rett  %l2 */
	save__window_overflow_rettseq[0] = __window_overflow_rettseq[0];
	save__window_overflow_rettseq[1] = __window_overflow_rettseq[1];
	save__window_overflow_rettseq[2] = __window_overflow_rettseq[2];
	
	/*29 10 00 31 	sethi  %hi(0x4000c400), %l4
	  81 c5 22 48 	jmp  %l4 + 0x248
	  01 00 00 00 	nop  */
	
	__window_overflow_rettseq[0] = ((((unsigned int)func)>>10) &0x3fffff) | 0x29000000; /* upper 22 */
	__window_overflow_rettseq[1] = ((((unsigned int)func)    ) &0x03ff  ) | 0x81c52000; /* lower 10 */
	__window_overflow_rettseq[2] = 0x01000000; /* nop */
	
	sparc_leon23_icache_flush();
	installed = 1;
    }
    return 0;
}

void uninstall_winoverflow_hook() {
    if (installed) {
	__window_overflow_rettseq[0] = save__window_overflow_rettseq[0];
	__window_overflow_rettseq[1] = save__window_overflow_rettseq[1];
	__window_overflow_rettseq[2] = save__window_overflow_rettseq[2];
	
	sparc_leon23_icache_flush();
	installed = 0;
    }
}
