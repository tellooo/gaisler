#ifndef AMPMSG_MUTEX_H
#define AMPMSG_MUTEX_H

#include <stdint.h>

struct mutex {
        volatile uint8_t val;
};

void mutex_init(struct mutex *mutex);
void mutex_lock(struct mutex *mutex);
void mutex_unlock(struct mutex *mutex);

#endif

