#include <emscripten.h>
#include <stdio.h>

EM_JS(void, foo, (void), {
  err("hello");
});

// Take the address of foo and store it in a global.
void (*f)(void) = &foo;

int main() {
  printf("main\n");
  // Calling en EM_JS function by its address, without also importing
  // as a normal function, doesn't yet work with dynamic linking and this
  // call will fail with `Missing signature argument to addFunction`.
  f();
  printf("done\n");
  return 0;
}
