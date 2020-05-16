int bcc_set_pil(int newpil);

static inline UWORD protect_start (void *ptr)
{
        (void) sizeof(ptr);
        return bcc_set_pil(15);
}

static inline void protect_end (void *ptr, UWORD plevel)
{
        bcc_set_pil(plevel);
}

#include_next <host-config.h>

