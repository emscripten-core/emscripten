#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/console.h>
#include <emscripten/proxying.h>

pthread_t t;
pthread_barrier_t started;

void timeout(void* arg) {
  emscripten_err("timeout");
  pthread_barrier_wait(&started);
}

void* thread_main(void* arg) {
  // Keep the thread runtime alive for now.
  emscripten_err("thread_main");
  emscripten_runtime_keepalive_push();
  emscripten_set_timeout(timeout, 0, NULL);
  return NULL;
}

void say_hello(void* arg) {
  emscripten_err("say_hello");
}

void keepalive_pop(void* arg) {
  emscripten_err("keepalive_pop");
  // After this the called, thread should exit (become joinable).
  emscripten_runtime_keepalive_pop();
}

int main() {
  printf("main\n");
  pthread_barrier_init(&started, NULL, 2);
  int rc = pthread_create(&t, NULL, thread_main, NULL);
  assert(rc == 0);

  // Wait until the thread is running from the event loop
  pthread_barrier_wait(&started);

  // Run something from the thread's event loop
  emscripten_proxy_sync(emscripten_proxy_get_system_queue(), t, &say_hello, NULL);

  // Run keepalive_pop, causing the thread to exit gracefully
  emscripten_proxy_sync(emscripten_proxy_get_system_queue(), t, &keepalive_pop, NULL);

  // Now the thread should be joinable
  void* thread_rtn = 0;
  rc = pthread_join(t, &thread_rtn);
  assert(rc == 0);
  printf("done join\n");
  return 0;
}
