#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "emscripten.h"
#include "emscripten/threading.h"

// Disable leak checking since we have allocations that deliberately outlive
// `main`.
const char* __asan_default_options() { return "detect_leaks=0"; }

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
  printf("running widget %d on %s\n",
         w->val,
         pthread_equal(pthread_self(), main_thread) ? "main" : "pthread");
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
  emscripten_async_as_sync_finish(w->ctx);
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
  widget w1, w2, w3, w4;
  init_widget(&w1, &i, 1);
  init_widget(&w2, &i, 2);
  init_widget(&w3, &i, 3);
  init_widget(&w4, &i, 4);

  // Dispatching to ourselves should synchronously call.
  emscripten_dispatch_to_thread_ptr(pthread_self(), do_run_widget, &w1);
  assert(i == 1);
  assert(w1.done);
  assert(pthread_equal(w1.thread, pthread_self()));

  emscripten_dispatch_to_thread(
    pthread_self(), EM_FUNC_SIG_VI, do_run_widget, NULL, &w2);
  assert(i == 2);
  assert(w2.done);
  assert(pthread_equal(w2.thread, pthread_self()));

  // Dispatching to another thread should also work.
  emscripten_dispatch_to_thread_ptr(main_thread, do_run_widget, &w3);
  await_widget(&w3);
  assert(i == 3);
  assert(w3.done);
  assert(pthread_equal(w3.thread, main_thread));

  emscripten_dispatch_to_thread(
    main_thread, EM_FUNC_SIG_VI, do_run_widget, NULL, &w4);
  await_widget(&w4);
  assert(i == 4);
  assert(w4.done);
  assert(pthread_equal(w4.thread, main_thread));

  destroy_widget(&w1);
  destroy_widget(&w2);
  destroy_widget(&w3);
  destroy_widget(&w4);
}

void test_dispatch_async() {
  printf("Testing async dispatch\n");

  int i = 0;
  widget* w5 = malloc_widget(NULL, 5);
  widget* w6 = malloc_widget(NULL, 6);
  widget w7, w8;
  init_widget(&w7, &i, 7);
  init_widget(&w8, &i, 8);

  // Dispatching to ourselves should NOT synchronously call.
  emscripten_dispatch_to_thread_async_ptr(
    pthread_self(), run_and_free_widget, w5);
  assert(!w5->done);

  emscripten_dispatch_to_thread_async(
    pthread_self(), EM_FUNC_SIG_VI, run_and_free_widget, NULL, w6);
  assert(!w6->done);

  // Dispatching to another thread should work.
  emscripten_dispatch_to_thread_async_ptr(main_thread, do_run_widget, &w7);
  await_widget(&w7);
  assert(i == 7);
  assert(w7.done);
  assert(pthread_equal(w7.thread, main_thread));

  emscripten_dispatch_to_thread_async(
    main_thread, EM_FUNC_SIG_VI, do_run_widget, NULL, &w8);
  await_widget(&w8);
  assert(i == 8);
  assert(w8.done);
  assert(pthread_equal(w8.thread, main_thread));

  destroy_widget(&w7);
  destroy_widget(&w8);
}

void test_dispatch_sync() {
  printf("Testing sync dispatch\n");

  int i = 0;
  widget w9, w10, w11, w12;
  init_widget(&w9, &i, 9);
  init_widget(&w10, &i, 10);
  init_widget(&w11, &i, 11);
  init_widget(&w12, &i, 12);

  // Dispatching to ourselves should synchronously call.
  emscripten_dispatch_to_thread_sync_ptr(pthread_self(), do_run_widget, &w9);
  assert(i == 9);
  assert(w9.done);
  assert(pthread_equal(w9.thread, pthread_self()));

  emscripten_dispatch_to_thread_sync(
    pthread_self(), EM_FUNC_SIG_VI, do_run_widget, NULL, &w10);
  assert(i == 10);
  assert(w10.done);
  assert(pthread_equal(w10.thread, pthread_self()));

  // Dispatching to another thread should also work without us having to
  // separately wait.
  emscripten_dispatch_to_thread_sync_ptr(main_thread, do_run_widget, &w11);
  assert(i == 11);
  assert(w11.done);
  assert(pthread_equal(w11.thread, main_thread));

  emscripten_dispatch_to_thread_sync(
    main_thread, EM_FUNC_SIG_VI, do_run_widget, NULL, &w12);
  assert(i == 12);
  assert(w12.done);
  assert(pthread_equal(w12.thread, main_thread));

  destroy_widget(&w9);
  destroy_widget(&w10);
  destroy_widget(&w11);
  destroy_widget(&w12);
}

void test_dispatch_async_as_sync() {
  printf("Testing async_as_sync dispatch\n");

  int i = 0;
  widget w13, w14, w15, w16;
  init_widget(&w13, &i, 13);
  init_widget(&w14, &i, 14);
  init_widget(&w15, &i, 15);
  init_widget(&w16, &i, 16);

  // Dispatching to ourselves should synchronously call and not hang as long as
  // the work finishes as part of the initial call.
  emscripten_dispatch_to_thread_async_as_sync_ptr(
    pthread_self(), start_and_finish_running_widget, &w13);
  assert(i == 13);
  assert(w13.done);
  assert(pthread_equal(w13.thread, pthread_self()));

  emscripten_dispatch_to_thread_async_as_sync(pthread_self(),
                                              EM_FUNC_SIG_VII,
                                              start_and_finish_running_widget,
                                              NULL,
                                              &w14);
  assert(i == 14);
  assert(w14.done);
  assert(pthread_equal(w14.thread, pthread_self()));

  // Dispatching to another thread should also work, even if the work is
  // asynchronous.
  emscripten_dispatch_to_thread_async_as_sync_ptr(
    main_thread, start_running_widget, &w15);
  assert(i == 15);
  assert(w15.done);
  assert(pthread_equal(w15.thread, main_thread));

  emscripten_dispatch_to_thread_async_as_sync(
    main_thread, EM_FUNC_SIG_VII, start_running_widget, NULL, &w16);
  assert(i == 16);
  assert(w16.done);
  assert(pthread_equal(w16.thread, main_thread));

  destroy_widget(&w13);
  destroy_widget(&w14);
  destroy_widget(&w15);
  destroy_widget(&w16);
}

void force_exit(void* arg) { emscripten_force_exit(0); }

int main(int argc, char* argv[]) {
  main_thread = emscripten_main_browser_thread_id();
  // We should be running in PROXY_TO_PTHREAD mode.
  assert(!pthread_equal(main_thread, pthread_self()));

  test_dispatch();
  test_dispatch_async();
  test_dispatch_sync();
  test_dispatch_async_as_sync();

  // Schedule a real exit for after the async calls have been run.
  emscripten_set_timeout(force_exit, 0, NULL);

  printf("done\n");
}
