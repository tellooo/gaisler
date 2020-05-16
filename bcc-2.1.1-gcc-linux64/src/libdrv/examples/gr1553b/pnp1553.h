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

/* Example of simple PnP header. Nothing thought through... */

#ifndef PNP1553_H_
#define PNP1553_H_

/* 32 byte PnP header */
struct pnp1553info {
	uint16_t vendor;		/* 0x00 */
	uint16_t device;		/* 0x02 */

	uint16_t version;		/* 0x04 */
	uint16_t class;			/* 0x06 */

	uint16_t subadr_rx_avail;	/* 0x08 */
	uint16_t subadr_tx_avail;	/* 0x0A */

	char     desc[20];		/* 0x0C */
};

#endif
