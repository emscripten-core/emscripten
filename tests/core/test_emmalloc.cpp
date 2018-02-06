#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>

extern void emmalloc_blank_slate_from_orbit();
extern void emmalloc_dump_everything();

// Test emmalloc internals, but through the external interface. We expect
// very specific outputs here based on the internals, this test would not
// pass in another malloc.

void check_where_we_would_malloc(size_t size, void* expected) {
  void* temp = malloc(size);
  assert(temp == expected);
  free(temp);
}

void stage(const char* name) {
  EM_ASM({
    Module.print('>> ' + Pointer_stringify($0));
  }, name);
  emmalloc_dump_everything();
}

void one() {
  stage("allocate 0");
  void* ptr = malloc(0);
  assert(ptr == 0);
  stage("allocate 100");
  void* first = malloc(100);
  stage("free 100");
  free(first);
  stage("allocate another 100");
  void* second = malloc(100);
  stage("allocate 10");
  assert(second == first);
  void* third = malloc(10);
  assert(size_t(third) == size_t(first) + 112 + 16); // allocation units are multiples of 16, so first allocates a payload of 112. then second has 16 of metadata
  stage("allocate 10 more");
  void* four = malloc(10);
  assert(size_t(four) == size_t(third) + 16 + 16); // allocation units are multiples of 16, so first allocates a payload of 16. then second has 16 of metadata
  stage("free the first");
  free(second);
  stage("several temp alloc/frees");
  // we reuse the first area, despite stuff later.
  for (int i = 0; i < 4; i++) {
    check_where_we_would_malloc(100, first);
  }
  stage("free everything");
  free(third);
  emmalloc_dump_everything();
  free(four);
  stage("allocate various sizes to see they all start at the start");
  for (int i = 1; i < 300; i++) {
    check_where_we_would_malloc(i, first);
  }
  stage("the_end");
}

int main() {
  one();
}

