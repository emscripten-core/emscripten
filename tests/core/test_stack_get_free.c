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
  printf("free: %u\n", free);
  printf("free2: %u\n", free2);
  assert(used + free == total);
  assert(free == free2);
}

int main() {
  printf("Stack free: %u\n", emscripten_stack_get_free());
  TestStackValidity();

  uintptr_t prevFree = emscripten_stack_get_free();
  for(int i = 0; i < 10; ++i) {
    void *p = alloca(emscripten_random() >= 0 ? 256*1024 : 255 * 1024);
    uintptr_t free = emscripten_stack_get_free();
    assert(prevFree - free == 256*1024);
    prevFree = free;
    DoSomething(p);
    printf("Stack free: %u\n", emscripten_stack_get_free());
    TestStackValidity();
  }
  return 0;
}
