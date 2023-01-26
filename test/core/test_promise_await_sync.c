#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <emscripten/promise.h>

em_promise_result_t increment(void** result, void* data, void* value) {
  emscripten_console_log("incrementing counter in promise callback");
  int* counter = data;
  (*counter)++;
  *result = (void*)42;
  return EM_PROMISE_FULFILL;
}

typedef struct counter_and_promise {
  int* counter;
  em_promise_t promise;
} counter_and_promise;

void increment2(void* arg) {
  emscripten_console_log("incrementing counter in callback");
  counter_and_promise* data = (counter_and_promise*)arg;
  (*data->counter)++;
  emscripten_promise_resolve(data->promise, EM_PROMISE_FULFILL, (void*)123);
  emscripten_promise_destroy(data->promise);
}

int main() {
  int counter = 0;

  // Synchronously await a promise we create.
  em_promise_t start = emscripten_promise_create();
  em_promise_t incremented =
    emscripten_promise_then(start, increment, NULL, &counter);

  emscripten_promise_resolve(start, EM_PROMISE_FULFILL, NULL);
  emscripten_promise_destroy(start);

  void* result = emscripten_promise_await_sync(incremented);
  assert(result == (void*)42);
  emscripten_promise_destroy(incremented);

  emscripten_console_logf("incremented counter: %d", counter);
  assert(counter == 1);

  // Synchronously await a promise resolved in a callback.
  counter_and_promise arg = (counter_and_promise){
    .counter = &counter, .promise = emscripten_promise_create()};
  emscripten_async_call(increment2, &arg, 0);

  result = emscripten_promise_await_sync(arg.promise);
  assert(result == (void*)123);

  emscripten_console_logf("incremented again: %d", counter);
  assert(counter == 2);

  emscripten_console_log("done");
  return 0;
}
