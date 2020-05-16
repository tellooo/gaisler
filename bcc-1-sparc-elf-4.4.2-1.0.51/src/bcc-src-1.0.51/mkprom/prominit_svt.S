/************************************************************************/
/*   This file is a part of the mkprom3 boot-prom utility               */
/*   Copyright (C) 2004 Gaisler Research                                */
/*                                                                      */
/*   This library is free software; you can redistribute it and/or      */
/*   modify it under the terms of the GNU General Public                */
/*   License as published by the Free Software Foundation; either       */
/*   version 2 of the License, or (at your option) any later version.   */
/*                                                                      */
/*   See the file COPYING.GPL for the full details of the license.      */
/************************************************************************/

	
/*
 * this will override the weak hardreset_custom in libgloss's locore.s
 */

/* Entry for traps which jump to a programmer-specified trap handler.  */
#ifndef _SOFT_FLOAT
#define PSR_INIT 0x10e0
#else
#define PSR_INIT 0x00e0
#endif
#define TRAPR(H)  wr %g0, PSR_INIT, %psr; mov %g0, %tbr; ba (H); mov %g0, %wim;
#define TRAP(H)  mov %psr, %l0; sethi %hi(H), %l4; jmp %l4+%lo(H); nop;

/* Unexcpected trap will halt the processor by forcing it to error state */
#define BAD_TRAP ta 0; nop; nop; nop;

/* Number of register windows */
#define NWINDOWS 	8

#define WIM_INIT   2
	
#define TT_MASK         0xff0   // trap type mask from tbr
#define TT_SHL          4       // shift to get a tbr value

	
  .seg    "text"
  .global _start_svt_weak

_start_svt_weak:
	/******************/
	/* initialize %tbr and %psr for vhdl simulation */
	rd	%psr, %g6
	rd	%tbr, %g7
	srl     %g7,TT_SHL,%g7
	andcc	%g7,(TT_MASK>>TT_SHL),%g0
	bne	1f
	 nop

	/* hardreset */
	set     PSR_INIT, %g6
	wr	%g0, %tbr
	 
1:	wr	%g6, %psr
	nop; nop; nop
	/******************/
	
	sethi %hi(_start_svt_real), %g6
	jmp %g6+%lo(_start_svt_real)
	 nop	

_set_svt_trap_table_prom:
	set	_start_svt_real,%o0
	sub	%sp,0x1000,%o1
	set	TT_MASK,%g1
	andn	%o1,%g1,%o1
	sub	%o1, 104, %sp
	wr	%o1,%tbr	
	
/*	
 void set(unsigned int h, unsigned int *addr )	 {
     addr[0] = ((h >> 10) & 0x3fffff) | 0x29000000; \/* 29000000: sethi	%hi(handler), %l4       *\/
     addr[1] = ((h) &          0x3ff) | 0x81c52000; \/* 81c52000: jmpl	%l4 + %lo(handler), %g0 *\/
     addr[2] = 0x01000000;                          \/* 01000000: nop *\/
     addr[3] = 0x01000000;                          \/* 01000000: nop *\/
 }
*/
		
	and  %o0, 0x3ff, %o4
	sethi  %hi(0x29000000), %g1
	srl  %o0, 0xa, %o0
	or  %o0, %g1, %o0
	sethi  %hi(0x81c52000), %o5
	sethi  %hi(0x1000000), %g1
	or  %o4, %o5, %o4
	st  %g1, [ %o1 + 0xc ]
	st  %o0, [ %o1 ]
	st  %o4, [ %o1 + 4 ]
        st  %g1, [ %o1 + 8 ]		
	retl
	 flush
	





