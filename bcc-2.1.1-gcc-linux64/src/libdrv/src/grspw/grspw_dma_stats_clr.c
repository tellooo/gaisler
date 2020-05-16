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

#include <drv/grspw.h>
#include <string.h>

void grspw_dma_stats_clr(struct grspw_dma_priv *dma)
{
        memset(&dma->stats, 0, sizeof(dma->stats));

	/* Init proper default values so that comparisons will work the
	 * first time.
	 */
        dma->stats.tx_sched_cnt_min = 0x3fffffff;
        dma->stats.rx_sched_cnt_min = 0x3fffffff;
}

