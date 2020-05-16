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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int leon3_ahbslv_scan(register unsigned int vendor,register unsigned int driver) {
  register unsigned int conf,i, *confp;
  register unsigned int cfg_area = (unsigned int ) (LEON3_IO_AREA | LEON3_CONF_AREA | LEON3_AHB_SLAVE_CONF_AREA);
  for (i = 0; i < LEON3_AHB_SLAVES; i++)  {
    confp = (unsigned int*)(cfg_area + (i * LEON3_AHB_CONF_WORDS * 4));
    conf = *confp;
    if ((amba_vendor(conf) == vendor) && (amba_device(conf) == driver)) {
      return (unsigned int)confp;
    }
  }
  return 0;
}

unsigned int leon3_getbase(register unsigned int *mbar,register unsigned int iobase, int *irq) {
  register unsigned int conf = mbar[1];
  return (unsigned int )(((iobase & 0xfff00000) |
          ((conf & 0xfff00000)>> 12)) & (((conf & 0x0000fff0) <<4) | 0xfff00000));
}

unsigned int leon3_apbslv_scan(register unsigned int base,
				register unsigned int vendor, 
				register unsigned int driver, 
				amba_apb_device *apbdevs, int c) {
  register unsigned int conf, i, *confp;int j = 0;
  for (i = 0; i < LEON3_APB_SLAVES; i++) {
    confp = (unsigned int*)(base + (i * LEON3_APB_CONF_WORDS * 4));
    conf = *confp;
    if ((amba_vendor(conf) == vendor) && (amba_device(conf) == driver)) {
      if (j < c) {
	apbdevs[j].start = leon3_getbase(confp,base,0);
        apbdevs[j].irq = amba_irq(conf);
	j++;
      }
    }
  }
  return j;
}


unsigned int leon3_getapbbase(register unsigned int vendor,
			      register unsigned int driver, 
			      amba_apb_device *apbdevs, int c) {
    unsigned int apb = leon3_ahbslv_scan(VENDOR_GAISLER,GAISLER_APBMST);
    apb = (*(unsigned int *)(apb + 16)) & LEON3_IO_AREA;
    apb |= LEON3_CONF_AREA;
    return leon3_apbslv_scan(apb,vendor,driver,apbdevs,c);
}

    
