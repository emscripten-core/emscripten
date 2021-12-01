#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "emscripten.h"
#include "emscripten/threading.h"

pthread_t main_thread;

typedef struct widget {
  // `val` will be stored to `out` and the current thread will be stored to
  // `thread` when the widget is run.
  int* out;
  int val;
  pthread_t thread;

  // Synchronization to allow waiting on a widget to run.
  pthread_mutex_t mutex;
  pthread_cond_t cond;

  // Nonzero iff the widget has been run.
  int done;

  // Only used for async_as_sync tests.
  em_async_as_sync_ctx* ctx;
} widget;

void init_widget(widget* w, int* out, int val) {
  widget empty = {out, val};
  *w = empty;
  pthread_mutex_init(&w->mutex, NULL);
  pthread_cond_init(&w->cond, NULL);
}

void destroy_widget(widget* w) {
  pthread_mutex_destroy(&w->mutex);
  pthread_cond_destroy(&w->cond);
}

widget* malloc_widget(int* out, int val) {
  widget* w = malloc(sizeof(widget));
  assert(w);
  init_widget(w, out, val);
  return w;
}

void free_widget(widget* w) {
  destroy_widget(w);
  free(w);
}

void run_widget(widget* w) {
  pthread_mutex_lock(&w->mutex);
  if (w->out) {
    *w->out = w->val;
  }
  w->thread = pthread_self();
  w->done = 1;
  pthread_mutex_unlock(&w->mutex);
  pthread_cond_broadcast(&w->cond);
}

void await_widget(widget* w) {
  pthread_mutex_lock(&w->mutex);
  while (!w->done) {
    pthread_cond_wait(&w->cond, &w->mutex);
  }
  pthread_mutex_unlock(&w->mutex);
}

// Helper functions we will dispatch to perform our work.

void do_run_widget(void* arg) { run_widget((widget*)arg); }

void run_and_free_widget(void* arg) {
  widget* w = (widget*)arg;
  run_widget(w);
  free_widget(w);
}

void finish_running_widget(void* arg) {
  widget* w = (widget*)arg;
  run_widget(w);
  emscripten_async_as_sync_ptr_finish(w->ctx);
}

void start_running_widget(em_async_as_sync_ctx* ctx, void* arg) {
  ((widget*)arg)->ctx = ctx;
  emscripten_async_call(finish_running_widget, arg, 0);
}

void start_and_finish_running_widget(em_async_as_sync_ctx* ctx, void* arg) {
  ((widget*)arg)->ctx = ctx;
  finish_running_widget(arg);
}

// Main test functions

void test_dispatch() {
  printf("Testing dispatch\n");

  int i = 0;
  widget w1, w2;
  init_widget(&w1, &i, 1);
  init_widget(&w2, &i, 2);

  // Dispatching to ourselves should synchronously call.
  emscripten_dispatch_to_thread_ptr(pthread_self(), do_run_widget, &w1);
  assert(i == 1);
  assert(w1.done);
  assert(pthread_equal(w1.thread, pthread_self()));

  // Dispatching to another thread should also work.
  emscripten_dispatch_to_thread_ptr(main_thread, do_run_widget, &w2);
  await_widget(&w2);
  assert(i == 2);
  assert(w2.done);
  assert(pthread_equal(w2.thread, main_thread));

  destroy_widget(&w1);
  destroy_widget(&w2);
}

void test_dispatch_async() {
  printf("Testing async dispatch\n");

  int i = 0;
  widget* w1 = malloc_widget(NULL, 1);
  widget w2;
  init_widget(&w2, &i, 2);

  // Dispatching to ourselves should NOT synchronously call.
  emscripten_dispatch_to_thread_async_ptr(
    pthread_self(), run_and_free_widget, &w1);
  assert(!w1->done);

  // Dispatching to another thread should work.
  emscripten_dispatch_to_thread_async_ptr(main_thread, do_run_widget, &w2);
  await_widget(&w2);
  assert(i == 2);
  assert(w2.done);
  assert(pthread_equal(w2.thread, main_thread));

  destroy_widget(&w2);
}

void test_dispatch_sync() {
  printf("Testing sync dispatch\n");

  int i = 0;
  widget w1, w2;
  init_widget(&w1, &i, 1);
  init_widget(&w2, &i, 2);

  // Dispatching to ourselves should synchronously call.
  emscripten_dispatch_to_thread_sync_ptr(pthread_self(), do_run_widget, &w1);
  assert(i == 1);
  assert(w1.done);
  assert(pthread_equal(w1.thread, pthread_self()));

  // Dispatching to another thread should also work without us having to
  // separately wait.
  emscripten_dispatch_to_thread_sync_ptr(main_thread, do_run_widget, &w2);
  assert(i == 2);
  assert(w2.done);
  assert(pthread_equal(w2.thread, main_thread));

  destroy_widget(&w1);
  destroy_widget(&w2);
}

void test_dispatch_async_as_sync() {
  printf("Testing async_as_sync dispatch\n");

  int i = 0;
  widget w1, w2;
  init_widget(&w1, &i, 1);
  init_widget(&w2, &i, 2);

  // Dispatching to ourselves should synchronously call and not hang as long as
  // the work finishes as part of the initial call.
  printf("async_as_sync 1\n");
  emscripten_dispatch_to_thread_async_as_sync_ptr(
    pthread_self(), start_and_finish_running_widget, &w1);
  assert(i == 1);
  assert(w1.done);
  assert(pthread_equal(w1.thread, pthread_self()));

  // Dispatching to another thread should also work, even if the work is
  // asynchronous.
  printf("async_as_sync 2\n");
  emscripten_dispatch_to_thread_async_as_sync_ptr(
    main_thread, start_running_widget, &w2);
  assert(i == 2);
  assert(w2.done);
  assert(pthread_equal(w2.thread, main_thread));

  destroy_widget(&w1);
  destroy_widget(&w2);
}

int main(int argc, char* argv[]) {
  main_thread = emscripten_main_browser_thread_id();
  // We should be running in PROXY_TO_PTHREAD mode.
  assert(!pthread_equal(main_thread, pthread_self()));

  test_dispatch();
  test_dispatch_async();
  test_dispatch_sync();
  test_dispatch_async_as_sync();
  printf("done\n");
}
