/*
 * Custom trap table installed at link time
 */

#include <stdio.h>
#include <bcc/bcc.h>

extern uint32_t __bcc_trap_table_svt_bad;

/* Override weak symbol __bcc_trap_table_svt_2 */
uint32_t *__bcc_trap_table_svt_2[16] = {
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
        &__bcc_trap_table_svt_bad,
};

/* trap handler implemented in assembly. */
extern void handler2b(void);

int main(void)
{
        int ret;

        ret = bcc_set_trap(0x2b, handler2b);
        if (ret == BCC_OK) {
                puts("handler installed for trap 0x2b");
        } else {
                puts("ERROR: could not install trap handler");
        }
        return 0;
}

