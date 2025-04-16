#include <emscripten.h>

EM_JS(void, foobar, (), {console.error("foobar called");});

void(*func_ptr)() = &foobar;

int main() {
  foobar();
  func_ptr();
  return 0;
}
