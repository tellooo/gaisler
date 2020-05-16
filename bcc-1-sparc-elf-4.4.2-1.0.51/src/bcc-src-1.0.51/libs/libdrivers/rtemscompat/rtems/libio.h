#ifndef _RTEMS_RTEMS_LIBIO_H
#define _RTEMS_RTEMS_LIBIO_H

#include <stdint.h>

struct termios;
typedef struct rtems_termios_callbacks {
  int    (*firstOpen)(int major, int minor, void *arg);
  int    (*lastClose)(int major, int minor, void *arg);
  int    (*pollRead)(int minor);
  int    (*write)(int minor, const char *buf, int len);
  int    (*setAttributes)(int minor, const struct termios *t);
  int    (*stopRemoteTx)(int minor);
  int    (*startRemoteTx)(int minor);
  int    outputUsesInterrupts;
} rtems_termios_callbacks;

#define TERMIOS_POLLED      0
#define TERMIOS_IRQ_DRIVEN  1
#define TERMIOS_TASK_DRIVEN 2

typedef struct rtems_libio_tt {
    void          *data1;
} rtems_libio_t;

typedef struct {
    rtems_libio_t          *iop;
    uint32_t                flags;
    uint32_t                mode;
} rtems_libio_open_close_args_t;

typedef struct {
    rtems_libio_t          *iop;
    uint32_t                command;
    void                   *buffer;
    uint32_t                ioctl_return;
} rtems_libio_ioctl_args_t;

typedef struct {
    rtems_libio_t          *iop;
    uint32_t                offset;
    char                   *buffer;
    uint32_t                count;
    uint32_t                flags;
    uint32_t                bytes_moved;
} rtems_libio_rw_args_t;

#endif
