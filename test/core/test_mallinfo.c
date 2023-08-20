// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Tests that we can use the dlmalloc mallinfo() function to obtain information
// about malloc()ed blocks and compute how much memory is used/freed.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <malloc.h>
#include <emscripten/em_asm.h>

size_t getTotalMemory() {
  return (size_t)EM_ASM_PTR(return HEAP8.length);
}

size_t getFreeMemory() {
  struct mallinfo i = mallinfo();
  uintptr_t totalMemory = getTotalMemory();
  uintptr_t dynamicTop = (uintptr_t)sbrk(0);
  return totalMemory - dynamicTop + i.fordblks;
}

int main() {
  size_t total_mem = getTotalMemory();
  size_t free_mem = getFreeMemory();

  printf("Before allocation:\n");
  printf("Total memory: %zu bytes\n", getTotalMemory());
  printf("Free memory: %zu bytes\n", getFreeMemory());
  printf("Used: %zu bytes (%.2f%%)\n", getTotalMemory() - getFreeMemory(), (getTotalMemory() - getFreeMemory()) * 100.0 / getTotalMemory());
  assert(getTotalMemory() == total_mem);
  assert(getFreeMemory() == free_mem);

  void *ptr = malloc(1024*1024);
  printf("\nAfter 1MB allocation: %p\n", ptr);
  printf("Total memory: %zu bytes\n", getTotalMemory());
  printf("Free memory: %zu bytes\n", getFreeMemory());
  printf("Used: %zu bytes (%.2f%%)\n", getTotalMemory() - getFreeMemory(), (getTotalMemory() - getFreeMemory()) * 100.0 / getTotalMemory());
  assert(getTotalMemory() == total_mem);
  assert(getFreeMemory() < free_mem);

  free(ptr);
  printf("\nAfter freeing:\n");
  printf("Total memory: %zu bytes\n", getTotalMemory());
  printf("Free memory: %zu bytes\n", getFreeMemory());
  printf("Used: %zu bytes (%.2f%%)\n", getTotalMemory() - getFreeMemory(), (getTotalMemory() - getFreeMemory()) * 100.0 / getTotalMemory());
  assert(getTotalMemory() == total_mem);
  assert(getFreeMemory() == free_mem);

  printf("OK.\n");
}
