// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>

#ifndef RANDOM_ITERS
#define RANDOM_ITERS 12345
#endif

extern void emmalloc_blank_slate_from_orbit();

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
    out('\n>> ' + UTF8ToString($0) + '\n');
  }, name);
}

const size_t ALLOCATION_UNIT = 8;

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
  assert(size_t(third) == size_t(first) + ((100 + ALLOCATION_UNIT - 1)&(-ALLOCATION_UNIT)) + ALLOCATION_UNIT); // allocation units are multiples of ALLOCATION_UNIT
  stage("allocate 10 more");
  void* four = malloc(10);
  assert(size_t(four) == size_t(third) + (2*ALLOCATION_UNIT) + ALLOCATION_UNIT); // payload (10 = 2 allocation units) and metadata
  stage("free the first");
  free(second);
  stage("several temp alloc/frees");
  // we reuse the first area, despite stuff later.
  for (int i = 0; i < 4; i++) {
    check_where_we_would_malloc(100, first);
  }
  stage("free all");
  free(third);
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
  assert((size_t)old % ALLOCATION_UNIT == 0);
  sbrk(3); // unalign things
  void* other = malloc(10);
  free(other);
  assert(other != old);
  assert((char*)other == (char*)old + 2 * ALLOCATION_UNIT);
}

void min_alloc() {
  stage("min_alloc");
  emmalloc_blank_slate_from_orbit();
  void* start = check_where_we_would_malloc(1);
  for (int i = 1; i < 100; i++) {
    void* temp = malloc(i);
    void* expected = (char*)start + ALLOCATION_UNIT + ALLOCATION_UNIT * ((i + ALLOCATION_UNIT - 1) / ALLOCATION_UNIT);
    check_where_we_would_malloc(1, expected);
    free(temp);
  }
}

void space_at_end() {
  stage("space_at_end");
  emmalloc_blank_slate_from_orbit();
  void* start = check_where_we_would_malloc(1);
  for (int i = 1; i < 50; i++) {
    for (int j = 1; j < 50; j++) {
      void* temp = malloc(i);
      free(temp);
      check_where_we_would_malloc(j, start);
    }
  }
}

void calloc() {
  stage("calloc");
  emmalloc_blank_slate_from_orbit();
  char* ptr = (char*)malloc(10);
  ptr[0] = 77;
  free(ptr);
  char* cptr = (char*)calloc(10, 1);
  assert(cptr == ptr);
  assert(ptr[0] == 0);
}

void realloc() {
  stage("realloc0");
  emmalloc_blank_slate_from_orbit();
  for (int i = 0; i < 2; i++) {
    char* ptr = (char*)malloc(10);
    stage("realloc0.1");
    char* raptr = (char*)realloc(ptr, 1);
    assert(raptr == ptr);
    stage("realloc0.2");
    char* raptr2 = (char*)realloc(raptr, 100);
    assert(raptr2 == ptr);
    char* last = (char*)malloc(1);
    assert(last >= ptr + 100);
    // slightly more still fits
    stage("realloc0.3");
    char* raptr3 = (char*)realloc(raptr2, 11);
    assert(raptr3 == ptr);
    // finally, realloc a size we must reallocate for
    stage("realloc0.4");
    char* raptr4 = (char*)realloc(raptr3, 1000);
    assert(raptr4);
    assert(raptr4 != ptr);
    // leaving those in place, do another iteration
  }
  stage("realloc1");
  emmalloc_blank_slate_from_orbit();
  {
    // realloc of NULL is like malloc
    void* ptr = check_where_we_would_malloc(10);
    assert(realloc(NULL, 10) == ptr);
  }
  stage("realloc2");
  emmalloc_blank_slate_from_orbit();
  {
    // realloc to 0 is like free
    void* ptr = malloc(10);
    assert(realloc(ptr, 0) == NULL);
    assert(check_where_we_would_malloc(10) == ptr);
  }
  stage("realloc3");
  emmalloc_blank_slate_from_orbit();
  {
    // realloc copies
    char* ptr = (char*)malloc(10);
    *ptr = 123;
    for (int i = 5; i <= 16; i++) {
      char* temp = (char*)realloc(ptr, i);
      assert(*temp == 123);
      assert(temp == ptr);
    }
    stage("realloc3.5");
    malloc(1);
    malloc(100);
    {
      char* temp = (char*)realloc(ptr, 17);
      assert(*temp == 123);
      assert(temp != ptr);
      ptr = temp;
    }
  }
}

void check_aligned(size_t align, size_t ptr) {
  if (align < 4 || ((align & (align - 1)) != 0)) {
    assert(ptr == 0);
  } else {
    assert(ptr);
    assert(ptr % align == 0);
  }
}

void aligned() {
  stage("aligned");
  for (int i = 0; i < 35; i++) {
    for (int j = 0; j < 35; j++) {
      emmalloc_blank_slate_from_orbit();
      size_t first = (size_t)memalign(i, 100);
      size_t second = (size_t)memalign(j, 100);
      printf("%d %d => %d %d\n", i, j, first, second);
      check_aligned(i, first);
      check_aligned(j, second);
    }
  }
}

void randoms() {
  stage("randoms");
  emmalloc_blank_slate_from_orbit();
  void* start = check_where_we_would_malloc(10);
  const int N = 1000;
  const int BINS = 128;
  void* bins[BINS];
  char values[BINS];
  for (int i = 0; i < BINS; i++) {
    bins[i] = NULL;
  }
  srandom(1337101);
  for (int i = 0; i < RANDOM_ITERS; i++) {
    unsigned int r = random();
    int alloc = r & 1;
    r >>= 1;
    int calloc_ = r & 1;
    r >>= 1;
    int bin = r & 127;
    r >>= 7;
    unsigned int size = r & 65535;
    r >>= 16;
    int useShifts = r & 1;
    r >>= 1;
    unsigned int shifts = r & 15;
    r >>= 4;
    if (size == 0) size = 1;
    if (useShifts) {
      size >>= shifts; // spread out values logarithmically
    }
    if (alloc || !bins[bin]) {
      if (bins[bin]) {
        char value = values[bin];
        assert(*(char*)(bins[bin]) == value /* one */);
        bins[bin] = realloc(bins[bin], size);
        if (bins[bin]) {
          assert(*(char*)(bins[bin]) == value /* two */);
        }
      } else {
        if (calloc_) {
          bins[bin] = malloc(size);
        } else {
          bins[bin] = calloc(size, 1);
        }
        values[bin] = random();
        if (bins[bin]) {
          *(char*)(bins[bin]) = values[bin];
          assert(*(char*)(bins[bin]) == values[bin] /* three */);
        }
      }
    } else {
      free(bins[bin]);
      bins[bin] = NULL;
    }
  }
  for (int i = 0; i < BINS; i++) {
    if (bins[i]) free(bins[i]);
  }
  // it's all freed, should be a blank slate
  assert(check_where_we_would_malloc(10) == start);
}

int main() {
  stage("beginning");

  basics();
  blank_slate();
  previous_sbrk();
  min_alloc();
  space_at_end();
  calloc();
  realloc();
  aligned();
  randoms();

  stage("the_end");
}

