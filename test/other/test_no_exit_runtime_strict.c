#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/em_js.h>
#include <stdio.h>

// Verify that `keepRuntimeAlive()` always returns true by default (i.e. when
// EXIT_RUNTIME=0).

EM_JS_DEPS(deps, "$keepRuntimeAlive");

EM_JS(void, timeout_func, (), {
  console.log("timeout_func: keepRuntimeAlive() ->", keepRuntimeAlive());
  // once this timeout is done the node process should exit with 0
});

int main() {
  int keep_alive = EM_ASM_INT({
    setTimeout(timeout_func);
    return keepRuntimeAlive();
  });
  printf("main done: %d\n", keep_alive);
  assert(keep_alive == 1);
  return 0;
}
