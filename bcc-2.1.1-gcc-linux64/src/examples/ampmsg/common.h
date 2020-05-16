#ifndef _NODE_COMMON_H_
#define _NODE_COMMON_H_

#include <stdint.h>
#include <msg.h>

enum {
        LOG_LEVEL_FOO,
        LOG_LEVEL_BAR,
        LOG_LEVEL_CAT,
        LOG_LEVEL_DOG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_POWERDOWN,
};

static const char *const LOG_LEVEL_TO_STR[] = {
        "foo",
        "bar",
        "cat",
        "dog",
        "INFO",
        "POWERDOWN"
};

/* a log message type */
struct lmsg {
        struct msg msg;
        char *str;
        int level;
};

void sleepus(uint32_t duration);

#endif

