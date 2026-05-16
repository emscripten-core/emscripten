#include <emscripten.h>

struct Pair { int a; int b; };

// Returning a small struct by value with the experimental multi-value ABI
// causes clang to emit a wasm function with two return values (i32, i32).
// This mirrors the shape that higher-level toolchains (e.g. wasm-bindgen)
// produce when lowering compound types to (ptr, len) pairs.
EMSCRIPTEN_KEEPALIVE struct Pair make_pair(int a, int b) {
  struct Pair p = {a, b};
  return p;
}

EMSCRIPTEN_KEEPALIVE int add(int a, int b) {
  return a + b;
}

int main() {}
