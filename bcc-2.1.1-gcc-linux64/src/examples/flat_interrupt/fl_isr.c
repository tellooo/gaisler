#include "fl_isr.h"

/* Access to this array is done with ldd instruction */
struct fl_isr_ivec fl_isr_vectors[FL_ISR_NVECS] __attribute__ ((aligned (8)));

int fl_isr_register(
        int source,
        void (*handler)(uintptr_t arg),
        uintptr_t arg
)
{
        fl_isr_vectors[source].handler = handler;
        fl_isr_vectors[source].arg = arg;
        return 0;
}

int fl_isr_init(void)
{
        /* Trap on unexpected interrupt. */
        for (int i = 0; i < FL_ISR_NVECS; i++) {
                fl_isr_register(i, (void (*)(uintptr_t)) 1, 0);
        }
        return 0;
}

