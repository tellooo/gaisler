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

#ifndef DRV_AUTO_H
#define DRV_AUTO_H

#include <stdint.h>
#include <stddef.h>

struct drv_node {
        struct drv_node *next;
};

struct drv_list {
        struct drv_node *head;
        struct drv_node *tail;
};

static inline void drv_list_clr(
        struct drv_list *list
)
{
        list->head = NULL;
        list->tail = NULL;
}

static inline void drv_list_addtail(
        struct drv_list *list,
        struct drv_node *pkt
)
{
        pkt->next = NULL;
        if (list->tail == NULL) {
                list->head = pkt;
        } else {
                list->tail->next = pkt;
        }
        list->tail = pkt;
}

static inline const struct drv_node *drv_list_getbyindex(
        const struct drv_list *list,
        int index
)
{
        const struct drv_node *node = list->head;
        for (int i = 0; i < index; i++) {
                if (!node) {
                        return NULL;
                }
                node = node->next;
        }
        return node;
}

struct drv_devreg {
        struct drv_node node;
        uint32_t addr;
        short interrupt;
        short device_id;
        short version;
};

typedef int (auto_cfg_regf)(struct drv_devreg *devreg);

struct auto_cfg {
        uint32_t vendor;
        uint32_t device;
        size_t devsize;
        int count;
        auto_cfg_regf *reg;
};

int drv_autoinit(struct auto_cfg *cfg);

#endif

