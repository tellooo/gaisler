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
/*
 * regwin.s for LEON
 */

#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/asmmacro.h>
        
  .seg    "text"

        ! srmmu trap or data access trap
	/* ------- */
	.weak	_srmmu_fault_svt
	.set	_srmmu_fault_svt,__srmmu_fault_svt
	.weak	_srmmu_fault
	.set	_srmmu_fault,__srmmu_fault
	/* ------- */

/* 1 (inst) or 9 (data) in %l6 */	
__srmmu_fault_svt:
__srmmu_fault:	
	ta 0; nop; nop; nop;
  	jmp  %l1			! Re-execute save.
  	rett %l2
