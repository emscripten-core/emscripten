#include <pthread.h>
#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <assert.h>

volatile int pthread_ran = 0;

EM_JS(int, am_i_pthread, (), {
  return ENVIRONMENT_IS_PTHREAD;
});

EM_JS(int, am_i_wasm_worker, (), {
  return ENVIRONMENT_IS_WASM_WORKER;
});

void *thread_main(void *arg) {
  emscripten_out("hello from pthread!");
  assert(am_i_pthread());
  assert(!am_i_wasm_worker());
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() == 0);
  pthread_ran = 1;
  return 0;
}

void worker_main() {
  emscripten_out("hello from wasm worker!");
  assert(!am_i_pthread());
  assert(am_i_wasm_worker());
  assert(emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() != 0);

  while(!emscripten_atomic_cas_u32((void*)&pthread_ran, 0, 1))
    emscripten_wasm_worker_sleep(10);
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);

  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */1024);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
