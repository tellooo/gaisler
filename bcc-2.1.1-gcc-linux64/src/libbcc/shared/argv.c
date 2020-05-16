#include <stddef.h>
#include "bcc/bcc_param.h"

/*
 * 5.1.2.2.1 Program startup
 * ...
 * -- argv[argc] shall be a null pointer.
 */

int __bcc_argc = 0;
/* "Array of char pointers" */
static char (*__bcc_argv)[] = { NULL };
/* "Pointer to array of char pointers" */
char *((*__bcc_argvp)[]) = &__bcc_argv;

