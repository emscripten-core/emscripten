#include <pthread.h>
#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <assert.h>
#define _GNU_SOURCE // for gettid
#include <unistd.h>

_Atomic pid_t main_tid = 0;
_Atomic pid_t pthread_tid = 0;
_Atomic pid_t worker_tid = 0;

void* _Atomic main_ptr = 0;
void* _Atomic pthread_ptr = 0;
void* _Atomic worker_ptr = 0;

EM_JS(int, am_i_pthread, (), {
  return ENVIRONMENT_IS_PTHREAD;
});

EM_JS(int, am_i_wasm_worker, (), {
  return ENVIRONMENT_IS_WASM_WORKER;
});

void *thread_main(void *arg) {
  pthread_tid = gettid();
  pthread_ptr = __builtin_thread_pointer();
  emscripten_outf("hello from pthread! (tid=%d) (ptr=%p)", pthread_tid, pthread_ptr);
  assert(pthread_tid && pthread_tid > main_tid);
  assert(pthread_ptr && pthread_ptr != main_ptr);
  assert(am_i_pthread());
  assert(!am_i_wasm_worker());
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() == 0);
  return 0;
}

void worker_main() {
  worker_tid = gettid();
  worker_ptr = __builtin_thread_pointer();
  emscripten_outf("hello from wasm worker! (tid=%d) (ptr=%p)", worker_tid, worker_ptr);
  assert(worker_tid && worker_tid > pthread_tid);
  assert(worker_ptr && worker_ptr != pthread_ptr);
  assert(!am_i_pthread());
  assert(am_i_wasm_worker());
  assert(emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() != 0);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main() {
  main_tid = gettid();
  main_ptr = __builtin_thread_pointer();
  emscripten_outf("in main (tid=%d) (ptr=%p)", main_tid, main_ptr);
  assert(main_tid > 0);
  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  pthread_join(thread, NULL);

  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */1024);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
