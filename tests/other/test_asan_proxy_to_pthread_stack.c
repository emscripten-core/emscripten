#include <stdint.h>
#include <emscripten.h>
int main(void) {
  // Allocate 64K on the stack, verifying that TOTAL_STACK is big enough and is
  // used by PROXY_TO_PTHREAD.
  int32_t data[64*1024];
  // Write, to verify it is in valid memory.
  data[0] = data[64*1024 - 1] = 0;
}
