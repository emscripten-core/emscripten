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

void do_exit() {
  emscripten_out("done");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void *thread_main(void *arg) {
  pthread_tid = gettid();
  pthread_ptr = __builtin_thread_pointer();
  emscripten_outf("thread_main: gettid=%d", pthread_tid);
  emscripten_outf("thread_main: __builtin_thread_pointer=%p", pthread_ptr);
  emscripten_outf("thread_main: pthread_self=%p", pthread_self());
  assert(pthread_tid && pthread_tid > main_tid);
  assert(pthread_ptr && pthread_ptr != main_ptr);
  assert(pthread_self() != 0);
  assert(am_i_pthread());
  assert(!am_i_wasm_worker());
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() == 0);
  return 0;
}

void worker_main() {
  worker_tid = gettid();
  worker_ptr = __builtin_thread_pointer();
  emscripten_outf("worker_main: gettid=%d", worker_tid);
  emscripten_outf("worker_main: __builtin_thread_pointer=%p", worker_ptr);
  emscripten_outf("worker_main: pthread_self=%p", pthread_self());
  assert(worker_tid && worker_tid > pthread_tid);
  assert(worker_ptr && worker_ptr != pthread_ptr);
  // Currently pthead_self return NULL when called from a wasm worker.
  // See https://github.com/emscripten-core/emscripten/issues/26631
  assert(pthread_self() == 0);
  assert(!am_i_pthread());
  assert(am_i_wasm_worker());
  assert(emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() != 0);

  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

int main() {
  main_tid = gettid();
  main_ptr = __builtin_thread_pointer();
  emscripten_outf("main: gettid=%d", main_tid);
  emscripten_outf("main: __builtin_thread_pointer=%p", main_ptr);
  emscripten_outf("main: pthread_self=%p", pthread_self());
  assert(main_tid > 0);
  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  pthread_join(thread, NULL);

  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */1024);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
