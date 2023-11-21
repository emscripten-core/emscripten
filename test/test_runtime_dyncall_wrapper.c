#include <emscripten.h>
#include "stdint.h"
#include "stdio.h"

uint64_t f1(uint64_t x) {
  return x;
}

void f2(int i, uint64_t j, float f, double d) {
  printf("i: %d j: %lld f: %f d: %lf\n", i, j, f, d);
}

void f3(uint64_t j1, uint64_t j2, uint64_t j3) {
  printf("j1: %lld, j2: %lld, j3: %lld\n", j1, j2, j3);
}


int main() {
  EM_ASM({
    var w = createDyncallWrapper("jj");
    console.log(w($0, 2, 7), getTempRet0());
  }, f1);

  EM_ASM({
    var w = createDyncallWrapper("vijfd");
    w($0, 2, 7, 2, 3.12, 77.12);
  }, f2);

  EM_ASM({
    var w = createDyncallWrapper("vjjj");
    w($0, 7, 2, 2, 7, 3, 3);
  }, f3);
}
