// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <emscripten.h>
#include <emscripten/emmalloc.h>

#ifndef RANDOM_ITERS
#define RANDOM_ITERS 12345
#endif

void emmalloc_blank_slate_from_orbit();

void stage(const char* name) {
  // Using printf here over out, at least until we can fix
  // https://github.com/emscripten-core/emscripten/issues/14804
  printf(">> %s\n", name);
  /*
  EM_ASM({
    out('>> ' + UTF8ToString($0) + '\n');
  }, name);
  */
}

void basics() {
  stage("basics");
  stage("allocate 0");
  void* ptr = malloc(0);
  assert(ptr != 0);
  free(ptr);
  stage("allocate 100");
  void* first = malloc(100);
  stage("free 100");
  free(first);
  stage("allocate another 100");
  void* second = malloc(100);
  stage("allocate 10");
#ifndef WASMFS
  // There is no strict guarantee that the second allocation be equal to the
  // first, but in practice on a fresh heap that tends to be the case. With
  // WasmFS, however, the heap has already seen a bunch of use by the time we
  // get here, and due to fragmentation etc. we cannot predict getting the exact
  // same result the second time.
  assert(second == first);
#endif
  void* third = malloc(10);
  assert(!emmalloc_validate_memory_regions());
  stage("allocate 10 more");
  void* four = malloc(10);
  assert(!emmalloc_validate_memory_regions());
  stage("free the first");
  free(second);
  stage("free all");
  free(third);
  free(four);
  assert(!emmalloc_validate_memory_regions());
}

void previous_sbrk() {
  stage("previous_sbrk");
  emmalloc_blank_slate_from_orbit();
  void* old = sbrk(0);
  assert((size_t)old % 4 == 0);
  sbrk(3); // unalign things
  void* other = malloc(10);
  free(other);
  assert(other != old);
}

void test_calloc() {
  stage("calloc");
  emmalloc_blank_slate_from_orbit();
  char* ptr = (char*)malloc(10);
  ptr[0] = 77;
  free(ptr);
  char* cptr = (char*)calloc(10, 1);
  assert(cptr == ptr);
  assert(ptr[0] == 0);
}

void test_realloc() {
  stage("realloc0");
  emmalloc_blank_slate_from_orbit();
  for (int i = 0; i < 2; i++) {
    char* ptr = (char*)malloc(100);
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

  // realloc of NULL is like malloc
  assert(realloc(NULL, 10) != 0);

  stage("realloc2");
  emmalloc_blank_slate_from_orbit();
  {
    // realloc to 0 is like free
    void* ptr = malloc(10);
    assert(realloc(ptr, 0) == NULL);
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
  if (align != 0 && ((align & (align - 1)) != 0)) {
    assert(ptr == 0);
  } else {
    assert(ptr);
    assert(align == 0 || ptr % align == 0);
  }
}

void aligned() {
  stage("aligned");
  for (int i = 0; i < 35; i++) {
    for (int j = 0; j < 35; j++) {
      emmalloc_blank_slate_from_orbit();
      size_t first = (size_t)memalign(i, 100);
      size_t second = (size_t)memalign(j, 100);
      printf("%d %d => %zu %zu\n", i, j, first, second);
      check_aligned(i, first);
      check_aligned(j, second);
    }
  }
}

void randoms() {
  stage("randoms");
  emmalloc_blank_slate_from_orbit();
  void* start = malloc(10);
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
  assert(!emmalloc_validate_memory_regions());
}

int main() {
  stage("beginning");

  basics();
  previous_sbrk();
  test_calloc();
  test_realloc();
  aligned();
  randoms();

  stage("the_end");
}

