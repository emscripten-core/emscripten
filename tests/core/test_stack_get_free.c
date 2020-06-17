#include <stdlib.h>
#include <stdio.h>
#include <alloca.h>
#include <emscripten.h>
#include <emscripten/stack.h>
#include <assert.h>
#include <string.h>

void __attribute__((noinline)) DoSomething(void *addr) {
  memcpy(addr, addr + 42, 13);
}

void TestStackValidity() {
  uintptr_t base = emscripten_stack_get_base();
  uintptr_t end = emscripten_stack_get_end();

  uintptr_t used = abs((intptr_t)base - (intptr_t)emscripten_stack_get_current());
  uintptr_t free = abs((intptr_t)end - (intptr_t)emscripten_stack_get_current());
  uintptr_t free2 = emscripten_stack_get_free();
  uintptr_t total = abs((intptr_t)end - (intptr_t)base);
  assert(used + free == total);
  assert(free == free2);
}

int increment = 256 * 1024;

int main() {
  TestStackValidity();

  uintptr_t origFree = emscripten_stack_get_free();
  uintptr_t prevFree = emscripten_stack_get_free();
  printf("Stack used: %u\n", origFree - emscripten_stack_get_free());
  for(int i = 0; i < 10; ++i) {
    void *p = alloca(increment);
    uintptr_t free = emscripten_stack_get_free();
    assert(prevFree - free == increment);
    prevFree = free;
    DoSomething(p);
    printf("Stack used: %u\n", origFree - emscripten_stack_get_free());
    TestStackValidity();
  }
  return 0;
}
