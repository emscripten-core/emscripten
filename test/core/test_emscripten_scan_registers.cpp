#include <set>
#include <emscripten.h>
#include <stdio.h>
#include <assert.h>

std::set<int> seenInts;

static int scans = 0;

#define DO_SCAN { emscripten_scan_registers(scan); emscripten_scan_stack(scan); }

void scan(void* x, void* y) {
  printf("scan\n");
  int* p = (int*)x;
  int* q = (int*)y;
  while (p < q) {
    seenInts.insert(*p);
    p++;
  }
  scans++;
}

__attribute__((noinline))
void inner(int x, int y) {
  if (x == y) inner(x + 1, y - 1); // avoid inlining in binaryen
  DO_SCAN
  printf("a %d, %d\n", x, y);
  assert(seenInts.count(314159));
  assert(seenInts.count(21828));
  assert(seenInts.size() < 1000);
  seenInts.clear();
  int z;
  if (x < y) {
    printf("left..\n");
    z = x + 100;
    DO_SCAN
    printf("..left\n");
  } else {
    printf("right..\n");
    z = y + 200;
    DO_SCAN
    printf("..right\n");
  }
  printf("b %d, %d, %d\n", x, y, z);
  assert(seenInts.count(314159));
  assert(seenInts.count(21828));
  assert(seenInts.count(22028));
  assert(seenInts.size() < 1000);
  seenInts.clear();
}

int main() {
  int x = EM_ASM_INT({ return 314159 });
  int y = EM_ASM_INT({ return 21828 });
  inner(x, y);
  x = EM_ASM_INT({ return $0 + 1 }, x);
  y = EM_ASM_INT({ return $0 - 1 }, y);
  DO_SCAN
  printf("c %d, %d\n", x, y);
  assert(seenInts.count(314160));
  assert(seenInts.count(21827));
  assert(seenInts.size() < 1000);
  assert(scans == 6);
  puts("ok");

  // Also test a very large amount of scans in a loop, to check that we avoid
  // unbounded call stack growth.
  for (int i = 0; i < 100; i++) {
    DO_SCAN
  }
}
