/* Target-dependent code for NetBSD/sparc.

   Copyright (C) 2002-2018 Free Software Foundation, Inc.
   Contributed by Wasabi Systems, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "frame.h"
#include "frame-unwind.h"
#include "gdbcore.h"
#include "gdbtypes.h"
#include "osabi.h"
#include "regcache.h"
#include "regset.h"
#include "symtab.h"
#include "trad-frame.h"

#include "sparc-tdep.h"

#define _initialize_sparc_tdep disable_sparc_tdep
#include "sparc-tdep.c"

static struct gdbarch *
leon_gdbarch_init(struct gdbarch_info info, struct gdbarch_list *arches)
{
	  struct gdbarch *gdbarch = sparc32_gdbarch_init(info, arches);
	  if (gdbarch) {
		  set_gdbarch_long_double_bit (gdbarch, 64);
		  set_gdbarch_long_double_format (gdbarch, floatformats_ieee_double);

		  // LEON has HW single-step support
		  // set_gdbarch_software_single_step (gdbarch, NULL);
	  }
	  return gdbarch;
}

void
_initialize_sparc_leon_tdep (void)
{
  register_gdbarch_init (bfd_arch_sparc, leon_gdbarch_init);
}

