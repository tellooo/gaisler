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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <asm-leon/elfmacro.h>
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/irq.h>
#include <asm-leon/jiffies.h>
#include <asm-leon/param.h>
#include <asm-leon/leoncompat.h>
	       
volatile struct timespec xtime __attribute__ ((aligned (16)));
volatile u64 jiffies_64 = INITIAL_JIFFIES;

