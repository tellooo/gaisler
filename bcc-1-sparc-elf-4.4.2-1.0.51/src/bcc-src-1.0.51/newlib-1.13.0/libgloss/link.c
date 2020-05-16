#include <errno.h>
#include "glue.h"

/*
 * unlink -- since we have no file system, 
 *           we just return an error.
 */
int
_DEFUN (_link, (existing, new),
        char *existing _AND
        char *new)
{
  errno = EIO;
  return (-1);
}


