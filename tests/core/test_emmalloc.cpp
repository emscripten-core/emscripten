#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>

extern void emmalloc_blank_slate_from_orbit();
extern void emmalloc_validate_all();
extern void emmalloc_dump_all();

// Test emmalloc internals, but through the external interface. We expect
// very specific outputs here based on the internals, this test would not
// pass in another malloc.

void* check_where_we_would_malloc(size_t size) {
  void* temp = malloc(size);
  free(temp);
  return temp;
}

void check_where_we_would_malloc(size_t size, void* expected) {
  void* temp = malloc(size);
  assert(temp == expected);
  free(temp);
}

void stage(const char* name) {
  EM_ASM({
    Module.print('\n>> ' + Pointer_stringify($0) + '\n');
  }, name);
  emmalloc_validate_all();
  emmalloc_dump_all();
}

void basics() {
  stage("basics");
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
  stage("free all");
  free(third);
  emmalloc_dump_all();
  free(four);
  stage("allocate various sizes to see they all start at the start");
  for (int i = 1; i < 1500; i++) {
    check_where_we_would_malloc(i, first);
  }
}

void blank_slate() {
  stage("blank_slate");
  emmalloc_blank_slate_from_orbit();
  void* ptr = malloc(0);
  free(ptr);
  for (int i = 0; i < 3; i++) {
    void* two = malloc(0);
    assert(two == ptr);
    free(two);
  }
  for (int i = 0; i < 3; i++) {
    emmalloc_blank_slate_from_orbit();
    void* two = malloc(0);
    assert(two == ptr);
    free(two);
  }
}

void previous_sbrk() {
  stage("previous_sbrk");
  emmalloc_blank_slate_from_orbit();
  void* old = sbrk(0);
  assert((size_t)old % 16 == 0);
  sbrk(3); // unalign things
  void* other = malloc(10);
  free(other);
  assert(other != old);
  assert((char*)other == (char*)old + 32);
}

void min_alloc() {
  stage("min_alloc");
  emmalloc_blank_slate_from_orbit();
  void* start = check_where_we_would_malloc(1);
  for (int i = 1; i < 100; i++) {
    void* temp = malloc(i);
    void* expected = (char*)start + 16 + 16 * ((i + 15) / 16);
    EM_ASM({ Module.print('min_alloc: ' + [$0, $1, $2]) }, i, temp, expected);
    check_where_we_would_malloc(1, expected);
    free(temp);
  }
}

// Add test for
//  * alloc 1,2,4,8,16, etc.
//  * free it
//  * alloc 1. should be at the start, not after a big emptiness

int main() {
  stage("beginning");

  basics();
  blank_slate();
  previous_sbrk();
  min_alloc();

  stage("the_end");
}

