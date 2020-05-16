#include <mutex.h>
#include <bcc/bcc.h>

#ifndef MUTEX_DEBUG
#define MUTEX_DEBUG 0
#endif

#include <stdio.h>
#define DBG(str, ...) if (MUTEX_DEBUG) { \
        printf(str, __VA_ARGS__); \
}

#define DBGFLUSH() if (MUTEX_DEBUG) { \
        fflush(NULL); \
}

void mutex_init(struct mutex *mutex)
{
        DBG("%s: ENTRY\n", __func__);
        mutex->val = 0;
}

void mutex_lock(struct mutex *mutex)
{
        DBG("%s: ENTRY\n", __func__);
        while (1) {
                uint8_t val;

                DBG("%s: spin...", __func__);
                DBGFLUSH();
                while (mutex->val) {
                        ;
                }
                val = bcc_ldstub((uint8_t *) &mutex->val);
                if (val) {
                        DBG("%s: did not get (val=%d)\n", __func__, val);
                } else {
                        DBG("%s: got it (val=%d)\n", __func__, val);
                        break;
                }
        }
}

void mutex_unlock(struct mutex *mutex)
{
        DBG("%s: ENTRY\n", __func__);
        mutex->val = 0;
}

