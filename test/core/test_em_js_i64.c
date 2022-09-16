#include <stdint.h>
#include <emscripten.h>

EM_JS(void, foo, (uint64_t a, int64_t b), {
  console.log(typeof a);
  console.log(typeof b);

  console.log('a:' + a);
  console.log('b:' + b);
})

int main() {
  foo(42000000000ull, -42ll);
}
