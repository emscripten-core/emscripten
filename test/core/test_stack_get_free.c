#include <stdlib.h>
#include <stdio.h>
#include <alloca.h>
#include <emscripten.h>
#include <emscripten/stack.h>
#include <assert.h>
#include <string.h>

void __attribute__((noinline)) DoSomething(char *addr) {
  memset(addr, 42, 13);
}

void TestStackValidity() {
  uintptr_t base = emscripten_stack_get_base();
  uintptr_t end = emscripten_stack_get_end();

  uintptr_t used = labs((intptr_t)base - (intptr_t)emscripten_stack_get_current());
  uintptr_t free = labs((intptr_t)end - (intptr_t)emscripten_stack_get_current());
  uintptr_t free2 = emscripten_stack_get_free();
  uintptr_t total = labs((intptr_t)end - (intptr_t)base);
  assert(used + free == total);
  assert(free == free2);
}

int increment = 256 * 1024;

int main() {
  emscripten_stack_init();
  TestStackValidity();

  uintptr_t origFree = emscripten_stack_get_free();
  uintptr_t prevFree = emscripten_stack_get_free();
  printf("Stack used: %lu\n", origFree - emscripten_stack_get_free());
  for (int i = 0; i < 10; ++i) {
    int increment_noopt = emscripten_random() >= 0 ? increment : 2;
    char *p = alloca(increment_noopt);
    DoSomething(p);
    uintptr_t free = emscripten_stack_get_free();
    assert(prevFree - free == increment);
    prevFree = free;
    // Print something from the allocationed region to prevent whole program
    // optimizations from elminiating the alloca completely.
    printf("Val: %d\n", p[10]);
    printf("Stack used: %zu\n", origFree - emscripten_stack_get_free());
    TestStackValidity();
  }
  return 0;
}
