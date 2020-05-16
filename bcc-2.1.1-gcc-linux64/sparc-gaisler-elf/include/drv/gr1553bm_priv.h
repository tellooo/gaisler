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

#ifndef __GR1553BM_PRIV_H__
#define __GR1553BM_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <drv/osal.h>
#include <drv/gr1553b.h>
#include <drv/gr1553bm.h>

struct gr1553bm_priv {
	GR1553B dev;
	struct osal_isr_ctx irq;

	struct gr1553b_regs *regs;
	SPIN_DECLARE(devlock)

	void *buffer;
	unsigned int buffer_base_hw;
	unsigned int buffer_base;
	unsigned int buffer_end;
	unsigned int buffer_size;
	unsigned int read_pos;
	int started;
	struct gr1553bm_config cfg;

	/* Time updated by IRQ when 24-bit Time counter overflows */
	volatile uint64_t time;
};

#ifdef __cplusplus
}
#endif

#endif /* __GR1553BM_PRIV_H__ */
