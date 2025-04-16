#include <assert.h>
#include <malloc.h>
#include <stdio.h>

// Mark as used to defeat LTO which can otherwise completely elimate the
// calls to malloc below.
void* ptr __attribute__((used));

int main()
{
  // Usable size should always be at least the size of 2 pointers
  // (See SMALLEST_ALLOCATION_SIZE in emmalloc.c)
  ptr = malloc(1);
  size_t usable = malloc_usable_size(ptr);
  printf("malloc_usable_size: %zu\n", usable);
  assert(usable == 2 * sizeof(void*));
  return 0;
}
