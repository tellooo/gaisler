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

/*#define DEBUG_CONFIG*/

/* Structure containing address to devices found on the Amba Plug&Play bus */
extern amba_confarea_type amba_conf;

#ifdef DEBUG_CONFIG
#define printk(fmt,arg...) \
{ char c[1024]; \
  sprintf(c,fmt,##arg); \
  DEBUG_puts(c); \
}
#else
#define printk(fmt,arg...) 
#endif

static void vendor_dev_string(unsigned long conf, char *vendorbuf, char *devbuf)
{
	int vendor = amba_vendor(conf);
	int dev = amba_device(conf);
	char *devstr;
	char *vendorstr;
#ifdef DEBUG_CONFIG	
	sprintf(vendorbuf, "Unknown vendor %2x", vendor);
	sprintf(devbuf, "Unknown device %2x", dev);
	vendorstr = vendor_id2str(vendor);
	if (vendorstr) {
		sprintf(vendorbuf, "%s", vendorstr);
	}
	devstr = device_id2str(vendor, dev);
	if (devstr) {
		sprintf(devbuf, "%s", devstr);
	}
#else	
	vendorbuf[0] = 0;
	devbuf[0] = 0;
#endif
}

void amba_prinf_config(void)
{
	char devbuf[128];
	char vendorbuf[128];
	unsigned int conf;
	int i = 0;
	int j = 0;
	unsigned int addr;
	unsigned int m;
	printk("             Vendors         Slaves\n");
	printk("Ahb masters:\n");
	i = 0;
	while (i < amba_conf.ahbmst.devnr) {
		conf = amba_get_confword(amba_conf.ahbmst, i, 0);
		vendor_dev_string(conf, vendorbuf, devbuf);
		printk("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf),
		       amba_device(conf), amba_irq(conf), vendorbuf, devbuf);
		for (j = 0; j < 4; j++) {
			m = amba_ahb_get_membar(amba_conf.ahbmst, i, j);
			if (m) {
				addr = amba_membar_start(m);
				printk(" +%i: 0x%x \n", j, addr);
			}
		}
		i++;
	}
	printk("Ahb slaves:\n");
	i = 0;
	while (i < amba_conf.ahbslv.devnr) {
		conf = amba_get_confword(amba_conf.ahbslv, i, 0);
		vendor_dev_string(conf, vendorbuf, devbuf);
		printk("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf),
		       amba_device(conf), amba_irq(conf), vendorbuf, devbuf);
		for (j = 0; j < 4; j++) {
			m = amba_ahb_get_membar(amba_conf.ahbslv, i, j);
			if (m) {
				addr = amba_membar_start(m);
				if (amba_membar_type(m) == AMBA_TYPE_AHBIO) {
					addr = AMBA_TYPE_AHBIO_ADDR(addr);
				} else if (amba_membar_type(m) ==
					   AMBA_TYPE_APBIO) {
					printk("Warning: apbio membar\n");
				}
				printk(" +%i: 0x%x (raw:0x%x)\n", j, addr, m);
			}
		}
		i++;
	}
	printk("Apb slaves:\n");
	i = 0;
	while (i < amba_conf.apbslv.devnr) {

		conf = amba_get_confword(amba_conf.apbslv, i, 0);
		vendor_dev_string(conf, vendorbuf, devbuf);
		printk("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf),
		       amba_device(conf), amba_irq(conf), vendorbuf, devbuf);

		m = amba_apb_get_membar(amba_conf.apbslv, i);
		addr = amba_iobar_start(amba_conf.apbslv.apbmst[i], m);
		printk(" +%2i: 0x%x (raw:0x%x) \n", 0, addr, m);

		i++;

	}

}

