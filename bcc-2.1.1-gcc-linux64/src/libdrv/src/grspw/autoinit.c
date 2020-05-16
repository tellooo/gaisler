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

#include <bcc/ambapp_ids.h>
#include <drv/grspw.h>
#include <drv/auto.h>

static struct auto_cfg THECFG = {
        .vendor = VENDOR_GAISLER,
        .device = GAISLER_SPW,
        .devsize = sizeof(struct grspw_devcfg),
        .count = 0,
        .reg = (auto_cfg_regf *) grspw_register,
};

int grspw_autoinit(void)
{
        int count;

        count = drv_autoinit(&THECFG);

        THECFG.device = GAISLER_SPW2;
        count += drv_autoinit(&THECFG);

        THECFG.device = GAISLER_SPW2_DMA;
        count += drv_autoinit(&THECFG);

        return count;
}

