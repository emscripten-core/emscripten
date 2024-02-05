#include <stdint.h>
#include <emscripten.h>

EM_JS(void, foo, (uint64_t a, int64_t b), {
  out(typeof a);
  out(typeof b);

  out('a:' + a);
  out('b:' + b);
})

int main() {
  foo(42000000000ull, -42ll);
}
