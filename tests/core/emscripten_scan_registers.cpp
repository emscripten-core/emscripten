#include <set>
#include <emscripten.h>
#include <stdio.h>

std::set<int> seenInts;

static int scans = 0;

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
  emscripten_scan_registers(scan);
  printf("a %d, %d\n", x, y);
  assert(seenInts.count(314159));
  assert(seenInts.count(21828));
  assert(seenInts.size() < 1000);
  seenInts.clear();
  int z;
  if (x < y) {
    printf("left..\n");
    z = x + 100;
    emscripten_scan_registers(scan);
    printf("..left\n");
  } else {
    printf("right..\n");
    z = y + 200;
    emscripten_scan_registers(scan);
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
  emscripten_scan_registers(scan);
  printf("c %d, %d\n", x, y);
  assert(seenInts.count(314160));
  assert(seenInts.count(21827));
  assert(seenInts.size() < 1000);
  assert(scans == 3);
  puts("ok");
}

