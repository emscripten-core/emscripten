#include <stdio.h>
#include <emscripten/em_asm.h>
#include <emscripten.h>

__asm__(
".section .data.my_global,\"\",@\n"
".globl my_global\n"
".globaltype my_global, i32\n"
"my_global:\n"
);

int get_global() {
  int val;
  // Without volatile here this test fails in O1 and above.
  __asm__ volatile ("global.get my_global\n"
                    "local.set %0\n" : "=r" (val));
  return val;
}

void set_global(int val) {
  __asm__("local.get %0\n"
          "global.set my_global\n" : : "r" (val));
}

int main() {
  printf("in main: %d\n", get_global());
  set_global(42);
  printf("new value: %d\n", get_global());
  EM_ASM({
      // With the ESM integration, the Wasm global be exported as a regular
      // number.  Otherwise it will be a WebAssembly.Global object.
#ifdef ESM_INTEGRATION
      assert(typeof _my_global == 'number', typeof _my_global);
      out('from js:', _my_global);
      _my_global += 1
#else
      assert(typeof _my_global == 'object', typeof _my_global);
      out('from js:', _my_global.value);
      _my_global.value += 1
#endif
  });
  printf("done: %d\n", get_global());
}
