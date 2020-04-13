#include <emscripten/emmalloc.h>
#include <stdio.h>

// Mark as used to defeat LTO which can otherwise completely elimate the
// calls to malloc below.
void* ptr __attribute__((used));

int main()
{
  ptr = malloc(1);
  printf("%zu\n", malloc_usable_size(ptr));
  return 0;
}
