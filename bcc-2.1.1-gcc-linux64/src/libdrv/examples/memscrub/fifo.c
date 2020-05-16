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

#include <stddef.h>

#include "fifo.h"

void fifo_init(struct fifo *fifo, int buflen)
{
        fifo->rdptr = 0;
        fifo->wrptr = 0;
        fifo->max = buflen-1;
}

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
int fifo_isfull(const struct fifo *fifo)
{
        int rdptr = fifo->rdptr;
        int wrptr = fifo->wrptr;

        if (((wrptr+1) == rdptr) ||
            ((wrptr == fifo->max) && (rdptr == 0))) {
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
int fifo_isempty(const struct fifo *fifo)
{
        return fifo->wrptr == fifo->rdptr;
}

int fifo_put(struct fifo *fifo)
{
        if (fifo_isfull(fifo)) {
                return 1;
        }

        if (fifo->wrptr < fifo->max) {
                fifo->wrptr++;
        } else {
                fifo->wrptr = 0;
        }

        return 0;
}

int fifo_rdptr(struct fifo *fifo)
{
        if (fifo_isempty(fifo)) {
                return -1;
        }
        return fifo->rdptr;
}

int fifo_get(struct fifo *fifo, int *index)
{
        if (fifo_isempty(fifo)) {
                return 1;
        }

        if (NULL != index) {
                *index = fifo->rdptr;
        }
        if (fifo->rdptr < fifo->max) {
                fifo->rdptr++;
        } else {
                fifo->rdptr = 0;
        }

        return 0;
}

