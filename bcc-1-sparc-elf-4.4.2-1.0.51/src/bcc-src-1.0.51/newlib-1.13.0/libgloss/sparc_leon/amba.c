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
amba_confarea_type amba_conf;

/* Pointers to Interrupt Controller configuration registers */
/*volatile LEON3_IrqCtrl_Regs_Map *LEON3_IrqCtrl_Regs = 0; *//*moved to lcpudata.S */
volatile LEON3_GpTimer_Regs_Map *LEON3_GpTimer_Regs = 0;
unsigned long LEON3_GpTimer_Irq = 0;

unsigned long amba_find_apbslv_addr(unsigned long vendor, unsigned long device,
				    unsigned long *irq)
{
	unsigned int i, conf, iobar;
	for (i = 0; i < amba_conf.apbslv.devnr; i++) {
		conf = amba_get_confword(amba_conf.apbslv, i, 0);
		if ((amba_vendor(conf) == vendor)
		    && (amba_device(conf) == device)) {
			if (irq) {
				*irq = amba_irq(conf);
			}
			iobar = amba_apb_get_membar(amba_conf.apbslv, i);
			return amba_iobar_start(amba_conf.apbslv.apbmst[i], iobar);
		}
	}
	return 0;
}

#define amba_insert_device(tab, address) \
{ \
  if (LEON3_BYPASS_LOAD_PA(address)) \
  { \
    (tab)->addr[(tab)->devnr] = (address); \
    (tab)->devnr ++; \
  } \
} while(0)

#define amba_insert_apb_device(tab, address, apbmst, idx)	\
{ \
  if (*(address)) \
  { \
    (tab)->addr[(tab)->devnr] = (address); \
    (tab)->apbmst[(tab)->devnr] = (apbmst);	\
    (tab)->apbmstidx[(tab)->devnr] = (idx);	\
    (tab)->devnr ++; \
  } \
} while(0)

/*
 *  Used to scan system bus. Probes for AHB masters, AHB slaves and 
 *  APB slaves. Addresses to configuration areas of the AHB masters,
 *  AHB slaves, APB slaves and APB master are storeds in 
 *  amba_ahb_masters, amba_ahb_slaves and amba.
 */

int amba_init_done = 0;

void amba_init(void)
{
	unsigned int *cfg_area;	/* address to configuration area */
	unsigned int mbar, conf, apbmst;
	int i, j, idx = 0;
        volatile LEON3_IrqCtrl_Regs_Map *_LEON3_IrqCtrl_Regs = 0;
        volatile LEON3_GpTimer_Regs_Map *_LEON3_GpTimer_Regs = 0;

        if (amba_init_done) {
          return;
        }        
        amba_init_done = 1;

	memset(&amba_conf, 0, sizeof(amba_conf));
	/*amba_conf.ahbmst.devnr = 0; amba_conf.ahbslv.devnr = 0; amba_conf.apbslv.devnr = 0;*/

	cfg_area = (unsigned int *)(LEON3_IO_AREA | LEON3_CONF_AREA);

	for (i = 0; i < LEON3_AHB_MASTERS; i++) {
		amba_insert_device(&amba_conf.ahbmst, cfg_area);
		cfg_area += LEON3_AHB_CONF_WORDS;
	}

	cfg_area =
	    (unsigned int *)(LEON3_IO_AREA | LEON3_CONF_AREA |
			     LEON3_AHB_SLAVE_CONF_AREA);
	for (i = 0; i < LEON3_AHB_SLAVES; i++) {
		amba_insert_device(&amba_conf.ahbslv, cfg_area);
		cfg_area += LEON3_AHB_CONF_WORDS;
	}

	for (i = 0; i < amba_conf.ahbslv.devnr; i++) {
		conf = amba_get_confword(amba_conf.ahbslv, i, 0);
		mbar = amba_ahb_get_membar(amba_conf.ahbslv, i, 0);
		if ((amba_vendor(conf) == VENDOR_GAISLER)
		    && (amba_device(conf) == GAISLER_APBMST)) {
			int k;
			/*amba_conf.apbmst = */apbmst = amba_membar_start(mbar);
			cfg_area =
			    (unsigned int *)(apbmst | LEON3_CONF_AREA);

			for (j = amba_conf.apbslv.devnr, k = 0; j < AMBA_MAXAPB_DEVS && k < AMBA_MAXAPB_DEVS_PERBUS ;
			     j++, k++) {
				amba_insert_apb_device(&amba_conf.apbslv, cfg_area, apbmst, idx);
				cfg_area += LEON3_APB_CONF_WORDS;
			}
			idx ++;
		}
	}

	/* Find LEON3 Interrupt controler */
	if ((_LEON3_IrqCtrl_Regs = (volatile LEON3_IrqCtrl_Regs_Map *)
                        amba_find_apbslv_addr(VENDOR_GAISLER, GAISLER_IRQMP, 0)))
                LEON3_IrqCtrl_Regs = _LEON3_IrqCtrl_Regs;
	if ((_LEON3_GpTimer_Regs = (volatile LEON3_GpTimer_Regs_Map *)
                        amba_find_apbslv_addr(VENDOR_GAISLER, GAISLER_GPTIMER,
                                &LEON3_GpTimer_Irq)))
                LEON3_GpTimer_Regs = _LEON3_GpTimer_Regs;
	if (LEON3_IrqCtrl_Regs) {
		LEON3_BYPASS_STORE_PA(&(LEON3_IrqCtrl_Regs->mask[0]), 0);
	}
}
