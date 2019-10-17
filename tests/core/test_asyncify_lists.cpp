#include <assert.h>
#include <stdio.h>
#include <emscripten.h>

volatile int x;

int bar();

// Foo is a function that needs asyncify support.
__attribute__((noinline))
int foo(int a = 0, double b = 0) {
  if (x == 1337) return bar(); // don't inline me
  assert(a == b);
  emscripten_sleep(1);
  return 1;
}

// Bar does not.
__attribute__((noinline))
int bar() {
  if (x == 1337) return foo(); // don't inline me
  return 2;
}

// C++ names
struct Structy {
  __attribute__((noinline))
  int funcy() {
    if (x == 1337) return funcy(); // don't inline me
    emscripten_sleep(1);
    return 3;
  }
};

// Baz does, because it calls foo.
__attribute__((noinline))
void baz() {
  if (x == 1337) {
    // don't inline me
    foo();
    bar();
    baz();
  }
  puts("baz");
  printf("foo: %d\n", foo());
  printf("bar: %d\n", bar());
  printf("c++: %d\n", Structy().funcy());
}

// Ditto, with extern C
__attribute__((noinline))
extern "C" void c_baz() {
  if (x == 1337) {
    // don't inline me
    foo();
    bar();
    baz();
  }
  puts("c_baz");
  printf("foo: %d\n", foo());
  printf("bar: %d\n", bar());
  printf("c++: %d\n", Structy().funcy());
}

int main() {
  baz();
  EM_ASM({
    Module.counter = (Module.counter || 0) + 1;
    if (Module.counter > 10) throw "infinite loop?";
  });
  c_baz();
}

