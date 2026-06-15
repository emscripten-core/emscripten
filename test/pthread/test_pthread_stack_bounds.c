#include <assert.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/stack.h>

void* thread_main(void* arg) {
  bool passed;
  size_t stack_base = emscripten_stack_get_base();
  size_t stack_max = emscripten_stack_get_end();
  size_t current = (size_t) &passed;
  assert(stack_base > current && current > stack_max);
  emscripten_force_exit(0);
  return 0;
}

pthread_t t;

int main(void) {
  pthread_create(&t, NULL, thread_main, NULL);
  emscripten_exit_with_live_runtime();
  __builtin_trap();
}
