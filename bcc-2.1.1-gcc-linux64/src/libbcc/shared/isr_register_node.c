/*
 * Copyright (c) 2017, Cobham Gaisler AB
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

#include <stdlib.h>
#include "bcc/bcc.h"
#include "isr_priv.h"

int bcc_isr_register_node(
        struct bcc_isr_node *node
)
{
        int source;
        void (*handler)(
                void *arg,
                int source
        );
        void *arg;

        source = node->source;
        handler = node->handler;
        arg = node->arg;

        if (source < 1) { return BCC_FAIL; }
        if (ISR_NSOURCES <= source) { return BCC_FAIL; }
        if (NULL == handler) { return BCC_FAIL; }

        DBG(
                "source=%d, handler=%x, arg=%p\n",
                source,
                (unsigned int) handler,
                arg
        );

        struct bcc_isr_node *n;
        int plevel;

        node->__private = NULL;

        /* Link in new node. */
        n = (struct bcc_isr_node *) &__bcc_isr_list[source];

        plevel = bcc_int_disable();

        while (n->__private) {
                n = n->__private;
        }
        n->__private = node;

        bcc_int_enable(plevel);

        return BCC_OK;
}

