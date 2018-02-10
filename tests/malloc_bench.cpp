#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sbrk()

const int BINS = 32768;
const int BIN_MASK = BINS - 1;
const int ITERS = 6 * 1024 * 1024;
//   8, 32: emmalloc slower
//  12, 28: emmalloc much sbrkier and also slower
// 256, 512: emmalloc much faster
const int MIN_SIZE = 12;
const int MAX_SIZE = 28;
const int SIZE_MASK = 0;
const bool POLL_SBRK = false;
const bool USE_REALLOC_UP = true;
const bool USE_REALLOC_DOWN = true;
const bool USE_CALLOC = false;

void randoms() {
  srandom(1);
  size_t before = (size_t)sbrk(0);
  double sum_sbrk = 0;
  size_t max_sbrk = before;
  void* bins[BINS];
  size_t allocated[BINS];
  size_t total_allocated = 0;
  size_t max_allocated = 0;
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
        if (calloc_ && USE_CALLOC) {
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
  printf("max allocated:    %u\n", max_allocated);
  printf("sbrk change:      %u\n", after - before);
  if (POLL_SBRK) {
    printf("sbrk mean change: %.2f\n", (sum_sbrk / double(ITERS)) - before);
    printf("sbrk max change:  %u\n", max_sbrk - before);
  }
}

int main() {
  randoms();
}

// ./emcc tests/malloc_bench.cpp -s TOTAL_MEMORY=68157440 -O2 -s WASM=1 system/lib/dlmalloc.c && time mozjs a.out.js && ./emcc tests/malloc_bench.cpp -O2 -s WASM=1 system/lib/emmalloc.cpp -DNDEBUG -o b.out.js -s TOTAL_MEMORY=68157440 && time mozjs b.out.js && ls -al a.out.wasm b.out.wasm

/*

Likely Havlak does many tiny tiny allocations?

Running test_benchmark: (5 tests)
Running Emscripten benchmarks... [ ignoring compilation | Wed Feb  7 13:09:33 2018 | em: commit 617f73c29b55bd0c1ef01ef5b45460b06591ca0c | llvm: /home/alon/Dev/fastcomp/cbuild/bin/ ]
test_havlak (test_benchmark.benchmark) ... 
     dlmalloc: mean: 1.911 (+-0.009) secs  median: 1.905  range: 1.899-1.922  (noise: 0.492%)  (3 runs) 
        size:    81799, compressed:    29554 
     emmalloc: mean: 2.641 (+-0.009) secs  median: 2.636  range: 2.628-2.650  (noise: 0.352%)  (3 runs)   Relative: 1.38 X slower
        size:    75838, compressed:    26941 
ok
test_zzz_lua_binarytrees (test_benchmark.benchmark) ... 
<building and saving lua_O3_fc3ec088c7307cf129d2701bf7e6ec07js__home_alon_Dev_fastcomp_cbuild_bin_ into cache> 
     dlmalloc: mean: 5.225 (+-0.073) secs  median: 5.184  range: 5.129-5.305  (noise: 1.391%)  (3 runs) 
        size:   389603, compressed:   130800 
<load lua_O3_fc3ec088c7307cf129d2701bf7e6ec07js__home_alon_Dev_fastcomp_cbuild_bin_ from cache> 
     emmalloc: mean: 5.767 (+-0.074) secs  median: 5.726  range: 5.670-5.849  (noise: 1.282%)  (3 runs)   Relative: 1.10 X slower
        size:   381811, compressed:   127424 
ok

alon@determincy:/tmp/emscripten_temp$ mozjs dlmalloc_havlak.js 
main() took 869 milliseconds
main() caused an 17584176 sbrk increase
alon@determincy:/tmp/emscripten_temp$ mozjs emmalloc_havlak.js 
main() took 1205 milliseconds
main() caused an 28340144 sbrk increase
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ mozjs dlmalloc_box2d.js 
main() took 1225 milliseconds
main() caused an 1510544 sbrk increase
alon@determincy:/tmp/emscripten_temp$ mozjs emmalloc_box2d.js 
main() took 1214 milliseconds
main() caused an 1508720 sbrk increase
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ mozjs dlmalloc_bullet.js 
main() took 1443 milliseconds
main() caused an 43442176 sbrk increase
alon@determincy:/tmp/emscripten_temp$ mozjs emmalloc_bullet.js 
main() took 1447 milliseconds
main() caused an 42914640 sbrk increase
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ mozjs dlmalloc_lua_binarytrees.c.js 
main() took 5173 milliseconds
main() caused an 27770496 sbrk increase
alon@determincy:/tmp/emscripten_temp$ mozjs emmalloc_lua_binarytrees.c.js 
main() took 5728 milliseconds
main() caused an 36987888 sbrk increase
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ 
alon@determincy:/tmp/emscripten_temp$ mozjs dlmalloc_zlib.c.js 
main() took 1444 milliseconds
main() caused an 569136 sbrk increase
alon@determincy:/tmp/emscripten_temp$ mozjs emmalloc_zlib.c.js 
main() took 1423 milliseconds
main() caused an 568160 sbrk increase

havlak
alon@florida:/tmp/emscripten_temp$ grep "malloc" o | wc -l
2258745
alon@florida:/tmp/emscripten_temp$ grep "malloc 12\." o | wc -l
 948951
alon@florida:/tmp/emscripten_temp$ grep "malloc 20\." o | wc -l
 396242
alon@florida:/tmp/emscripten_temp$ grep "malloc 16\." o | wc -l
 323768
alon@florida:/tmp/emscripten_temp$ grep "malloc 8\." o | wc -l
 241854
alon@florida:/tmp/emscripten_temp$ grep "malloc 4\." o | wc -l
 100826
alon@florida:/tmp/emscripten_temp$ grep "malloc 28\." o | wc -l
  50413
alon@florida:/tmp/emscripten_temp$ grep "malloc 11\." o | wc -l
  15000

dlmalloc, align 16                 align 8      emmalloc  rotated
48 => 66                           56.1           56        56
44                                 48             56        52
40                                 48.1           48        48
36                                 40.1           48        44
32 => 49                           41             40        40
28                                 32.05          40        36
24                                 32.05          32        32
20 => 35                           24.1           32*       28
16 => 35                           24.1           24        24
12 => 18                           16.15          24*       20
 8 => 18                           16.15          16        16
 4 => 18                           16.15          16        16

*/

