/*
 * Copyright (c) 2018, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DRV_FIFO_H
#define DRV_FIFO_H

#include <stdint.h>
#include <stddef.h>

struct fifo {
        uint8_t *buf;
        uint8_t *rdptr;
        uint8_t *wrptr;
        uint8_t *max;
};

/*
 * Initialize FIFO. The fifo structure is initialized to control buflen number
 * of bytes which are located at buf. At return of this function the FIFO is in
 * empty state. It is the users responsibility to allocate the buffer.
 */
void fifo_init(struct fifo *fifo, int buflen, uint8_t *buf);

/* Return full status of FIFO */
static inline int fifo_isfull(const struct fifo *fifo);

/* Return empty status of FIFO */
static inline int fifo_isempty(const struct fifo *fifo);

/*
 * Enqueue a byte in FIFO. The data byte is enqueued in the FIFO iff the FIFO
 * is not full.
 * Return value:
 * 0 - Data is enqueued.
 * nonzero - FIFO was full and data is not enqueued.
 */
static inline int fifo_put(struct fifo *fifo, uint8_t data);

/*
 * Dequeue a byte from FIFO. One data byte is dequeued from the FIFO if
 * available. The dequeued byte is stored in the location specified by the data
 * parameter if non-NULL. If data is NULL then a byte will be dequeued if
 * possible and discarded.
 * Return value:
 * 0 - Data is dequeued.
 * nonzero - FIFO was empty and data is not dequeued.
*/
static inline int fifo_get(struct fifo *fifo, uint8_t *data);


/** Inline implementation **/

/*
 * Full states
 *
 * buf + 0 | rd |
 * buf + 1 |    | wr
 * buf + 2 |    | rd
 * buf + 3 |    |
 *       . |    |
 *     max | wr |
 */
static inline int fifo_isfull(const struct fifo *fifo)
{
        uint8_t *rdptr = fifo->rdptr;
        uint8_t *wrptr = fifo->wrptr;

        if (((wrptr+1) == rdptr) ||
            ((wrptr == fifo->max) && (rdptr == fifo->buf))) {
                return 1;
        }
        return 0;
}

/*
 * Empty states
 *
 * buf + 0 | rd = wr
 * buf + 1 |
 * buf + 2 |
 * buf + 3 |
 *       . |
 *     max |
 */
static inline int fifo_isempty(const struct fifo *fifo)
{
        return fifo->wrptr == fifo->rdptr;
}

static inline int fifo_put(struct fifo *fifo, uint8_t data)
{
        if (fifo_isfull(fifo)) {
                return 1;
        }

        *fifo->wrptr = data;
        if (fifo->wrptr < fifo->max) {
                fifo->wrptr++;
        } else {
                fifo->wrptr = fifo->buf;
        }

        return 0;
}

static inline int fifo_get(struct fifo *fifo, uint8_t *data)
{
        if (fifo_isempty(fifo)) {
                return 1;
        }

        if (NULL != data) {
                *data = *fifo->rdptr;
        }
        if (fifo->rdptr < fifo->max) {
                fifo->rdptr++;
        } else {
                fifo->rdptr = fifo->buf;
        }

        return 0;
}

#endif

