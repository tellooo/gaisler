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
   
	.text
	/* ------- */
  	.weak	cpuinit
	.set	cpuinit,_cpuinit
	/* ------- */
	!.global cpuinit
_cpuinit:

        mov %o7,%g6

	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 0xf, %g5

1:      set 0x01, %o0              !VENDOR_GAISLER

2:	mov %g6,%o7
        retl
	nop

        ! force link of jiffies_64
        .global jiffies_64
3:      set jiffies_64,%g1


	.text
