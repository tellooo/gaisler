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

#ifndef __GR1553B_PRIV_H__
#define __GR1553B_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <drv/auto.h>
#include <drv/gr1553bc_priv.h>
#include <drv/gr1553bm_priv.h>
#include <drv/gr1553rt_priv.h>

struct gr1553_device_feature {
	struct gr1553_device_feature *next;
	struct gr1553_device *dev;
	int minor;
};

struct gr1553_device {
	struct gr1553_devcfg *dev;
	int features;
	int alloc;
	struct gr1553bm_priv bm;
	union {
		struct gr1553bc_priv bc;
		struct gr1553rt_priv rt;
	} p;

	struct gr1553_device_feature feat[3];
};

/*
 * Device configuration
 *
 * This structure is used for registering the base addr of a device and its
 * interrupt number. It also contains the private device structure.
 */
struct gr1553_devcfg {
        struct drv_devreg regs;
        struct gr1553_device priv;
};

#ifdef __cplusplus
}
#endif

#endif /* __GR1553B_PRIV_H__ */
