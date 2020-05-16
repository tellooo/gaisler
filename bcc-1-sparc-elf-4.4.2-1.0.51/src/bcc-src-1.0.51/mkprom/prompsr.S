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

/* Entry for traps which jump to a programmer-specified trap handler.  */
#ifndef _SOFT_FLOAT
#define PSR_INIT 0x10e0
#else
#define PSR_INIT 0x00e0
#endif
	
  .seg    "text"
  .global _hardreset_mvt, _hardreset_real, _cleanregs_custom_weak, _start_svt_weak
  .global _start_svt_real
  .global _cleanregs_libgloss

_hardreset_mvt:
	wr	%g0, PSR_INIT, %psr       /* init psr so that vhdl model don't failt when accessing locals */
	sethi %hi(_hardreset_real), %g4
	jmp %g4+%lo(_hardreset_real)
	 nop

_cleanregs_custom_weak:                   
	sethi %hi(_cleanregs_libgloss), %g4
	jmp %g4+%lo(_cleanregs_libgloss)
	 nop

	/* this asumes that %g6 is never used in the code */
_start_svt_weak:
	rd	%tbr, %g6
	sll	%g6,20,%g6
	srl	%g6,24,%g6
	mov	%psr, %l3
	cmp	%g0,%g6
	bne,a	2f
	mov	%l3, %psr

	wr	%g0, PSR_INIT, %psr       /* init psr so that vhdl model don't failt when accessing locals */
	 nop
	
	/* continue normally */
2:	sethi %hi(_start_svt_real), %g6
	jmp %g6+%lo(_start_svt_real)
	 nop
	





