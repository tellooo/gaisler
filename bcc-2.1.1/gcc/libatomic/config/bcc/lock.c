#include "libatomic_i.h"

static int __bcc_libatomic_lock;

void
libat_lock_n (void *ptr, size_t n)
{
        (void) sizeof(ptr);
        (void) sizeof(n);
        __bcc_libatomic_lock = protect_start(NULL);
}

void
libat_unlock_n (void *ptr, size_t n)
{
        (void) sizeof(ptr);
        (void) sizeof(n);
        protect_end(NULL, __bcc_libatomic_lock);
}

