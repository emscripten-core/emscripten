#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <wasm_simd128.h>

int EMSCRIPTEN_KEEPALIVE main(int argc, char** argv) {
  v128_t a = wasm_i8x16_const_splat(1);
  v128_t b = wasm_i8x16_const_splat(2);
  v128_t c = wasm_i8x16_const_splat(3);
  assert(wasm_i8x16_all_true(wasm_i8x16_eq(wasm_i8x16_add(a, b), c)));
  printf("Success!\n");
}
