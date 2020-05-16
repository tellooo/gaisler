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
        
        .seg    "data"
	.global sparc_leon23_cache_flush, sparc_leon23_icache_flush, sparc_leon23_dcache_flush
	
	.global _leon_version
	
	.seg    "text"

/* =============================================== */
	
/* use only %o7 */	
sparc_leon23_icache_flush:
sparc_leon3_icache_flush:
	sta %g0, [%g0] ASI_LEON3_IFLUSH
	retl
	 nop
sparc_leon23_cache_flush:	
sparc_leon3_cache_flush:
	sta %g0, [%g0] ASI_LEON3_IFLUSH
	sta %g0, [%g0] ASI_LEON3_DFLUSH
	retl
	 nop
sparc_leon23_dcache_flush:	
sparc_leon3_dcache_flush:
	sta %g0, [%g0] ASI_LEON3_DFLUSH
	retl
	 nop

