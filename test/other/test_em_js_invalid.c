#include <emscripten/em_js.h>

EM_JS(int, foo, (), {
  * this is not valid js *
});

int main() {
  foo();
}
