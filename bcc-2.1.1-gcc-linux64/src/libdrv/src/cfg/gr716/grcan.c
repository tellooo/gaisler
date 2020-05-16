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

#include <stddef.h>
#include <drv/gr716/grcan.h>
#include "pnp.h"
#include <bcc/ambapp_ids.h>

struct grcan_devcfg *GR716_GRCAN_DRV_ALL[] = {
        & (struct grcan_devcfg) {
                .regs = {
                        .addr       = GAISLER_GRCAN_0_PNP_APB,
                        .interrupt  = GAISLER_GRCAN_0_PNP_APB_IRQ,
                        .device_id  = GAISLER_GRCAN,
                        .version    = GAISLER_GRCAN_0_PNP_VERSION,
                },
        },
        & (struct grcan_devcfg) {
                .regs = {
                        .addr       = GAISLER_GRCAN_1_PNP_APB,
                        .interrupt  = GAISLER_GRCAN_1_PNP_APB_IRQ,
                        .device_id  = GAISLER_GRCAN,
                        .version    = GAISLER_GRCAN_1_PNP_VERSION,
                },
        },
        NULL
};

