#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <emscripten/em_asm.h>

int main() {
  int64_t num = (2LL << 55) | 42LL;
  printf("native = %lld\n", num);

  EM_ASM({
    console.log("js     = " + $0);
  }, num);

  // EM_ASM doesn't currently have any supprot for unsigned values so UINT64_MAX
  // will show up on the JS side -1.
  uint64_t unsigned_num = UINT64_MAX;
  printf("native = %llu\n", unsigned_num);

  EM_ASM({
    console.log("js     = " + $0);
  }, unsigned_num);
}
