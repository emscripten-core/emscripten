#define _GNU_SOURCE // for gettid and pthread_getattr_np
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <assert.h>
#include <unistd.h>

_Atomic pid_t main_tid = 0;
_Atomic pid_t pthread_tid = 0;
_Atomic pid_t worker_tid = 0;

void* _Atomic main_ptr = 0;
void* _Atomic pthread_ptr = 0;
void* _Atomic worker_ptr = 0;

pthread_key_t key;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rec_mutex;
int mutex_protected_data = 0;

EM_JS(int, am_i_pthread, (), {
  return ENVIRONMENT_IS_PTHREAD;
});

EM_JS(int, am_i_wasm_worker, (), {
  return ENVIRONMENT_IS_WASM_WORKER;
});

void do_exit() {
  assert(mutex_protected_data == 2);
  emscripten_out("done");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void *thread_main(void *arg) {
  // Test self ID
  pthread_tid = gettid();
  pthread_ptr = __builtin_thread_pointer();
  emscripten_outf("thread_main: gettid=%d", pthread_tid);
  emscripten_outf("thread_main: __builtin_thread_pointer=%p", pthread_ptr);
  emscripten_outf("thread_main: pthread_self=%p", pthread_self());
  assert(pthread_tid && pthread_tid > main_tid);
  assert(pthread_ptr && pthread_ptr != main_ptr);
  assert(pthread_self() != 0);
  assert(pthread_equal(pthread_self(), pthread_self()));
  assert(am_i_pthread());
  assert(!am_i_wasm_worker());
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() == 0);

  // Test TSD
  int local_data = 42;
  assert(pthread_setspecific(key, &local_data) == 0);
  assert(*(int*)pthread_getspecific(key) == 42);

  // Test mutex
  assert(pthread_mutex_lock(&mutex) == 0);
  mutex_protected_data++;
  assert(pthread_mutex_unlock(&mutex) == 0);

  // Test recursive mutex
  assert(pthread_mutex_lock(&rec_mutex) == 0);
  assert(pthread_mutex_lock(&rec_mutex) == 0); // Lock again
  assert(pthread_mutex_unlock(&rec_mutex) == 0);
  assert(pthread_mutex_unlock(&rec_mutex) == 0);

  // Test stack info
  pthread_attr_t attr;
  void *stack_addr;
  size_t stack_size;
  assert(pthread_getattr_np(pthread_self(), &attr) == 0);
  assert(pthread_attr_getstack(&attr, &stack_addr, &stack_size) == 0);
  intptr_t stack_low = (intptr_t)stack_addr;
  intptr_t stack_high = stack_low + stack_size;
  emscripten_outf("worker stack: stack_high=%#lx, stack_low=%#lx, size=%zu, local=%p", stack_high, stack_low, stack_size, &local_data);
  assert(stack_size > 0);
  assert(stack_addr != NULL);
  assert((intptr_t)&local_data <= stack_high);
  assert((intptr_t)&local_data >= stack_low);

  return 0;
}

void worker_main() {
  // Test self ID
  worker_tid = gettid();
  worker_ptr = __builtin_thread_pointer();
  emscripten_outf("worker_main: gettid=%d", worker_tid);
  emscripten_outf("worker_main: __builtin_thread_pointer=%p", worker_ptr);
  emscripten_outf("worker_main: pthread_self=%p", pthread_self());
  assert(worker_tid && worker_tid > pthread_tid);
  assert(worker_ptr && worker_ptr != pthread_ptr);
  // pthread_self() should return a valid value when called from a Wasm Worker
  // in hybrid mode.
  assert(pthread_self() != 0);
  assert(pthread_equal(pthread_self(), pthread_self()));
  assert(!am_i_pthread());
  assert(am_i_wasm_worker());
  assert(emscripten_current_thread_is_wasm_worker());
  assert(emscripten_wasm_worker_self_id() != 0);

  // Test TSD
  int local_data = 43; // Use a different value to ensure it's thread-local
  assert(pthread_setspecific(key, &local_data) == 0);
  assert(*(int*)pthread_getspecific(key) == 43);

  // Test mutex
  assert(pthread_mutex_lock(&mutex) == 0);
  mutex_protected_data++;
  assert(pthread_mutex_unlock(&mutex) == 0);

  // Test recursive mutex
  assert(pthread_mutex_lock(&rec_mutex) == 0);
  assert(pthread_mutex_lock(&rec_mutex) == 0); // Lock again
  assert(pthread_mutex_unlock(&rec_mutex) == 0);
  assert(pthread_mutex_unlock(&rec_mutex) == 0);

  // Test stack info
  pthread_attr_t attr;
  void *stack_addr;
  size_t stack_size;
  assert(pthread_getattr_np(pthread_self(), &attr) == 0);
  assert(pthread_attr_getstack(&attr, &stack_addr, &stack_size) == 0);
  intptr_t stack_low = (intptr_t)stack_addr;
  intptr_t stack_high = stack_low + stack_size;
  emscripten_outf("worker stack: stack_high=%#lx, stack_low=%#lx, size=%zu, local=%p", stack_high, stack_low, stack_size, &local_data);
  assert(stack_size > 0);
  assert(stack_addr != NULL);
  assert((intptr_t)&local_data <= stack_high);
  assert((intptr_t)&local_data >= stack_low);

  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

int main() {
  main_tid = gettid();
  main_ptr = __builtin_thread_pointer();
  emscripten_outf("main: gettid=%d", main_tid);
  emscripten_outf("main: __builtin_thread_pointer=%p", main_ptr);
  emscripten_outf("main: pthread_self=%p", pthread_self());
  assert(main_tid > 0);
  assert(pthread_key_create(&key, NULL) == 0);

  pthread_mutexattr_t attr;
  assert(pthread_mutexattr_init(&attr) == 0);
  assert(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0);
  assert(pthread_mutex_init(&rec_mutex, &attr) == 0);
  assert(pthread_mutexattr_destroy(&attr) == 0);
  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  pthread_join(thread, NULL);

  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */2048);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
