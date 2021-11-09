#include <stdlib.h>

// Musl has an aligned_alloc routine, but that builds on top of standard malloc(). We are using dlmalloc, so
// can route to its implementation instead.
void * weak aligned_alloc(size_t alignment, size_t size)
{
  void *ptr;
  if ((alignment % sizeof(void *) != 0) || (size % alignment) != 0)
    return 0;
  int ret = posix_memalign(&ptr, alignment, size);
  return (ret == 0) ? ptr : 0;
}
