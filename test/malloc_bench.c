// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Used internally to test performance of emmalloc against other
// malloc implementations.  (Not run as part of the emscripten test suite).

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for sbrk()

const int BINS = 32768;
const int BIN_MASK = BINS - 1;
const int ITERS = 6 * 1024 * 1024;
//  12, 64: emmalloc slower
//  12, 28: emmalloc much sbrkier and also slower
// 256, 512: emmalloc faster without USE_MEMORY
const int MIN_SIZE = 16;
const int MAX_SIZE = 64;
const int SIZE_MASK = 7;
const bool POLL_SBRK = false;
const bool USE_REALLOC_UP = true;
const bool USE_REALLOC_DOWN = true;
const bool USE_CALLOC = false;
const bool USE_MEMORY = true;
const bool USE_SHIFTS = false;

void randoms() {
  srandom(1);
  size_t before = (size_t)sbrk(0);
  double sum_sbrk = 0;
  size_t max_sbrk = before;
  void* bins[BINS];
  size_t allocated[BINS];
  size_t total_allocated = 0;
  size_t max_allocated = 0;
  size_t checksum = 0;
  size_t sizes = 0;
  size_t allocations = 0;
  for (int i = 0; i < BINS; i++) {
    bins[i] = NULL;
  }
  for (int i = 0; i < ITERS; i++) {
    int bin = random() & BIN_MASK;
    unsigned int r = random();
    int alloc = r & 1;
    r >>= 1;
    int calloc_ = r & 1;
    r >>= 1;
    unsigned int size = r & 65535;
    r >>= 16;
    int useShifts = r & 1;
    r >>= 1;
    unsigned int shifts = r & 15;
    r >>= 4;
    if (MAX_SIZE) {
      size = size % (MAX_SIZE + 1);
    }
    if (USE_SHIFTS && useShifts) {
      size >>= shifts; // spread out values logarithmically
    }
    if (SIZE_MASK) size = size & ~SIZE_MASK;
    if (MIN_SIZE && size < MIN_SIZE) size = MIN_SIZE;
    if (MAX_SIZE && size > MAX_SIZE) size = MAX_SIZE;
    //printf("%d\n", size);
    if (alloc || !bins[bin]) {
      if (bins[bin]) {
        bool up = size >= allocated[bin];
        if ((up && USE_REALLOC_UP) || (!up && USE_REALLOC_DOWN)) {
          total_allocated -= allocated[bin];
          bins[bin] = realloc(bins[bin], size);
          allocated[bin] = size;
          total_allocated += size;
        } else {
          // malloc and free manually
          free(bins[bin]);
          bins[bin] = NULL;
          total_allocated -= allocated[bin];
          allocated[bin] = 0;
          bins[bin] = malloc(size);
          allocated[bin] = size;
          total_allocated += size;
        }
      } else {
        if (calloc_ && USE_CALLOC) {
          bins[bin] = calloc(size, 1);
          allocated[bin] = size;
          total_allocated += size;
        } else {
          bins[bin] = malloc(size);
          allocated[bin] = size;
          total_allocated += size;
        }
      }
      if (bins[bin]) {
        allocations++;
        sizes += size;
      }
      if (USE_MEMORY && bins[bin]) {
        for (int i = 0; i < size; i++) {
          ((char*)(bins[bin]))[i] = i;
        }
      }
    } else {
      if (USE_MEMORY && bins[bin]) {
        for (int i = 0; i < size; i++) {
          checksum += ((char*)(bins[bin]))[i];
        }
      }
      free(bins[bin]);
      bins[bin] = NULL;
      total_allocated -= allocated[bin];
      allocated[bin] = 0;
    }
    if (total_allocated > max_allocated) {
      max_allocated = total_allocated;
    }
    if (POLL_SBRK) {
      size_t curr = (size_t)sbrk(0);
      if (curr > max_sbrk) max_sbrk = curr;
      sum_sbrk += curr;
    }
  }
  for (int i = 0; i < BINS; i++) {
    if (bins[i]) {
      free(bins[i]);
      total_allocated -= allocated[i];
    }
  }
  size_t after = (size_t)sbrk(0);
  printf("checksum:         %zx\n", checksum);
  printf("allocations:      %zu\n", allocations);
  printf("mean alloc size:  %.2f\n", ((double)sizes) / allocations);
  printf("max allocated:    %zu\n", max_allocated);
  double allocs_at_max = max_allocated / (((double)sizes) / allocations);
  printf("allocations #max  %.2f\n", allocs_at_max);
  size_t sbrk_change = after - before;
  printf("sbrk chng:        %zu\n", sbrk_change);
  printf("sbrk chng/allocs: %.2f\n", sbrk_change / ((double)allocs_at_max));
  printf("overhead:         %.2f\n", -((((double)sizes) / allocations) - (sbrk_change / ((double)allocs_at_max))));
  printf("sbrk top now:     %p\n", (void*)sbrk(0));
  if (POLL_SBRK) {
    printf("sbrk mean change: %.2f\n", (sum_sbrk / (double)ITERS) - before);
    printf("sbrk max change:  %lu\n", max_sbrk - before);
  }
}

int main() {
  randoms();
}
