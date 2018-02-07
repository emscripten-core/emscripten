#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sbrk()

const int BINS = 256; // Max 256 (8 bits)
const int BIN_MASK = BINS - 1;
const int ITERS = 2 * 1024 * 1024;
const int MIN_SIZE = 8; // 1 is somewhat bad, tiny allocations are not good for us
const int MAX_SIZE = 4096;
const int SIZE_MASK = 0;
const bool POLL_SBRK = false;
const bool USE_REALLOC_UP = true; // the bad one!
const bool USE_REALLOC_DOWN = false;

void randoms() {
  srandom(1);
  size_t before = (size_t)sbrk(0);
  size_t max_sbrk = before;
  void* bins[BINS];
  size_t allocated[BINS];
  size_t total_allocated = 0;
  size_t max_allocated = 0;
  for (int i = 0; i < BINS; i++) {
    bins[i] = NULL;
  }
  for (int i = 0; i < ITERS; i++) {
    unsigned int r = random();
    int alloc = r & 1;
    r >>= 1;
    int calloc_ = r & 1;
    r >>= 1;
    int bin = r & BIN_MASK;
    r >>= 8;
    unsigned int size = r & 65535;
    r >>= 16;
    int useShifts = r & 1;
    r >>= 1;
    unsigned int shifts = r & 15;
    r >>= 4;
    if (useShifts) {
      size >>= shifts; // spread out values logarithmically
    }
    if (SIZE_MASK) size = size & ~SIZE_MASK;
    if (MIN_SIZE && size <= MIN_SIZE) size = MIN_SIZE;
    if (MAX_SIZE && size >= MAX_SIZE) size = MAX_SIZE;
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
        if (calloc_) {
          bins[bin] = malloc(size);
          allocated[bin] = size;
          total_allocated += size;
        } else {
          bins[bin] = calloc(size, 1);
          allocated[bin] = size;
          total_allocated += size;
        }
      }
    } else {
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
    }
  }
  for (int i = 0; i < BINS; i++) {
    if (bins[i]) {
      free(bins[i]);
      total_allocated -= allocated[i];
    }
  }
  size_t after = (size_t)sbrk(0);
  printf("max allocated:   %u   (total left should be 0: %u)\n", max_allocated, total_allocated); 
  printf("sbrk change:     %u\n", after - before);
  if (POLL_SBRK) {
    printf("sbrk max change: %u\n", max_sbrk - before);
  }
}

int main() {
  randoms();
}

