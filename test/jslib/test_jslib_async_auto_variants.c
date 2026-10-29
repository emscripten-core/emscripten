#include <assert.h>
#include <emscripten.h>
#include <emscripten/promise.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

intptr_t answer(int ms, intptr_t value);
em_promise_t answer_promise(int ms, intptr_t value);
void wait_ms(int ms);
em_promise_t wait_ms_promise(int ms);

int stage;

em_promise_result_t fail(void** result, void* data, void* value) {
  assert(0 && "promise rejected");
}

em_promise_result_t on_waited(void** result, void* data, void* value) {
  assert(stage++ == 3);
  // A void function's promise is fulfilled with NULL.
  assert(value == NULL);
  printf("done\n");
#ifdef __EMSCRIPTEN_PTHREADS__
  exit(0);
#endif
  return EM_PROMISE_FULFILL;
}

em_promise_result_t on_rejected(void** result, void* data, void* value) {
  assert(stage++ == 2);
  assert(value == NULL);
  em_promise_t p = wait_ms_promise(5);
  em_promise_t next = emscripten_promise_then(p, on_waited, fail, NULL);
  emscripten_promise_destroy(p);
  emscripten_promise_destroy(next);
  return EM_PROMISE_FULFILL;
}

em_promise_result_t on_fulfilled(void** result, void* data, void* value) {
  assert(stage++ == 1);
  assert((intptr_t)value == 42);
  // A rejected promise.
  em_promise_t p = answer_promise(5, 0);
  em_promise_t next = emscripten_promise_then(p, fail, on_rejected, NULL);
  emscripten_promise_destroy(p);
  emscripten_promise_destroy(next);
  return EM_PROMISE_FULFILL;
}

em_promise_result_t on_sync(void** result, void* data, void* value) {
  assert(stage++ == 0);
  assert((intptr_t)value == 7);
  em_promise_t p = answer_promise(5, 42);
  em_promise_t next = emscripten_promise_then(p, on_fulfilled, fail, NULL);
  emscripten_promise_destroy(p);
  emscripten_promise_destroy(next);
  return EM_PROMISE_FULFILL;
}

int main() {
  assert(answer(0, 7) == 7);
#if defined(__EMSCRIPTEN_PTHREADS__) || defined(AUTO)
  assert(answer(5, 42) == 42);
#else
  // Where the function cannot wait, the body is told so.
  assert(answer(5, 42) == -1);
#endif
  wait_ms(0);
  wait_ms(5);

  // A synchronous completion is a fulfilled promise.
  em_promise_t p = answer_promise(0, 7);
  em_promise_t next = emscripten_promise_then(p, on_sync, fail, NULL);
  emscripten_promise_destroy(p);
  emscripten_promise_destroy(next);
  return 0;
}
