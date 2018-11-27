#include <stdlib.h>

// Musl has an aligned_alloc routine, but that builds on top of standard malloc(). We are using dlmalloc, so
// can route to its implementation instead.
void *aligned_alloc(size_t alignment, size_t size)
{
  void *ptr;
  int ret = posix_memalign(&ptr, alignment, size);
  return (ret == 0) ? ptr : 0;
}
