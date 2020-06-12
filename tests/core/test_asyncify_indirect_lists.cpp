#include <assert.h>
#include <stdio.h>
#include <emscripten.h>

int x = 0;

struct Structy {
  virtual int virty() {
    if (x == 1337) return virty(); // don't inline me
    emscripten_sleep(1);
    return 42;
  }
};

__attribute__((noinline))
void virt() {
  if (x == 1337) {
    // don't inline me
    virt();
  }
  Structy x;
  Structy *y = &x;
  printf("virt: %d\n", y->virty()); // but the indirect call itself!
}

int main() {
  EM_ASM({
    Module.counter = (Module.counter || 0) + 1;
    if (Module.counter > 10) throw "infinite loop due to asyncify bug?";
  });
  virt();
}
