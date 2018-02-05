#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Test emmalloc internals, but through the external interface. We expect
// very specific outputs here based on the internals, this test would not
// pass in another malloc.

int main() {
  void* ptr = malloc(0);
  assert(ptr == 0);
  void* first = malloc(100);
  free(first);
  void* second = malloc(100);
  assert(second == first);
  puts("ok");
}

