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

#ifndef __BM_LOGGER_H_
#define __BM_LOGGER_H_

typedef struct bm_logger* bm_logger_t;

/**
 * Initilize a bm_log object
 *
 * bm_log   - Object to be initalized
 * log_base - Base address for the BM driver
 */
int bm_init(bm_logger_t *bm, void* log_base);

/**
 * Collect BM data
 */
int bm_log(bm_logger_t bm);

/**
 * Returns current count
 */
int bm_count(bm_logger_t bm);

/**
 * Shutdown and clean-up a bm_log object
 */
void bm_stop(bm_logger_t bm);

#endif /* __BM_LOGGER_H_ */
