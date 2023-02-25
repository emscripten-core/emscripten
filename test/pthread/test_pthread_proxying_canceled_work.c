#include <assert.h>
#include <emscripten/proxying.h>
#include <pthread.h>
#include <stdio.h>

em_proxying_queue* queue;

void explode(void* arg) { assert(0 && "the work should not be run!"); }

void set_flag(void* flag) {
  // Schedule the flag to be set on the next turn of the event loop so that we
  // can be sure cleanup has finished first. We need to use EM_ASM and JS here
  // because this code needs to run after the thread runtime has exited.

  // clang-format off
  EM_ASM({setTimeout(() => Atomics.store(HEAP32, $0 >> 2, 1))}, flag);
  // clang-format on
}

// Used to call `set_flag` on thread exit or cancellation.
pthread_key_t dtor_key;

void* cancel_self(void* canceled) {
  pthread_setspecific(dtor_key, canceled);
  pthread_cancel(pthread_self());
  pthread_testcancel();
  assert(0 && "thread should have been canceled!");
  return NULL;
}

void* exit_self(void* exited) {
  pthread_setspecific(dtor_key, exited);
  pthread_exit(NULL);
  assert(0 && "thread should have exited!");
  return NULL;
}

void test_cancel_then_proxy() {
  printf("testing cancel followed by proxy\n");

  pthread_t thread;
  _Atomic int canceled = 0;
  pthread_create(&thread, NULL, cancel_self, &canceled);

  // Wait for the thread to be canceled.
  while (!canceled) {
  }

  // Proxying work to the thread should return an error.
  int ret = emscripten_proxy_sync(queue, thread, explode, NULL);
  assert(ret == 0);

  pthread_join(thread, NULL);
}

void test_exit_then_proxy() {
  printf("testing exit followed by proxy\n");

  pthread_t thread;
  _Atomic int exited = 0;
  pthread_create(&thread, NULL, exit_self, &exited);

  // Wait for the thread to exit.
  while (!exited) {
  }

  // Proxying work to the thread should return an error.
  int ret = emscripten_proxy_sync(queue, thread, explode, NULL);
  assert(ret == 0);

  pthread_join(thread, NULL);
}

int main() {
  queue = em_proxying_queue_create();
  pthread_key_create(&dtor_key, set_flag);

  test_cancel_then_proxy();
  test_exit_then_proxy();

  em_proxying_queue_destroy(queue);

  printf("done\n");
}
