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
    Module.print('\n>> ' + Pointer_stringify($0) + '\n');
  }, name);
}

const size_t MIN_ALLOC = 8;
const size_t ALLOC_FACTOR = 4;
const size_t METADATA = 8;

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
  assert(size_t(third) == size_t(first) + ((100 + MIN_ALLOC - 1)&(-MIN_ALLOC)) + MIN_ALLOC); // allocation units are multiples of MIN_ALLOC
  stage("allocate 10 more");
  void* four = malloc(10);
  assert(size_t(four) == size_t(third) + (2*MIN_ALLOC) + MIN_ALLOC); // payload (10 = 2 allocation units) and metadata
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
  stage("8/4 issues");
  for (int k = 0; k < 2; k++) {
    for (int i = 1; i < 20; i++) {
      for (int j = 1; j < 20; j++) {
        emmalloc_blank_slate_from_orbit();
        void* pre = NULL;
        if (k) pre = malloc(4);
        void* first = malloc(i);
        char* second = (char*)first;
        // 8 allocated bytes must be 8-byte aligned, less is just 4-byte
        if (i < 8) {
          assert(size_t(first) % 4 == 0);
          second += 8; // minimum allocation unit is 8
        } else {
          assert(size_t(first) % 8 == 0);
          second += (i + 3) & -4; // first payload is aligned to a muliple of 4
        }
        second += 8; // metadata, fixed size
        if (j >= 8) {
          // if second needs 8-byte alignment, it may add 4 to ensure that
          if (size_t(second) % 8 != 0) {
            second += 4;
          }
        }
        check_where_we_would_malloc(j, second);
        free(first);
        free(pre);
      }
    }
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
  while (size_t(sbrk(0)) % MIN_ALLOC != 0) {
    sbrk(1);
  }
  void* old = sbrk(0);
  sbrk(3); // unalign things
  void* other = malloc(10);
  free(other);
  assert(other != old);
  assert((char*)other == (char*)old + 2 * MIN_ALLOC);
}

void min_alloc() {
  stage("min_alloc");
  for (int i = 1; i < 100; i++) {
    emmalloc_blank_slate_from_orbit();
    void* start = check_where_we_would_malloc(1);
    void* temp = malloc(i);
    char* expected = (char*)start + METADATA;
    if (i < MIN_ALLOC) {
      expected += MIN_ALLOC;
    } else {
      expected += ALLOC_FACTOR * ((i + ALLOC_FACTOR - 1) / ALLOC_FACTOR);
    }
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
  stage("realloc");
  emmalloc_blank_slate_from_orbit();
  for (int i = 0; i < 2; i++) {
    char* ptr = (char*)malloc(10);
    char* raptr = (char*)realloc(ptr, 1);
    assert(raptr == ptr);
    char* raptr2 = (char*)realloc(ptr, 100);
    assert(raptr2 == ptr);
    char* last = (char*)malloc(1);
    assert(last >= ptr + 100);
    // slightly more still fits
    char* raptr3 = (char*)realloc(ptr, 11);
    assert(raptr3 == ptr);
    // finally, realloc a size we must reallocate for
    char* raptr4 = (char*)realloc(ptr, 1000);
    assert(raptr4);
    assert(raptr4 != ptr);
    // leaving those in place, do another iteration
  }
  emmalloc_blank_slate_from_orbit();
  {
    // realloc of NULL is like malloc
    void* ptr = check_where_we_would_malloc(10);
    assert(realloc(NULL, 10) == ptr);
  }
  emmalloc_blank_slate_from_orbit();
  {
    // realloc to 0 is like free
    void* ptr = malloc(10);
    assert(realloc(ptr, 0) == NULL);
    assert(check_where_we_would_malloc(10) == ptr);
  }
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
  randoms();

  stage("the_end");
}

