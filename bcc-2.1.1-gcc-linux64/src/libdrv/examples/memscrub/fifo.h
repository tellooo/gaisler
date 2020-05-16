/*
 * Copyright (c) 2019, Cobham Gaisler AB
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

/*
 * Lock-free FIFO logic
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */
#ifndef FIFO_LOGIC_H
#define FIFO_LOGIC_H

struct fifo {
        int volatile rdptr;
        int volatile wrptr;
        int max;
};

/*
 * Initialize FIFO. The fifo structure is initialized to control buflen number
 * of 32-bit words which are located at buf. At return of this function the
 * FIFO is in empty state. It is the users responsibility to allocate the
 * buffer.
 */
void fifo_init(struct fifo *fifo, int buflen);

/* Return full status of FIFO */
int fifo_isfull(const struct fifo *fifo);

/*
 * Return free space as number of elements.
 *
 * At least this number of fifo_put() will be accepted before fifo_get().
 */
int fifo_spaceleft(const struct fifo *fifo);

/* Return empty status of FIFO */
int fifo_isempty(const struct fifo *fifo);

/* Return index for next put operation */
static inline int fifo_wrptr(struct fifo *fifo)
{
        return fifo->wrptr;
}

/*
 * Enqueue a word in FIFO. The data byte is enqueued in the FIFO iff the FIFO
 * is not full.
 * Return value:
 * 0 - Data is enqueued.
 * nonzero - FIFO was full and data is not enqueued.
 */
int fifo_put(struct fifo *fifo);

/* Return index for next get operation, or -1 if none */
int fifo_rdptr(struct fifo *fifo);

/*
 * Dequeue a byte from FIFO. One data byte is dequeued from the FIFO if
 * available. The dequeued byte is stored in the location specified by the data
 * parameter if non-NULL. If data is NULL then a byte will be dequeued if
 * possible and discarded.
 * Return value:
 * 0 - Data is dequeued.
 * nonzero - FIFO was empty and data is not dequeued.
*/
int fifo_get(struct fifo *fifo, int *index);

#endif

