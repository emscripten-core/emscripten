#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Test emmalloc internals, but through the external interface. We expect
// very specific outputs here based on the internals, this test would not
// pass in another malloc.

int main() {
  printf("allocate 0\n");
  void* ptr = malloc(0);
  assert(ptr == 0);
  printf("allocate 100\n");
  void* first = malloc(100);
  printf("free 100\n");
  free(first);
  printf("allocate another 100\n");
  void* second = malloc(100);
  assert(second == first);
  void* third = malloc(10);
  assert(third == first + 112); // allocation units are multiples of 16
  puts("ok");
}

