#include <wasm_simd128.h>
#include <stdio.h>
#include <stdio.h>
#include <emscripten/bind.h>

void print_madd() {
  v128_t v1 = wasm_f32x4_make(1.2f, 3.4f, 5.6f, 7.8f);
  v128_t v2 = wasm_f32x4_make(2.1f, 4.3f, 6.5f, 8.7f);
  v128_t v3 = wasm_f32x4_make(3.1f, 5.3f, 8.5f, 9.7f);
  v128_t v4 = wasm_f32x4_relaxed_madd(v1, v2, v3);
  printf("v3: [%.1f, %.1f, %.1f, %.1f]\n",
         wasm_f32x4_extract_lane(v4, 0),
         wasm_f32x4_extract_lane(v4, 1),
         wasm_f32x4_extract_lane(v4, 2),
         wasm_f32x4_extract_lane(v4, 3));
}

int main() {
  print_madd();
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("print_madd", &print_madd);
}
