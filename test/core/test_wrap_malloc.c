// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/heap.h>
#include <stdio.h>
#include <stdlib.h>

static int totalAllocs;
static int totalFrees;
static int totalReallocs;

void *malloc(size_t size) {
  ++totalAllocs;
  void *ptr = emscripten_builtin_malloc(size);
  emscripten_console_logf("Allocated %zu bytes, got %p. %d pointers allocated total.", size, ptr, totalAllocs);
  return ptr;
}

void *realloc(void* ptr, size_t size) {
  ++totalReallocs;
  ptr = emscripten_builtin_realloc(ptr, size);
  emscripten_console_logf("Reallocated %zu bytes, got %p. %d pointers re-allocated total.", size, ptr, totalReallocs);
  return ptr;
}

void free(void *ptr) {
  ++totalFrees;
  emscripten_builtin_free(ptr);
  emscripten_console_logf("Freed ptr %p, %d pointers freed total.", ptr, totalFrees);
}

int main() {
  // Reset these globals here, to ignore any allocations during startup by the
  // system
  totalAllocs = 0;
  totalFrees = 0;

  for (int i = 0; i < 20; ++i) {
    void *ptr = malloc(1024 * 1024);
    // Make sure we do something with the pointer do ensure the optimizer
    // doesn't completely remove the allocation.
    emscripten_console_logf("alloacted: %p", ptr);
    free(ptr);
  }

  void* ptr = malloc(50);
  ptr = realloc(ptr, 50);

  emscripten_console_logf("totalAllocs: %d", totalAllocs);
  emscripten_console_logf("totalFrees: %d", totalFrees);
  emscripten_console_logf("totalReallocs: %d", totalReallocs);
  assert(totalAllocs == 21);
  assert(totalReallocs == 1);
  assert(totalFrees == 20);
  emscripten_console_logf("OK.");
  return 0;
}
