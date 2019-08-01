#include <set>
#include <emscripten.h>
#include <stdio.h>

std::set<int> seenInts;

void scan(void* x, void* y) {
  printf("scan\n");
  int* p = (int*)x;
  int* q = (int*)y;
  while (p < q) {
    //printf("  %d\n", *p);
    seenInts.insert(*p);
    p++;
  }
}

int main() {
  int x = EM_ASM_INT({ return 314159 });
  int y = EM_ASM_INT({ return 21828 });
  emscripten_scan_registers(scan);
  printf("a %d, %d\n", x, y);
  assert(seenInts.count(314159));
  assert(seenInts.count(21828));
  assert(seenInts.count(314160) == 0); // in theory these two might be nonzero by chance
  assert(seenInts.count(21827) == 0);
  x = EM_ASM_INT({ return $0 + 1 }, x);
  y = EM_ASM_INT({ return $0 - 1 }, y);
  emscripten_scan_registers(scan);
  printf("b %d, %d\n", x, y);
  assert(seenInts.count(314160));
  assert(seenInts.count(21827));
  puts("ok");
}

