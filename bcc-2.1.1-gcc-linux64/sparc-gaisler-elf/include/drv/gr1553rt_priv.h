/*
 * Copyright 2018 Cobham Gaisler AB
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/** PRIVATE
 * Driver private structures.  These shall never be referenced by a user.
 */

#ifndef __GR1553RT_PRIV_H__
#define __GR1553RT_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <drv/osal.h>
#include <drv/gr1553b.h>
#include <drv/gr1553rt.h>

/* Software description of a subaddress */
struct gr1553rt_subadr {
	/* RX LIST */
	unsigned char rxlistid;
	/* TX LIST */
	unsigned char txlistid;
};

struct gr1553rt_irqerr {
	gr1553rt_irqerr_t func;
	void *data;
};

struct gr1553rt_irqmc {
	gr1553rt_irqmc_t func;
	void *data;
};

struct gr1553rt_irq {
	gr1553rt_irq_t func;
	void *data;
};

struct gr1553rt_priv {
	/* Pointer to Hardware registers */
	struct gr1553b_regs *regs;

	/* Software State */
	int started;
	struct gr1553rt_cfg cfg;
	SPIN_DECLARE(devlock)

	/* Handle to GR1553B RT device layer */
	GR1553B dev;
	struct osal_isr_ctx irq;

	/* Each Index represents one RT Subaddress. 31 = Broadcast */
	struct gr1553rt_subadr subadrs[32];

	/* Pointer to array of Software's description of a hardware
	 * descriptor.
	 */
	void *bd_sw_buffer;
	struct gr1553rt_sw_bd *swbds;

	/* List of Free descriptors */
	unsigned short swbd_free;
	int swbd_free_cnt;

	/* Hardware SubAddress descriptors given for CPU and Hardware */
	void *satab_buffer;
	struct gr1553rt_sa *sas_cpu;	/* Translated for CPU */
	struct gr1553rt_sa *sas_hw;	/* Translated for Hardware */

	/* Hardware descriptors address given for CPU and hardware */
	void *bd_buffer;
	int bds_cnt;			/* Number of descriptors */
	struct gr1553rt_bd *bds_cpu;	/* Translated for CPU */
	struct gr1553rt_bd *bds_hw;	/* Translated for Hardware */


	/* Event Log buffer in */
	void *evlog_buffer;
	unsigned int *evlog_cpu_next;	/* Next LOG entry to be handled */
	unsigned int *evlog_cpu_base;	/* First Entry in LOG */
	unsigned int *evlog_cpu_end;	/* Last+1 Entry in LOG */
	unsigned int *evlog_hw_base;	/* Translated for Hardware */

	/* Each Index represents a LIST ID */
	struct gr1553rt_list *lists[RTLISTID_MAX];

	/* IRQ handlers, one per SUBADDRESS */
	struct gr1553rt_irq irq_rx[32];
	struct gr1553rt_irq irq_tx[32];

	/* ISR called when an ERROR IRQ is received */
	struct gr1553rt_irqerr irq_err;

	/* ISR called when an Mode Code is received */
	struct gr1553rt_irqmc irq_mc;
};

#ifdef __cplusplus
}
#endif

#endif /* __GR1553RT_PRIV_H__ */
