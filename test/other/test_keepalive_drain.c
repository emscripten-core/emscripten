#include <emscripten.h>
#include <emscripten/em_asm.h>
#include <stdio.h>
#include <stdlib.h>

// Suspends and never resumes: a pending operation with no producer.
EM_ASYNC_JS(void, suspend_forever, (), { await new Promise(() => {}); });

void exit_cb(void* arg) {
  printf("done\n");
  exit(0);
}

void force_exit_cb(void* arg) {
  printf("done\n");
  emscripten_force_exit(0);
}

int main() {
#if defined(LIVE_RUNTIME)
  printf("done\n");
  emscripten_exit_with_live_runtime();
#elif defined(EXIT_FROM_CALLBACK)
  emscripten_async_call(exit_cb, NULL, 0);
  suspend_forever();
#elif defined(FORCE_EXIT)
  emscripten_async_call(force_exit_cb, NULL, 0);
  suspend_forever();
#elif defined(HANG)
  printf("suspending\n");
  suspend_forever();
#endif
  printf("done\n");
  return 0;
}
