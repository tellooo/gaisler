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

#ifndef __GR1553BC_PRIV_H__
#define __GR1553BC_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <drv/osal.h>
#include <drv/gr1553b.h>
#include <drv/gr1553bc.h>

struct gr1553bc_priv {
	GR1553B dev;
	struct osal_isr_ctx irq;

	struct gr1553b_regs *regs;
	struct gr1553bc_list *list;
	struct gr1553bc_list *alist;
	int started;
	SPIN_DECLARE(devlock)

	/* IRQ log management */
	uint32_t irq_log_p[(GR1553BC_IRQLOG_SIZE*2)/sizeof(uint32_t)];
	uint32_t *irq_log_base;
	uint32_t *irq_log_curr;
	uint32_t *irq_log_end;
	uint32_t *irq_log_base_hw;

	/* Standard IRQ handler function */
	bcirq_func_t irq_func;
	void *irq_data;
};

/* Translate Descriptor address from CPU-address to Hardware Address */
static __inline__ union gr1553bc_bd *gr1553bc_bd_cpu2hw
	(
	struct gr1553bc_list *list,
	union gr1553bc_bd *bd
	)
{
	return (union gr1553bc_bd *)(((unsigned int)bd - list->table_cpu) +
		list->table_hw);
}

#ifdef __cplusplus
}
#endif

#endif /* __GR1553BC_PRIV_H__ */
