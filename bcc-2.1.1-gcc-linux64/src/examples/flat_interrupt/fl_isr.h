#ifndef FL_ISR_H
#define FL_ISR_H

#include <stdint.h>

#define FL_ISR_NVECS 16

struct fl_isr_ivec {
        uintptr_t arg;
        void (*handler)(uintptr_t arg);
};

int fl_isr_init(void);

int fl_isr_register(
        int source,
        void (*handler)(uintptr_t arg),
        uintptr_t arg
);

/* private */
extern struct fl_isr_ivec fl_isr_vectors[FL_ISR_NVECS];

#endif

