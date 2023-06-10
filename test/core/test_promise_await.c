#include <assert.h>
#include <emscripten/promise.h>
#include <emscripten/em_js.h>
#include <emscripten/emscripten.h>
#include <stdbool.h>
#include <stdio.h>

void fulfill_from_timout(void* arg) {
  emscripten_promise_resolve((em_promise_t)arg, EM_PROMISE_FULFILL, (void*)43);
}

void test_already_fulfilled() {
  // Test waiting on an already fulfilled promise.
  em_promise_t p = emscripten_promise_create();
  emscripten_promise_resolve(p, EM_PROMISE_FULFILL, (void*)42);

  printf("waiting on promise: %p\n", p);
  em_settled_result_t res = emscripten_promise_await(p);
  printf(".. done wait: %d %ld\n", res.result, (intptr_t)res.value);

  assert(res.result == EM_PROMISE_FULFILL);
  assert(res.value == (void*)42);
  emscripten_promise_destroy(p);
}

void test_not_yet_fulfilled() {
  em_promise_t p = emscripten_promise_create();
  emscripten_async_call(fulfill_from_timout, p, 0);

  printf("waiting on promise: %p\n", p);
  em_settled_result_t res = emscripten_promise_await(p);
  printf(".. done wait: %d %ld\n", res.result, (intptr_t)res.value);

  assert(res.result == EM_PROMISE_FULFILL);
  assert(res.value == (void*)43);
  emscripten_promise_destroy(p);
}

void test_rejected() {
  em_promise_t p = emscripten_promise_create();
  emscripten_promise_resolve(p, EM_PROMISE_REJECT, (void*)44);

  printf("waiting on promise: %p\n", p);
  em_settled_result_t res = emscripten_promise_await(p);
  printf(".. done wait: %d %ld\n", res.result, (intptr_t)res.value);

  assert(res.result == EM_PROMISE_REJECT);
  assert(res.value == (void*)44);
  emscripten_promise_destroy(p);
}

int main() {
  printf("main\n");

  test_already_fulfilled();
  test_not_yet_fulfilled();
  test_rejected();

  printf("main done\n");
  return 0;
}
