#ifndef _PT_MALLOC_H_
#define _PT_MALLOC_H_

#include <malloc.h>

malloc_t pthread_malloc (size_t size);
int pthread_free (malloc_t ptr);
malloc_t pthread_realloc (malloc_t ptr,size_t size);
malloc_t pthread_calloc (size_t nmemb, size_t elsize);
void pthread_cfree (malloc_t ptr);
malloc_t pthread_memalign(size_t alignment, size_t size);
malloc_t pthread_valloc(size_t size);

#endif /* _PT_DEBUG_H */
